/**
 *
 *  @file build_command.cpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2026, Gaspard Kirira.
 *  All rights reserved.
 *  https://github.com/cgride/cli
 *
 *  Use of this source code is governed by an MIT license
 *  that can be found in the LICENSE file.
 *
 *  Cgride
 *
 */
#include <cgride/cli/build_command.hpp>

#include <filesystem>
#include <string>
#include <utility>

#include <cgride/config/config_loader.hpp>
#include <cgride/config/project_reader.hpp>
#include <cgride/core/error.hpp>
#include <cgride/engine/build_engine.hpp>
#include <cgride/engine/build_options.hpp>
#include <cgride/engine/build_request.hpp>
#include <cgride/toolchains/discovery.hpp>
#include <cgride/toolchains/compiler_kind.hpp>

namespace cgride::cli
{
  namespace
  {
    [[nodiscard]] std::filesystem::path default_build_directory(
        const CommandContext &context)
    {
      return context.resolved_project_root() / ".cgride" / "build";
    }

    [[nodiscard]] cgride::engine::BuildOptions make_build_options(
        const CommandContext &context)
    {
      const auto &cli_options = context.options();

      cgride::engine::BuildOptions build_options;

      build_options
          .build_directory(default_build_directory(context))
          .mode(cli_options.release()
                    ? cgride::engine::BuildMode::Release
                    : cgride::engine::BuildMode::Debug)
          .jobs(cli_options.jobs())
          .rebuild(cli_options.rebuild())
          .use_cache(!cli_options.no_cache())
          .dry_run(cli_options.dry_run())
          .verbose(cli_options.verbose());

      if (cli_options.has_target())
      {
        build_options.target(cli_options.target().value());
      }

      return build_options;
    }

    void print_error_detail(
        Terminal &terminal,
        const cgride::core::Error &error)
    {
      terminal.print_error(error.message());

      if (error.detail().has_value() && !error.detail().value().empty())
      {
        terminal.writeln_error("detail: " + error.detail().value());
      }

      if (error.path().has_value() && !error.path().value().empty())
      {
        terminal.writeln_error("path: " + error.path().value().string());
      }
    }

    [[nodiscard]] ExitCode config_error(
        Terminal &terminal,
        const cgride::core::Error &error)
    {
      print_error_detail(terminal, error);
      return ExitCode::ConfigError;
    }

    [[nodiscard]] ExitCode build_error(
        Terminal &terminal,
        const cgride::core::Error &error)
    {
      print_error_detail(terminal, error);
      return ExitCode::BuildError;
    }

  } // namespace

  ExitCode BuildCommand::run(CommandContext &context) const
  {
    auto &terminal = context.terminal();

    if (!context.valid())
    {
      terminal.print_error("Invalid command context.");
      return ExitCode::InternalError;
    }

    terminal.verbose(context.options().verbose());

    terminal.print_verbose("Loading project configuration.");

    cgride::config::ConfigLoader loader(context.config_options());

    auto document = loader.load();

    if (!document)
    {
      return config_error(terminal, document.error());
    }

    terminal.print_verbose("Reading project model.");

    cgride::config::ProjectReader reader;

    auto project = reader.read(document.value());

    if (!project)
    {
      return config_error(terminal, project.error());
    }

    terminal.print_verbose("Discovering C++ toolchain.");

    auto toolchain = cgride::toolchains::discover_toolchain(
        cgride::toolchains::CompilerKind::Unknown);

    if (!toolchain)
    {
      return build_error(terminal, toolchain.error());
    }

    terminal.print_info("Building project.");

    cgride::engine::BuildRequest request;

    request
        .project(std::move(project.value()))
        .toolchain(std::move(toolchain.value()))
        .options(make_build_options(context));

    cgride::engine::BuildEngine engine;

    auto result = engine.build(request);

    if (result.failed())
    {
      if (result.error().has_value())
      {
        return build_error(terminal, result.error().value());
      }

      terminal.print_error("Build failed.");
      return ExitCode::BuildError;
    }

    terminal.print_success("Build finished.");

    return ExitCode::Success;
  }

} // namespace cgride::cli

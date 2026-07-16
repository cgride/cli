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

#include <utility>

#include <cgride/core/error.hpp>
#include <cgride/engine/build_engine.hpp>
#include <cgride/engine/build_request.hpp>

#include "project_loader.hpp"

namespace cgride::cli
{
  namespace
  {
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

    terminal.print_verbose("Loading project.");

    auto loaded = load_project(context);

    if (!loaded)
    {
      return config_error(terminal, loaded.error());
    }

    terminal.print_verbose("Preparing build request.");

    auto toolchain = discover_cli_toolchain(context);

    if (!toolchain)
    {
      return build_error(terminal, toolchain.error());
    }

    cgride::engine::BuildRequest request;

    request
        .project(std::move(loaded.value().project))
        .toolchain(std::move(toolchain.value()))
        .options(make_build_options(context, loaded.value().project_root));

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

    return ExitCode::Success;
  }

} // namespace cgride::cli

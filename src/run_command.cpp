/**
 *
 *  @file run_command.cpp
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
#include <cgride/cli/run_command.hpp>

#include <filesystem>
#include <string>
#include <utility>

#include <cgride/core/command.hpp>
#include <cgride/core/error.hpp>
#include <cgride/engine/build_engine.hpp>
#include <cgride/engine/build_request.hpp>
#include <cgride/executor/execution_options.hpp>
#include <cgride/executor/process.hpp>

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

  ExitCode RunCommand::run(CommandContext &context) const
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

    const auto *selected_target = select_executable_target(
        loaded.value().project,
        context.options().target());

    if (selected_target == nullptr)
    {
      if (context.options().has_target())
      {
        terminal.print_error("Requested run target was not found or is not executable.");
        terminal.writeln_error("target: " + context.options().target().value());
      }
      else
      {
        terminal.print_error("Project does not contain an executable target to run.");
      }

      return ExitCode::ConfigError;
    }

    const auto selected_target_name = selected_target->name();
    auto build_options = make_build_options(context, loaded.value().project_root);
    build_options.target(selected_target_name);

    auto executable = executable_path_for(
        loaded.value().project_root,
        build_options,
        *selected_target);

    auto toolchain = discover_cli_toolchain(context);

    if (!toolchain)
    {
      return build_error(terminal, toolchain.error());
    }

    cgride::engine::BuildRequest request;

    request
        .project(std::move(loaded.value().project))
        .toolchain(std::move(toolchain.value()))
        .options(build_options);

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

    cgride::core::Command command(executable.string());
    command.cwd(loaded.value().project_root);
    command.search_in_path(false);

    cgride::executor::ExecutionOptions execution_options;
    execution_options.capture_output(false);

    terminal.print_verbose(cgride::executor::describe_command(command));

    auto process = cgride::executor::run_process(command, execution_options);

    if (!process)
    {
      print_error_detail(terminal, process.error());
      return ExitCode::BuildError;
    }

    if (process.value().has_error())
    {
      print_error_detail(terminal, process.value().error().value());
      return ExitCode::BuildError;
    }

    if (process.value().exit_code().value_or(1) != 0)
    {
      return ExitCode::Failure;
    }

    return ExitCode::Success;
  }

} // namespace cgride::cli

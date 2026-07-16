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

#include <chrono>
#include <filesystem>
#include <system_error>
#include <vector>
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


    using Clock = std::chrono::steady_clock;

    [[nodiscard]] std::string format_duration(Clock::duration duration)
    {
      const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
      return std::to_string(milliseconds) + "ms";
    }

    void collect_target_inputs(
        const cgride::project::Project &project,
        const cgride::project::Target &target,
        std::vector<std::filesystem::path> &inputs)
    {
      for (const auto &entry : target.source_set().entries())
      {
        if (entry.valid() && entry.kind() == cgride::project::SourceKind::File)
        {
          inputs.push_back(entry.path());
        }
      }

      for (const auto &link : target.target_links())
      {
        if (!link.valid())
        {
          continue;
        }

        const auto *dependency = project.find_target(link.target_name());

        if (dependency != nullptr)
        {
          collect_target_inputs(project, *dependency, inputs);
        }
      }
    }

    [[nodiscard]] bool executable_is_fresh(
        const LoadedProject &loaded,
        const cgride::project::Target &target,
        const std::filesystem::path &executable)
    {
      std::error_code error;

      if (!std::filesystem::exists(executable, error) || error)
      {
        return false;
      }

      const auto executable_time = std::filesystem::last_write_time(executable, error);

      if (error)
      {
        return false;
      }

      std::vector<std::filesystem::path> inputs;
      inputs.push_back(loaded.project_file);
      collect_target_inputs(loaded.project, target, inputs);

      for (const auto &input : inputs)
      {
        if (!std::filesystem::exists(input, error) || error)
        {
          return false;
        }

        const auto input_time = std::filesystem::last_write_time(input, error);

        if (error || input_time > executable_time)
        {
          return false;
        }
      }

      return true;
    }

    [[nodiscard]] ExitCode run_executable(
        Terminal &terminal,
        const std::filesystem::path &project_root,
        const std::filesystem::path &executable)
    {
      cgride::core::Command command(executable.string());
      command.cwd(project_root);
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

  } // namespace

  ExitCode RunCommand::run(CommandContext &context) const
  {
    auto &terminal = context.terminal();

    if (!context.valid())
    {
      terminal.print_error("Invalid command context.");
      return ExitCode::InternalError;
    }

    const auto command_started_at = Clock::now();
    terminal.verbose(context.options().verbose());
    terminal.print_verbose("Loading project.");

    const auto load_started_at = Clock::now();
    auto loaded = load_project(context);

    if (!loaded)
    {
      return config_error(terminal, loaded.error());
    }

    auto loaded_project = std::move(loaded.value());
    terminal.print_verbose("project root: " + loaded_project.project_root.string());
    terminal.print_verbose("load project: " + format_duration(Clock::now() - load_started_at));

    const auto *selected_target = select_executable_target(
        loaded_project.project,
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
    auto build_options = make_build_options(context, loaded_project.project_root);
    build_options.target(selected_target_name);

    auto executable = executable_path_for(
        loaded_project.project_root,
        build_options,
        *selected_target);

    if (!context.options().rebuild() && !context.options().no_cache() &&
        executable_is_fresh(loaded_project, *selected_target, executable))
    {
      terminal.print_verbose("target: " + selected_target_name);
      terminal.print_verbose("graph: fresh");
      terminal.print_verbose("total: " + format_duration(Clock::now() - command_started_at));
      return run_executable(terminal, loaded_project.project_root, executable);
    }

    cgride::toolchains::Toolchain selected_toolchain;

    if (loaded_project.toolchain.has_value())
    {
      terminal.print_verbose("toolchain: reused");
      selected_toolchain = std::move(loaded_project.toolchain.value());
    }
    else
    {
      const auto toolchain_started_at = Clock::now();
      auto toolchain = discover_cli_toolchain(context);

      if (!toolchain)
      {
        return build_error(terminal, toolchain.error());
      }

      terminal.print_verbose("toolchain: discovered in " + format_duration(Clock::now() - toolchain_started_at));
      selected_toolchain = std::move(toolchain.value());
    }

    cgride::engine::BuildRequest request;

    request
        .project(std::move(loaded_project.project))
        .toolchain(std::move(selected_toolchain))
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

    terminal.print_verbose("total: " + format_duration(Clock::now() - command_started_at));
    return run_executable(terminal, loaded_project.project_root, executable);
  }

} // namespace cgride::cli

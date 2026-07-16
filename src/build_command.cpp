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

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

#include <cgride/core/error.hpp>
#include <cgride/core/event.hpp>
#include <cgride/engine/build_engine.hpp>
#include <cgride/engine/build_request.hpp>

#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif

#include "project_loader.hpp"

namespace cgride::cli
{
  namespace
  {
    using Clock = std::chrono::steady_clock;

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

    [[nodiscard]] bool stdout_is_terminal() noexcept
    {
#if defined(_WIN32)
      return ::_isatty(::_fileno(stdout)) != 0;
#else
      return ::isatty(STDOUT_FILENO) != 0;
#endif
    }

    [[nodiscard]] bool can_render_dynamic_progress(const Terminal &terminal) noexcept
    {
      return !terminal.quiet() && (&terminal.output_stream() == &std::cout) && stdout_is_terminal();
    }

    [[nodiscard]] std::string format_duration(Clock::duration duration)
    {
      const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

      std::ostringstream output;

      if (milliseconds < 1000)
      {
        output << milliseconds << "ms";
        return output.str();
      }

      output << std::fixed << std::setprecision(milliseconds < 10000 ? 2 : 1)
             << (static_cast<double>(milliseconds) / 1000.0) << 's';
      return output.str();
    }

    [[nodiscard]] std::string strip_prefix(std::string value, std::string_view prefix)
    {
      if (value.starts_with(prefix))
      {
        value.erase(0, prefix.size());
      }

      return value;
    }

    [[nodiscard]] std::string compact_task_name(std::string value)
    {
      value = strip_prefix(std::move(value), "Task started: ");
      value = strip_prefix(std::move(value), "Task finished: ");

      constexpr std::size_t max_size = 58;

      if (value.size() <= max_size)
      {
        return value;
      }

      return value.substr(0, max_size - 3) + "...";
    }

    [[nodiscard]] std::string label_for_build(
        const LoadedProject &loaded,
        const cgride::engine::BuildOptions &options)
    {
      if (options.has_target())
      {
        return options.target().value();
      }

      if (!loaded.project.targets().empty() && loaded.project.targets().front() != nullptr)
      {
        return loaded.project.targets().front()->name();
      }

      if (!loaded.project.name().empty() && loaded.project.name() != "cgride_project")
      {
        return loaded.project.name();
      }

      return loaded.project_file.parent_path().filename().string();
    }

    class BuildProgress
    {
    public:
      BuildProgress(
          Terminal &terminal,
          std::string label,
          cgride::engine::BuildMode mode,
          std::size_t total_tasks)
          : terminal_(terminal),
            label_(std::move(label)),
            mode_(mode),
            total_tasks_(total_tasks),
            dynamic_(can_render_dynamic_progress(terminal))
      {
      }

      void start()
      {
        started_at_ = Clock::now();
        last_rendered_at_ = started_at_;

        if (dynamic_)
        {
          current_task_ = "planning";
          render(true);
        }
      }

      void handle(const cgride::core::Event &event)
      {
        switch (event.kind())
        {
        case cgride::core::EventKind::TaskStarted:
          current_task_ = compact_task_name(event.message());
          render(false);
          break;

        case cgride::core::EventKind::TaskFinished:
        case cgride::core::EventKind::TaskSkipped:
          ++completed_tasks_;
          current_task_ = compact_task_name(event.message());
          render(false);
          break;

        default:
          break;
        }
      }

      void finish(const cgride::engine::BuildResult &result)
      {
        const auto elapsed = Clock::now() - started_at_;

        if (dynamic_)
        {
          clear_line();
        }

        if (terminal_.quiet())
        {
          return;
        }

        if (result.failed())
        {
          terminal_.writeln(summary_line("failed", elapsed));
          return;
        }

        terminal_.writeln(summary_line("built", elapsed));
      }

    private:
      [[nodiscard]] std::string summary_line(
          std::string_view status,
          Clock::duration elapsed) const
      {
        std::ostringstream output;
        output << "cgride " << status << ' ' << label_ << " ["
               << cgride::engine::to_string(mode_) << "] in "
               << format_duration(elapsed);

        if (total_tasks_ != 0)
        {
          output << " (" << completed_tasks_ << '/' << total_tasks_ << " tasks)";
        }

        return output.str();
      }

      [[nodiscard]] std::string progress_bar() const
      {
        constexpr std::size_t width = 18;
        const auto filled = total_tasks_ == 0
                                ? std::size_t{0}
                                : std::min(width, (completed_tasks_ * width) / total_tasks_);

        std::string bar;
        bar.reserve(width);

        for (std::size_t index = 0; index < width; ++index)
        {
          bar.push_back(index < filled ? '#' : '.');
        }

        return bar;
      }

      void render(bool force)
      {
        if (!dynamic_)
        {
          return;
        }

        const auto now = Clock::now();

        if (!force && now - last_rendered_at_ < std::chrono::milliseconds(70))
        {
          return;
        }

        last_rendered_at_ = now;
        const char spinner[] = {'-', '\\', '|', '/'};
        const auto spin = spinner[spinner_index_++ % 4];

        std::ostringstream line;
        line << "\r\033[2K" << spin << " cgride "
             << '[' << progress_bar() << "] "
             << completed_tasks_ << '/' << total_tasks_ << ' '
             << label_ << "  " << current_task_ << "  "
             << format_duration(now - started_at_);

        terminal_.write(line.str());
        terminal_.output_stream().flush();
      }

      void clear_line()
      {
        terminal_.write("\r\033[2K");
        terminal_.output_stream().flush();
      }

      Terminal &terminal_;
      std::string label_{};
      cgride::engine::BuildMode mode_{cgride::engine::BuildMode::Debug};
      std::size_t total_tasks_{0};
      std::size_t completed_tasks_{0};
      std::size_t spinner_index_{0};
      std::string current_task_{};
      bool dynamic_{false};
      Clock::time_point started_at_{Clock::now()};
      Clock::time_point last_rendered_at_{Clock::now()};
    };

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

    auto build_options = make_build_options(context, loaded.value().project_root);
    const auto label = label_for_build(loaded.value(), build_options);

    cgride::engine::BuildRequest request;

    request
        .project(std::move(loaded.value().project))
        .toolchain(std::move(toolchain.value()))
        .options(build_options);

    cgride::engine::BuildEngine engine;
    auto planned = engine.plan(request);

    if (!planned)
    {
      return build_error(terminal, planned.error());
    }

    auto plan = std::move(planned.value());
    BuildProgress progress(
        terminal,
        label,
        build_options.mode(),
        plan.graph().size());

    build_options.on_event([&progress](const cgride::core::Event &event) {
      progress.handle(event);
    });

    progress.start();

    auto result = engine.execute(plan, build_options);

    progress.finish(result);

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

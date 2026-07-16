/**
 *
 *  @file cli_options.cpp
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
#include <cgride/cli/cli_options.hpp>

#include <charconv>
#include <string>
#include <utility>

#include <cgride/core/error.hpp>

namespace cgride::cli
{
  namespace
  {
    [[nodiscard]] cgride::core::Error usage_error(
        std::string message)
    {
      return cgride::core::Error(
          cgride::core::ErrorCode::InvalidArgument,
          std::move(message));
    }

    [[nodiscard]] cgride::core::Error usage_error(
        std::string message,
        std::string detail)
    {
      return cgride::core::Error(
          cgride::core::ErrorCode::InvalidArgument,
          std::move(message),
          std::move(detail));
    }

    [[nodiscard]] cgride::core::Result<CliCommand> parse_command(
        std::string_view value)
    {
      if (value == "help")
      {
        return CliCommand::Help;
      }

      if (value == "version")
      {
        return CliCommand::Version;
      }

      if (value == "build")
      {
        return CliCommand::Build;
      }

      if (value == "run")
      {
        return CliCommand::Run;
      }

      return usage_error(
          "Unknown command.",
          std::string(value));
    }

    [[nodiscard]] cgride::core::Result<std::size_t> parse_jobs(
        const std::string &value)
    {
      if (value.empty())
      {
        return usage_error("Option --jobs requires a value.");
      }

      std::size_t jobs = 0;

      const auto *begin = value.data();
      const auto *end = value.data() + value.size();

      const auto result = std::from_chars(begin, end, jobs);

      if (result.ec != std::errc{} || result.ptr != end)
      {
        return usage_error(
            "Option --jobs expects a positive integer.",
            value);
      }

      if (jobs == 0)
      {
        return usage_error(
            "Option --jobs must be greater than zero.",
            value);
      }

      return jobs;
    }

    [[nodiscard]] bool option_is_present_without_value(
        const CommandLine &command_line,
        std::string_view name)
    {
      return command_line.has(name) &&
             !command_line.option_value(name).has_value();
    }

  } // namespace

  std::string_view to_string(CliCommand command) noexcept
  {
    switch (command)
    {
    case CliCommand::Help:
      return "help";

    case CliCommand::Version:
      return "version";

    case CliCommand::Build:
      return "build";

    case CliCommand::Run:
      return "run";
    }

    return "help";
  }

  CliOptions CliOptions::defaults()
  {
    return CliOptions{};
  }

  cgride::core::Result<CliOptions> CliOptions::parse(
      const CommandLine &command_line)
  {
    CliOptions options;

    if (command_line.has("--help") || command_line.has("-h"))
    {
      options.command(CliCommand::Help);
      return options;
    }

    if (command_line.has("--version") || command_line.has("-V"))
    {
      options.command(CliCommand::Version);
      return options;
    }

    const auto positionals = command_line.positional_arguments();

    if (positionals.empty())
    {
      options.command(CliCommand::Help);
    }
    else
    {
      auto command = parse_command(positionals.front());

      if (!command)
      {
        return command.error();
      }

      options.command(command.value());
    }

    if (positionals.size() > 1)
    {
      return usage_error(
          "Unexpected positional argument.",
          positionals[1]);
    }

    if (option_is_present_without_value(command_line, "--root"))
    {
      return usage_error("Option --root requires a value.");
    }

    if (auto value = command_line.option_value("--root"))
    {
      options.project_root(*value);
    }

    if (option_is_present_without_value(command_line, "--config"))
    {
      return usage_error("Option --config requires a value.");
    }

    if (auto value = command_line.option_value("--config"))
    {
      options.config_path(*value);
    }

    if (option_is_present_without_value(command_line, "--target"))
    {
      return usage_error("Option --target requires a value.");
    }

    if (auto value = command_line.option_value("--target"))
    {
      options.target(*value);
    }

    if (option_is_present_without_value(command_line, "--jobs"))
    {
      return usage_error("Option --jobs requires a value.");
    }

    if (auto value = command_line.option_value("--jobs"))
    {
      auto jobs = parse_jobs(*value);

      if (!jobs)
      {
        return jobs.error();
      }

      options.jobs(jobs.value());
    }

    options
        .release(command_line.has("--release"))
        .rebuild(command_line.has("--rebuild"))
        .no_cache(command_line.has("--no-cache"))
        .dry_run(command_line.has("--dry-run"))
        .verbose(command_line.has("--verbose"));

    if (!options.valid())
    {
      return usage_error("Invalid CLI options.");
    }

    return options;
  }

  CliOptions &CliOptions::command(CliCommand command) noexcept
  {
    command_ = command;
    return *this;
  }

  CliOptions &CliOptions::project_root(std::filesystem::path path)
  {
    project_root_ = std::move(path);
    return *this;
  }

  CliOptions &CliOptions::config_path(std::filesystem::path path)
  {
    config_path_ = std::move(path);
    return *this;
  }

  CliOptions &CliOptions::target(std::string target)
  {
    target_ = std::move(target);
    return *this;
  }

  CliOptions &CliOptions::clear_target() noexcept
  {
    target_.reset();
    return *this;
  }

  CliOptions &CliOptions::jobs(std::size_t jobs) noexcept
  {
    jobs_ = jobs;
    return *this;
  }

  CliOptions &CliOptions::release(bool value) noexcept
  {
    release_ = value;
    return *this;
  }

  CliOptions &CliOptions::rebuild(bool value) noexcept
  {
    rebuild_ = value;
    return *this;
  }

  CliOptions &CliOptions::no_cache(bool value) noexcept
  {
    no_cache_ = value;
    return *this;
  }

  CliOptions &CliOptions::dry_run(bool value) noexcept
  {
    dry_run_ = value;
    return *this;
  }

  CliOptions &CliOptions::verbose(bool value) noexcept
  {
    verbose_ = value;
    return *this;
  }

  CliCommand CliOptions::command() const noexcept
  {
    return command_;
  }

  const std::filesystem::path &CliOptions::project_root() const noexcept
  {
    return project_root_;
  }

  const std::filesystem::path &CliOptions::config_path() const noexcept
  {
    return config_path_;
  }

  const std::optional<std::string> &CliOptions::target() const noexcept
  {
    return target_;
  }

  bool CliOptions::has_target() const noexcept
  {
    return target_.has_value();
  }

  std::size_t CliOptions::jobs() const noexcept
  {
    return jobs_;
  }

  bool CliOptions::release() const noexcept
  {
    return release_;
  }

  bool CliOptions::rebuild() const noexcept
  {
    return rebuild_;
  }

  bool CliOptions::no_cache() const noexcept
  {
    return no_cache_;
  }

  bool CliOptions::dry_run() const noexcept
  {
    return dry_run_;
  }

  bool CliOptions::verbose() const noexcept
  {
    return verbose_;
  }

  bool CliOptions::valid() const noexcept
  {
    if (project_root_.empty())
    {
      return false;
    }

    if (config_path_.empty())
    {
      return false;
    }

    if (target_.has_value() && target_->empty())
    {
      return false;
    }

    return true;
  }

} // namespace cgride::cli

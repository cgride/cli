/**
 *
 *  @file command_context.cpp
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
#include <cgride/cli/command_context.hpp>

#include <system_error>
#include <utility>

namespace cgride::cli
{
  namespace
  {
    [[nodiscard]] std::filesystem::path current_directory()
    {
      std::error_code error;

      auto path = std::filesystem::current_path(error);

      if (error || path.empty())
      {
        return ".";
      }

      return path;
    }

  } // namespace

  CommandContext::CommandContext()
      : options_(CliOptions::defaults()),
        working_directory_(current_directory()),
        terminal_()
  {
  }

  CommandContext::CommandContext(CliOptions options)
      : options_(std::move(options)),
        working_directory_(current_directory()),
        terminal_()
  {
  }

  CommandContext::CommandContext(
      CliOptions options,
      Terminal terminal)
      : options_(std::move(options)),
        working_directory_(current_directory()),
        terminal_(terminal)
  {
  }

  CommandContext::CommandContext(
      CliOptions options,
      std::filesystem::path working_directory,
      Terminal terminal)
      : options_(std::move(options)),
        working_directory_(std::move(working_directory)),
        terminal_(terminal)
  {
  }

  CommandContext &CommandContext::options(CliOptions options)
  {
    options_ = std::move(options);
    return *this;
  }

  CommandContext &CommandContext::working_directory(std::filesystem::path path)
  {
    working_directory_ = std::move(path);
    return *this;
  }

  CommandContext &CommandContext::terminal(Terminal terminal)
  {
    terminal_ = terminal;
    return *this;
  }

  CliOptions &CommandContext::options() noexcept
  {
    return options_;
  }

  const CliOptions &CommandContext::options() const noexcept
  {
    return options_;
  }

  Terminal &CommandContext::terminal() noexcept
  {
    return terminal_;
  }

  const Terminal &CommandContext::terminal() const noexcept
  {
    return terminal_;
  }

  const std::filesystem::path &CommandContext::working_directory() const noexcept
  {
    return working_directory_;
  }

  std::filesystem::path CommandContext::resolved_project_root() const
  {
    const auto &root = options_.project_root();

    if (root.is_absolute())
    {
      return root;
    }

    if (working_directory_.empty())
    {
      return root;
    }

    return working_directory_ / root;
  }

  cgride::config::ConfigOptions CommandContext::config_options() const
  {
    cgride::config::ConfigOptions config_options;

    config_options
        .project_root(resolved_project_root())
        .config_path(options_.config_path())
        .strict(true)
        .allow_missing(false);

    return config_options;
  }

  bool CommandContext::valid() const noexcept
  {
    return options_.valid() && !working_directory_.empty();
  }

} // namespace cgride::cli

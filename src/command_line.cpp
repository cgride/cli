/**
 *
 *  @file command_line.cpp
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
#include <cgride/cli/command_line.hpp>

#include <utility>
#include <string>

namespace cgride::cli
{
  namespace
  {
    [[nodiscard]] bool starts_with_option_assignment(
        std::string_view argument,
        std::string_view name) noexcept
    {
      return argument.size() > name.size() + 1 &&
             argument.starts_with(name) &&
             argument[name.size()] == '=';
    }

    [[nodiscard]] bool is_option(std::string_view argument) noexcept
    {
      return argument.size() > 1 &&
             argument[0] == '-';
    }

    [[nodiscard]] bool is_option_value_candidate(
        std::string_view argument) noexcept
    {
      return !argument.empty() &&
             argument != "--" &&
             !is_option(argument);
    }

  } // namespace

  CommandLine::CommandLine(
      std::string program_name,
      std::vector<std::string> arguments)
      : program_name_(std::move(program_name)),
        arguments_(std::move(arguments))
  {
  }

  CommandLine CommandLine::from_argv(
      int argc,
      char **argv)
  {
    return from_argv(
        argc,
        const_cast<const char *const *>(argv));
  }

  CommandLine CommandLine::from_argv(
      int argc,
      const char *const *argv)
  {
    CommandLine command_line;

    if (argc <= 0 || argv == nullptr)
    {
      return command_line;
    }

    if (argv[0] != nullptr)
    {
      command_line.program_name(argv[0]);
    }

    for (int index = 1; index < argc; ++index)
    {
      if (argv[index] == nullptr)
      {
        continue;
      }

      command_line.argument(argv[index]);
    }

    return command_line;
  }

  CommandLine &CommandLine::program_name(std::string name)
  {
    program_name_ = std::move(name);
    return *this;
  }

  CommandLine &CommandLine::argument(std::string argument)
  {
    arguments_.push_back(std::move(argument));
    return *this;
  }

  CommandLine &CommandLine::arguments(std::vector<std::string> arguments)
  {
    arguments_ = std::move(arguments);
    return *this;
  }

  CommandLine &CommandLine::clear_arguments() noexcept
  {
    arguments_.clear();
    return *this;
  }

  const std::string &CommandLine::program_name() const noexcept
  {
    return program_name_;
  }

  const std::vector<std::string> &CommandLine::arguments() const noexcept
  {
    return arguments_;
  }

  std::optional<std::string> CommandLine::argument(
      std::size_t index) const
  {
    if (index >= arguments_.size())
    {
      return std::nullopt;
    }

    return arguments_[index];
  }

  std::size_t CommandLine::argument_count() const noexcept
  {
    return arguments_.size();
  }

  bool CommandLine::empty() const noexcept
  {
    return arguments_.empty();
  }

  bool CommandLine::valid() const noexcept
  {
    return !program_name_.empty();
  }

  bool CommandLine::contains(std::string_view value) const noexcept
  {
    for (const auto &argument : arguments_)
    {
      if (argument == value)
      {
        return true;
      }
    }

    return false;
  }

  bool CommandLine::has(std::string_view name) const noexcept
  {
    return has_flag(name);
  }

  bool CommandLine::has_flag(std::string_view name) const noexcept
  {
    if (name.empty())
    {
      return false;
    }

    for (const auto &argument : arguments_)
    {
      if (argument == "--")
      {
        break;
      }

      if (argument == name || starts_with_option_assignment(argument, name))
      {
        return true;
      }
    }

    return false;
  }

  std::optional<std::string> CommandLine::option_value(
      std::string_view name) const
  {
    if (name.empty())
    {
      return std::nullopt;
    }

    for (std::size_t index = 0; index < arguments_.size(); ++index)
    {
      const auto &argument = arguments_[index];

      if (argument == "--")
      {
        break;
      }

      if (starts_with_option_assignment(argument, name))
      {
        return argument.substr(name.size() + 1);
      }

      if (argument == name)
      {
        const auto next_index = index + 1;

        if (next_index >= arguments_.size())
        {
          return std::nullopt;
        }

        if (!is_option_value_candidate(arguments_[next_index]))
        {
          return std::nullopt;
        }

        return arguments_[next_index];
      }
    }

    return std::nullopt;
  }

  std::vector<std::string> CommandLine::positional_arguments() const
  {
    std::vector<std::string> positionals;
    bool after_separator = false;

    for (std::size_t index = 0; index < arguments_.size(); ++index)
    {
      const auto &argument = arguments_[index];

      if (after_separator)
      {
        positionals.push_back(argument);
        continue;
      }

      if (argument == "--")
      {
        after_separator = true;
        continue;
      }

      if (!is_option(argument))
      {
        positionals.push_back(argument);
        continue;
      }

      if (argument.find('=') != std::string::npos)
      {
        continue;
      }

      const auto next_index = index + 1;

      if (next_index < arguments_.size() &&
          is_option_value_candidate(arguments_[next_index]))
      {
        ++index;
      }
    }

    return positionals;
  }

} // namespace cgride::cli

/**
 *
 *  @file command_line.hpp
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
#ifndef CGRIDE_CLI_COMMAND_LINE_HPP
#define CGRIDE_CLI_COMMAND_LINE_HPP

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace cgride::cli
{
  /**
   * @class CommandLine
   * @brief Small value object around argc/argv.
   *
   * CommandLine keeps the raw command-line arguments in a convenient form. It
   * does not decide which command should run and it does not perform CLI
   * validation by itself.
   */
  class CommandLine
  {
  public:
    /**
     * @brief Construct an empty command line.
     */
    CommandLine() = default;

    /**
     * @brief Construct a command line from a program name and arguments.
     *
     * @param program_name Program name.
     * @param arguments Command-line arguments without program name.
     */
    CommandLine(
        std::string program_name,
        std::vector<std::string> arguments);

    /**
     * @brief Create a command line from argc/argv.
     *
     * @param argc Argument count.
     * @param argv Argument values.
     * @return Parsed command line container.
     */
    [[nodiscard]] static CommandLine from_argv(
        int argc,
        char **argv);

    /**
     * @brief Create a command line from argc/argv.
     *
     * @param argc Argument count.
     * @param argv Argument values.
     * @return Parsed command line container.
     */
    [[nodiscard]] static CommandLine from_argv(
        int argc,
        const char *const *argv);

    /**
     * @brief Set the program name.
     *
     * @param name Program name.
     * @return Reference to this command line.
     */
    CommandLine &program_name(std::string name);

    /**
     * @brief Add one argument.
     *
     * @param argument Argument value.
     * @return Reference to this command line.
     */
    CommandLine &argument(std::string argument);

    /**
     * @brief Replace all arguments.
     *
     * @param arguments Argument values without program name.
     * @return Reference to this command line.
     */
    CommandLine &arguments(std::vector<std::string> arguments);

    /**
     * @brief Clear all arguments but keep the program name.
     *
     * @return Reference to this command line.
     */
    CommandLine &clear_arguments() noexcept;

    /**
     * @brief Access the program name.
     */
    [[nodiscard]] const std::string &program_name() const noexcept;

    /**
     * @brief Access all arguments without program name.
     */
    [[nodiscard]] const std::vector<std::string> &arguments() const noexcept;

    /**
     * @brief Return one argument by index.
     *
     * @param index Argument index.
     * @return Argument value when available.
     */
    [[nodiscard]] std::optional<std::string> argument(
        std::size_t index) const;

    /**
     * @brief Return the number of arguments without program name.
     */
    [[nodiscard]] std::size_t argument_count() const noexcept;

    /**
     * @brief Return true when there are no arguments.
     */
    [[nodiscard]] bool empty() const noexcept;

    /**
     * @brief Return true when a program name is available.
     */
    [[nodiscard]] bool valid() const noexcept;

    /**
     * @brief Return true when an exact argument exists.
     *
     * @param value Argument value.
     * @return True when the argument exists.
     */
    [[nodiscard]] bool contains(std::string_view value) const noexcept;

    /**
     * @brief Return true when an option or flag exists.
     *
     * Supports both `--name` and `--name=value`.
     *
     * @param name Option name.
     * @return True when the option exists.
     */
    [[nodiscard]] bool has(std::string_view name) const noexcept;

    /**
     * @brief Return true when an option or flag exists.
     *
     * @param name Option name.
     * @return True when the option exists.
     */
    [[nodiscard]] bool has_flag(std::string_view name) const noexcept;

    /**
     * @brief Return the value for an option.
     *
     * Supports both `--name value` and `--name=value`.
     *
     * @param name Option name.
     * @return Option value when available.
     */
    [[nodiscard]] std::optional<std::string> option_value(
        std::string_view name) const;

    /**
     * @brief Return positional arguments.
     *
     * Options and their values are skipped. Values after `--` are positional.
     *
     * @return Positional argument values.
     */
    [[nodiscard]] std::vector<std::string> positional_arguments() const;

  private:
    std::string program_name_{};
    std::vector<std::string> arguments_{};
  };

} // namespace cgride::cli

#endif // CGRIDE_CLI_COMMAND_LINE_HPP

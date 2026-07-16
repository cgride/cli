/**
 *
 *  @file command_context.hpp
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
#ifndef CGRIDE_CLI_COMMAND_CONTEXT_HPP
#define CGRIDE_CLI_COMMAND_CONTEXT_HPP

#include <filesystem>

#include <cgride/cli/cli_options.hpp>
#include <cgride/cli/terminal.hpp>
#include <cgride/config/config_options.hpp>

namespace cgride::cli
{
  /**
   * @class CommandContext
   * @brief Shared execution context passed to CLI commands.
   *
   * CommandContext keeps the parsed options, the terminal wrapper, and the
   * current working directory together. It contains no command dispatch logic.
   */
  class CommandContext
  {
  public:
    /**
     * @brief Construct a default command context.
     */
    CommandContext();

    /**
     * @brief Construct a command context from CLI options.
     *
     * @param options Parsed CLI options.
     */
    explicit CommandContext(CliOptions options);

    /**
     * @brief Construct a command context from CLI options and terminal.
     *
     * @param options Parsed CLI options.
     * @param terminal Terminal wrapper.
     */
    CommandContext(
        CliOptions options,
        Terminal terminal);

    /**
     * @brief Construct a command context from all values.
     *
     * @param options Parsed CLI options.
     * @param working_directory Current working directory.
     * @param terminal Terminal wrapper.
     */
    CommandContext(
        CliOptions options,
        std::filesystem::path working_directory,
        Terminal terminal);

    /**
     * @brief Set CLI options.
     *
     * @param options Parsed CLI options.
     * @return Reference to this context.
     */
    CommandContext &options(CliOptions options);

    /**
     * @brief Set the working directory.
     *
     * @param path Working directory.
     * @return Reference to this context.
     */
    CommandContext &working_directory(std::filesystem::path path);

    /**
     * @brief Set the terminal wrapper.
     *
     * @param terminal Terminal wrapper.
     * @return Reference to this context.
     */
    CommandContext &terminal(Terminal terminal);

    /**
     * @brief Access mutable CLI options.
     */
    [[nodiscard]] CliOptions &options() noexcept;

    /**
     * @brief Access immutable CLI options.
     */
    [[nodiscard]] const CliOptions &options() const noexcept;

    /**
     * @brief Access mutable terminal wrapper.
     */
    [[nodiscard]] Terminal &terminal() noexcept;

    /**
     * @brief Access immutable terminal wrapper.
     */
    [[nodiscard]] const Terminal &terminal() const noexcept;

    /**
     * @brief Access the working directory.
     */
    [[nodiscard]] const std::filesystem::path &working_directory() const noexcept;

    /**
     * @brief Resolve the configured project root against the working directory.
     *
     * @return Absolute or working-directory-relative project root.
     */
    [[nodiscard]] std::filesystem::path resolved_project_root() const;

    /**
     * @brief Create config loading options from CLI options.
     *
     * @return Config module options.
     */
    [[nodiscard]] cgride::config::ConfigOptions config_options() const;

    /**
     * @brief Return true when the context is structurally valid.
     */
    [[nodiscard]] bool valid() const noexcept;

  private:
    CliOptions options_{};
    std::filesystem::path working_directory_{};
    Terminal terminal_{};
  };

} // namespace cgride::cli

#endif // CGRIDE_CLI_COMMAND_CONTEXT_HPP

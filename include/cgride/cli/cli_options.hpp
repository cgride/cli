/**
 *
 *  @file cli_options.hpp
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
#ifndef CGRIDE_CLI_CLI_OPTIONS_HPP
#define CGRIDE_CLI_CLI_OPTIONS_HPP

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

#include <cgride/cli/command_line.hpp>
#include <cgride/core/result.hpp>

namespace cgride::cli
{
  /**
   * @enum CliCommand
   * @brief Top-level CLI command selected by the user.
   */
  enum class CliCommand
  {
    Help,
    Version,
    Build,
    Run
  };

  /**
   * @brief Convert a CLI command to a stable string.
   *
   * @param command CLI command.
   * @return Stable command string.
   */
  [[nodiscard]] std::string_view to_string(CliCommand command) noexcept;

  /**
   * @class CliOptions
   * @brief Parsed user-facing CLI options.
   *
   * This class stores the command and flags after parsing the command line. It
   * does not execute any build action.
   */
  class CliOptions
  {
  public:
    /**
     * @brief Construct default CLI options.
     */
    CliOptions() = default;

    /**
     * @brief Return default CLI options.
     */
    [[nodiscard]] static CliOptions defaults();

    /**
     * @brief Parse CLI options from a command line.
     *
     * @param command_line Command line.
     * @return Parsed CLI options or an error.
     */
    [[nodiscard]] static cgride::core::Result<CliOptions> parse(
        const CommandLine &command_line);

    /**
     * @brief Set the selected command.
     */
    CliOptions &command(CliCommand command) noexcept;

    /**
     * @brief Set the project root directory.
     */
    CliOptions &project_root(std::filesystem::path path);

    /**
     * @brief Set the config file path.
     */
    CliOptions &config_path(std::filesystem::path path);

    /**
     * @brief Set the selected build target.
     */
    CliOptions &target(std::string target);

    /**
     * @brief Clear the selected build target.
     */
    CliOptions &clear_target() noexcept;

    /**
     * @brief Set the number of build jobs.
     */
    CliOptions &jobs(std::size_t jobs) noexcept;

    /**
     * @brief Set release mode.
     */
    CliOptions &release(bool value) noexcept;

    /**
     * @brief Set rebuild mode.
     */
    CliOptions &rebuild(bool value) noexcept;

    /**
     * @brief Set no-cache mode.
     */
    CliOptions &no_cache(bool value) noexcept;

    /**
     * @brief Set dry-run mode.
     */
    CliOptions &dry_run(bool value) noexcept;

    /**
     * @brief Set verbose mode.
     */
    CliOptions &verbose(bool value) noexcept;

    /**
     * @brief Access the selected command.
     */
    [[nodiscard]] CliCommand command() const noexcept;

    /**
     * @brief Access the project root directory.
     */
    [[nodiscard]] const std::filesystem::path &project_root() const noexcept;

    /**
     * @brief Access the config file path.
     */
    [[nodiscard]] const std::filesystem::path &config_path() const noexcept;

    /**
     * @brief Access the selected build target.
     */
    [[nodiscard]] const std::optional<std::string> &target() const noexcept;

    /**
     * @brief Return true when a target was selected.
     */
    [[nodiscard]] bool has_target() const noexcept;

    /**
     * @brief Access the number of build jobs.
     *
     * A value of 0 means automatic/default job selection.
     */
    [[nodiscard]] std::size_t jobs() const noexcept;

    /**
     * @brief Return true when release mode is enabled.
     */
    [[nodiscard]] bool release() const noexcept;

    /**
     * @brief Return true when rebuild mode is enabled.
     */
    [[nodiscard]] bool rebuild() const noexcept;

    /**
     * @brief Return true when cache usage is disabled.
     */
    [[nodiscard]] bool no_cache() const noexcept;

    /**
     * @brief Return true when dry-run mode is enabled.
     */
    [[nodiscard]] bool dry_run() const noexcept;

    /**
     * @brief Return true when verbose mode is enabled.
     */
    [[nodiscard]] bool verbose() const noexcept;

    /**
     * @brief Return true when the options are structurally valid.
     */
    [[nodiscard]] bool valid() const noexcept;

  private:
    CliCommand command_{CliCommand::Help};
    std::filesystem::path project_root_{"."};
    std::filesystem::path config_path_{"cgride.config"};
    std::optional<std::string> target_{};
    std::size_t jobs_{0};
    bool release_{false};
    bool rebuild_{false};
    bool no_cache_{false};
    bool dry_run_{false};
    bool verbose_{false};
  };

} // namespace cgride::cli

#endif // CGRIDE_CLI_CLI_OPTIONS_HPP

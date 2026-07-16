/**
 *
 *  @file terminal.hpp
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
#ifndef CGRIDE_CLI_TERMINAL_HPP
#define CGRIDE_CLI_TERMINAL_HPP

#include <iosfwd>
#include <string_view>

namespace cgride::cli
{
  /**
   * @class Terminal
   * @brief Small terminal output wrapper for the CLI.
   *
   * Terminal owns no build logic. It only centralizes user-facing output so
   * commands can be tested without writing directly to std::cout or std::cerr.
   */
  class Terminal
  {
  public:
    /**
     * @brief Construct a terminal using std::cout and std::cerr.
     */
    Terminal();

    /**
     * @brief Construct a terminal from custom streams.
     *
     * @param output Output stream.
     * @param error Error stream.
     */
    Terminal(
        std::ostream &output,
        std::ostream &error);

    /**
     * @brief Set the output stream.
     *
     * @param stream Output stream.
     * @return Reference to this terminal.
     */
    Terminal &output_stream(std::ostream &stream) noexcept;

    /**
     * @brief Set the error stream.
     *
     * @param stream Error stream.
     * @return Reference to this terminal.
     */
    Terminal &error_stream(std::ostream &stream) noexcept;

    /**
     * @brief Set quiet mode.
     *
     * Quiet mode suppresses normal output but keeps errors visible.
     *
     * @param value Quiet mode value.
     * @return Reference to this terminal.
     */
    Terminal &quiet(bool value) noexcept;

    /**
     * @brief Set verbose mode.
     *
     * @param value Verbose mode value.
     * @return Reference to this terminal.
     */
    Terminal &verbose(bool value) noexcept;

    /**
     * @brief Access the output stream.
     */
    [[nodiscard]] std::ostream &output_stream() const noexcept;

    /**
     * @brief Access the error stream.
     */
    [[nodiscard]] std::ostream &error_stream() const noexcept;

    /**
     * @brief Return true when quiet mode is enabled.
     */
    [[nodiscard]] bool quiet() const noexcept;

    /**
     * @brief Return true when verbose mode is enabled.
     */
    [[nodiscard]] bool verbose() const noexcept;

    /**
     * @brief Write text to the output stream.
     *
     * @param text Text to write.
     * @return Reference to this terminal.
     */
    Terminal &write(std::string_view text);

    /**
     * @brief Write text and a newline to the output stream.
     *
     * @param text Text to write.
     * @return Reference to this terminal.
     */
    Terminal &writeln(std::string_view text = {});

    /**
     * @brief Write text to the error stream.
     *
     * @param text Text to write.
     * @return Reference to this terminal.
     */
    Terminal &write_error(std::string_view text);

    /**
     * @brief Write text and a newline to the error stream.
     *
     * @param text Text to write.
     * @return Reference to this terminal.
     */
    Terminal &writeln_error(std::string_view text = {});

    /**
     * @brief Print an informational message.
     *
     * @param message Message to print.
     * @return Reference to this terminal.
     */
    Terminal &print_info(std::string_view message);

    /**
     * @brief Print a success message.
     *
     * @param message Message to print.
     * @return Reference to this terminal.
     */
    Terminal &print_success(std::string_view message);

    /**
     * @brief Print a warning message.
     *
     * @param message Message to print.
     * @return Reference to this terminal.
     */
    Terminal &print_warning(std::string_view message);

    /**
     * @brief Print an error message.
     *
     * @param message Message to print.
     * @return Reference to this terminal.
     */
    Terminal &print_error(std::string_view message);

    /**
     * @brief Print a verbose message only when verbose mode is enabled.
     *
     * @param message Message to print.
     * @return Reference to this terminal.
     */
    Terminal &print_verbose(std::string_view message);

  private:
    std::ostream *output_stream_{};
    std::ostream *error_stream_{};
    bool quiet_{false};
    bool verbose_{false};
  };

} // namespace cgride::cli

#endif // CGRIDE_CLI_TERMINAL_HPP

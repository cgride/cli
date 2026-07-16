/**
 *
 *  @file application.hpp
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
#ifndef CGRIDE_CLI_APPLICATION_HPP
#define CGRIDE_CLI_APPLICATION_HPP

#include <cgride/cli/command_line.hpp>
#include <cgride/cli/exit_code.hpp>
#include <cgride/cli/terminal.hpp>
#include <cgride/core/error.hpp>

namespace cgride::cli
{
  /**
   * @class Application
   * @brief Main CLI application dispatcher.
   *
   * Application parses command-line arguments, prints help/version output, and
   * dispatches supported commands. It does not contain build logic directly.
   */
  class Application
  {
  public:
    /**
     * @brief Construct an application using the default terminal.
     */
    Application();

    /**
     * @brief Construct an application using a custom terminal.
     *
     * @param terminal Terminal wrapper.
     */
    explicit Application(Terminal terminal);

    /**
     * @brief Run the application from argc/argv.
     *
     * @param argc Argument count.
     * @param argv Argument values.
     * @return Process exit code.
     */
    [[nodiscard]] ExitCode run(
        int argc,
        char **argv);

    /**
     * @brief Run the application from a command line object.
     *
     * @param command_line Command line.
     * @return Process exit code.
     */
    [[nodiscard]] ExitCode run(
        const CommandLine &command_line);

    /**
     * @brief Access mutable terminal.
     */
    [[nodiscard]] Terminal &terminal() noexcept;

    /**
     * @brief Access immutable terminal.
     */
    [[nodiscard]] const Terminal &terminal() const noexcept;

  private:
    void print_help();
    void print_version();
    void print_usage_error(const cgride::core::Error &error);

    Terminal terminal_{};
  };

} // namespace cgride::cli

#endif // CGRIDE_CLI_APPLICATION_HPP

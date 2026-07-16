/**
 *
 *  @file build_command.hpp
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
#ifndef CGRIDE_CLI_BUILD_COMMAND_HPP
#define CGRIDE_CLI_BUILD_COMMAND_HPP

#include <cgride/cli/command_context.hpp>
#include <cgride/cli/exit_code.hpp>

namespace cgride::cli
{
  /**
   * @class BuildCommand
   * @brief Implements the `cgride build` command.
   *
   * BuildCommand is the CLI bridge between user input and the build engine. It
   * reads the project config, creates a build request, and delegates actual
   * build work to `cgride::engine`.
   */
  class BuildCommand
  {
  public:
    /**
     * @brief Run the build command.
     *
     * @param context Shared CLI command context.
     * @return Process exit code.
     */
    [[nodiscard]] ExitCode run(CommandContext &context) const;
  };

} // namespace cgride::cli

#endif // CGRIDE_CLI_BUILD_COMMAND_HPP

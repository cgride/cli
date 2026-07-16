/**
 *
 *  @file run_command.hpp
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
#ifndef CGRIDE_CLI_RUN_COMMAND_HPP
#define CGRIDE_CLI_RUN_COMMAND_HPP

#include <cgride/cli/command_context.hpp>
#include <cgride/cli/exit_code.hpp>

namespace cgride::cli
{
  /**
   * @class RunCommand
   * @brief Implements the `cgride run` command.
   *
   * The run command builds the project first. Launching the produced executable
   * will be added when executable artifact mapping is exposed by the build
   * engine.
   */
  class RunCommand
  {
  public:
    /**
     * @brief Run the command.
     *
     * @param context Shared CLI command context.
     * @return Process exit code.
     */
    [[nodiscard]] ExitCode run(CommandContext &context) const;
  };

} // namespace cgride::cli

#endif // CGRIDE_CLI_RUN_COMMAND_HPP

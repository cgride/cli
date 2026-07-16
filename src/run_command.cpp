/**
 *
 *  @file run_command.cpp
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
#include <cgride/cli/run_command.hpp>
#include <cgride/cli/build_command.hpp>

namespace cgride::cli
{
  ExitCode RunCommand::run(CommandContext &context) const
  {
    auto &terminal = context.terminal();

    terminal.print_verbose("Running build step before launch.");

    BuildCommand build_command;

    const auto build_result = build_command.run(context);

    if (failed(build_result))
    {
      return build_result;
    }

    terminal.print_info("Run step is not available yet.");
    terminal.print_info("The project was built successfully.");

    return ExitCode::Success;
  }

} // namespace cgride::cli

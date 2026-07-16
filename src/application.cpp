/**
 *
 *  @file application.cpp
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
#include <cgride/cli/application.hpp>

#include <string>
#include <utility>

#include <cgride/cli/build_command.hpp>
#include <cgride/cli/cli_options.hpp>
#include <cgride/cli/command_context.hpp>
#include <cgride/cli/run_command.hpp>
#include <cgride/cli/version.hpp>

namespace cgride::cli
{
  Application::Application()
      : terminal_()
  {
  }

  Application::Application(Terminal terminal)
      : terminal_(terminal)
  {
  }

  ExitCode Application::run(
      int argc,
      char **argv)
  {
    return run(CommandLine::from_argv(argc, argv));
  }

  ExitCode Application::run(
      const CommandLine &command_line)
  {
    auto parsed_options = CliOptions::parse(command_line);

    if (!parsed_options)
    {
      print_usage_error(parsed_options.error());
      print_help();
      return ExitCode::UsageError;
    }

    auto options = parsed_options.value();

    terminal_.verbose(options.verbose());

    switch (options.command())
    {
    case CliCommand::Help:
      print_help();
      return ExitCode::Success;

    case CliCommand::Version:
      print_version();
      return ExitCode::Success;

    case CliCommand::Build:
    {
      CommandContext context(options, terminal_);

      BuildCommand command;
      return command.run(context);
    }

    case CliCommand::Run:
    {
      CommandContext context(options, terminal_);

      RunCommand command;
      return command.run(context);
    }
    }

    terminal_.print_error("Unsupported command.");
    return ExitCode::UsageError;
  }

  Terminal &Application::terminal() noexcept
  {
    return terminal_;
  }

  const Terminal &Application::terminal() const noexcept
  {
    return terminal_;
  }

  void Application::print_help()
  {
    terminal_
        .writeln("Cgride")
        .writeln()
        .writeln("Usage:")
        .writeln("  cgride <command> [options]")
        .writeln()
        .writeln("Commands:")
        .writeln("  build              Build the current project")
        .writeln("  run                Build the project and run the produced executable later")
        .writeln("  help               Show this help message")
        .writeln("  version            Show the Cgride CLI version")
        .writeln()
        .writeln("Options:")
        .writeln("  --root <path>      Project root directory")
        .writeln("  --config <path>    Config file path")
        .writeln("  --target <name>    Build a specific target")
        .writeln("  --release          Build in release mode")
        .writeln("  --jobs <count>     Number of build jobs")
        .writeln("  --rebuild          Force a rebuild")
        .writeln("  --no-cache         Disable build cache usage")
        .writeln("  --dry-run          Prepare the build without executing it")
        .writeln("  --verbose          Print verbose output")
        .writeln("  --help, -h         Show this help message")
        .writeln("  --version, -V      Show version information");
  }

  void Application::print_version()
  {
    terminal_.writeln("cgride " + std::string(version_string));
  }

  void Application::print_usage_error(
      const cgride::core::Error &error)
  {
    terminal_.print_error(error.message());

    if (error.detail().has_value() && !error.detail().value().empty())
    {
      terminal_.writeln_error("detail: " + error.detail().value());
    }

    if (error.path().has_value() && !error.path().value().empty())
    {
      terminal_.writeln_error("path: " + error.path().value().string());
    }
  }

} // namespace cgride::cli

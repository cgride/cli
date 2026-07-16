/**
 *
 *  @file command_line_test.cpp
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
#include <cassert>
#include <string>
#include <vector>

#include <cgride/cli/command_line.hpp>

int main()
{
  {
    cgride::cli::CommandLine command_line;

    assert(command_line.program_name().empty());
    assert(command_line.arguments().empty());
    assert(command_line.argument_count() == 0);
    assert(command_line.empty());
    assert(!command_line.valid());
    assert(!command_line.argument(0).has_value());
    assert(!command_line.contains("build"));
    assert(!command_line.has("--help"));
    assert(!command_line.has_flag("--help"));
    assert(!command_line.option_value("--config").has_value());
    assert(command_line.positional_arguments().empty());
  }

  {
    cgride::cli::CommandLine command_line(
        "cgride",
        {"build", "--release", "--verbose"});

    assert(command_line.program_name() == "cgride");
    assert(command_line.argument_count() == 3);
    assert(!command_line.empty());
    assert(command_line.valid());

    assert(command_line.argument(0).value() == "build");
    assert(command_line.argument(1).value() == "--release");
    assert(command_line.argument(2).value() == "--verbose");
    assert(!command_line.argument(3).has_value());

    assert(command_line.contains("build"));
    assert(command_line.contains("--release"));
    assert(command_line.contains("--verbose"));
    assert(!command_line.contains("run"));

    assert(command_line.has("--release"));
    assert(command_line.has_flag("--verbose"));
    assert(!command_line.has("--config"));
  }

  {
    cgride::cli::CommandLine command_line;

    command_line
        .program_name("cgride")
        .argument("build")
        .argument("--config")
        .argument("cgride.config");

    assert(command_line.program_name() == "cgride");
    assert(command_line.argument_count() == 3);
    assert(command_line.valid());

    assert(command_line.has("--config"));
    assert(command_line.option_value("--config").has_value());
    assert(command_line.option_value("--config").value() == "cgride.config");

    command_line.clear_arguments();

    assert(command_line.argument_count() == 0);
    assert(command_line.empty());
    assert(command_line.valid());
  }

  {
    cgride::cli::CommandLine command_line;

    command_line
        .program_name("cgride")
        .arguments({"build", "--root", ".", "--config", "build.cgride"});

    assert(command_line.argument_count() == 5);
    assert(command_line.option_value("--root").has_value());
    assert(command_line.option_value("--root").value() == ".");
    assert(command_line.option_value("--config").has_value());
    assert(command_line.option_value("--config").value() == "build.cgride");
  }

  {
    cgride::cli::CommandLine command_line(
        "cgride",
        {"build", "--config=cgride.config", "--target=app"});

    assert(command_line.has("--config"));
    assert(command_line.has("--target"));

    assert(command_line.option_value("--config").has_value());
    assert(command_line.option_value("--config").value() == "cgride.config");

    assert(command_line.option_value("--target").has_value());
    assert(command_line.option_value("--target").value() == "app");
  }

  {
    cgride::cli::CommandLine command_line(
        "cgride",
        {"build", "--config", "cgride.config", "--target", "app"});

    assert(command_line.option_value("--config").has_value());
    assert(command_line.option_value("--config").value() == "cgride.config");

    assert(command_line.option_value("--target").has_value());
    assert(command_line.option_value("--target").value() == "app");
  }

  {
    cgride::cli::CommandLine command_line(
        "cgride",
        {"build", "--config"});

    assert(command_line.has("--config"));
    assert(!command_line.option_value("--config").has_value());
  }

  {
    cgride::cli::CommandLine command_line(
        "cgride",
        {"build", "--config", "--release"});

    assert(command_line.has("--config"));
    assert(!command_line.option_value("--config").has_value());
    assert(command_line.has("--release"));
  }

  {
    cgride::cli::CommandLine command_line(
        "cgride",
        {"build", "--root", ".", "--config", "cgride.config"});

    const auto positionals = command_line.positional_arguments();

    assert(positionals.size() == 1);
    assert(positionals[0] == "build");
  }

  {
    cgride::cli::CommandLine command_line(
        "cgride",
        {"build", "--target=app", "--release"});

    const auto positionals = command_line.positional_arguments();

    assert(positionals.size() == 1);
    assert(positionals[0] == "build");
  }

  {
    cgride::cli::CommandLine command_line(
        "cgride",
        {"build", "--", "--not-an-option", "value"});

    const auto positionals = command_line.positional_arguments();

    assert(positionals.size() == 3);
    assert(positionals[0] == "build");
    assert(positionals[1] == "--not-an-option");
    assert(positionals[2] == "value");

    assert(!command_line.has("--not-an-option"));
    assert(!command_line.option_value("--not-an-option").has_value());
  }

  {
    const char *argv[] = {
        "cgride",
        "build",
        "--config",
        "cgride.config",
        "--release"};

    auto command_line = cgride::cli::CommandLine::from_argv(5, argv);

    assert(command_line.program_name() == "cgride");
    assert(command_line.argument_count() == 4);
    assert(command_line.argument(0).value() == "build");
    assert(command_line.option_value("--config").value() == "cgride.config");
    assert(command_line.has("--release"));
  }

  {
    auto command_line = cgride::cli::CommandLine::from_argv(
        0,
        static_cast<const char *const *>(nullptr));

    assert(command_line.program_name().empty());
    assert(command_line.arguments().empty());
    assert(!command_line.valid());
  }

  {
    const char *argv[] = {
        "cgride",
        nullptr,
        "build",
        nullptr,
        "--help"};

    auto command_line = cgride::cli::CommandLine::from_argv(5, argv);

    assert(command_line.program_name() == "cgride");
    assert(command_line.argument_count() == 2);
    assert(command_line.argument(0).value() == "build");
    assert(command_line.argument(1).value() == "--help");
    assert(command_line.has("--help"));
  }

  return 0;
}

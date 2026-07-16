/**
 *
 *  @file cli_options_test.cpp
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
#include <filesystem>
#include <string_view>

#include <cgride/cli/cli_options.hpp>
#include <cgride/core/error.hpp>

int main()
{
  {
    assert(cgride::cli::to_string(cgride::cli::CliCommand::Help) == std::string_view("help"));
    assert(cgride::cli::to_string(cgride::cli::CliCommand::Version) == std::string_view("version"));
    assert(cgride::cli::to_string(cgride::cli::CliCommand::Build) == std::string_view("build"));
    assert(cgride::cli::to_string(cgride::cli::CliCommand::Run) == std::string_view("run"));
  }

  {
    auto options = cgride::cli::CliOptions::defaults();

    assert(options.command() == cgride::cli::CliCommand::Help);
    assert(options.project_root() == std::filesystem::path("."));
    assert(options.config_path() == std::filesystem::path("cgride.config"));
    assert(!options.has_target());
    assert(!options.target().has_value());
    assert(options.jobs() == 0);
    assert(!options.release());
    assert(!options.rebuild());
    assert(!options.no_cache());
    assert(!options.dry_run());
    assert(!options.verbose());
    assert(options.valid());
  }

  {
    cgride::cli::CliOptions options;

    options
        .command(cgride::cli::CliCommand::Build)
        .project_root("examples/app")
        .config_path("project.cgride")
        .target("app")
        .jobs(8)
        .release(true)
        .rebuild(true)
        .no_cache(true)
        .dry_run(true)
        .verbose(true);

    assert(options.command() == cgride::cli::CliCommand::Build);
    assert(options.project_root() == std::filesystem::path("examples/app"));
    assert(options.config_path() == std::filesystem::path("project.cgride"));
    assert(options.has_target());
    assert(options.target().value() == "app");
    assert(options.jobs() == 8);
    assert(options.release());
    assert(options.rebuild());
    assert(options.no_cache());
    assert(options.dry_run());
    assert(options.verbose());
    assert(options.valid());

    options.clear_target();

    assert(!options.has_target());
    assert(!options.target().has_value());
    assert(options.valid());
  }

  {
    cgride::cli::CliOptions options;

    options.project_root({});

    assert(!options.valid());

    options.project_root(".");

    assert(options.valid());

    options.config_path({});

    assert(!options.valid());

    options.config_path("cgride.config");

    assert(options.valid());

    options.target("");

    assert(!options.valid());
  }

  {
    cgride::cli::CommandLine command_line("cgride", {});

    auto result = cgride::cli::CliOptions::parse(command_line);

    assert(result);

    auto options = result.value();

    assert(options.command() == cgride::cli::CliCommand::Help);
    assert(options.valid());
  }

  {
    cgride::cli::CommandLine command_line("cgride", {"help"});

    auto result = cgride::cli::CliOptions::parse(command_line);

    assert(result);
    assert(result.value().command() == cgride::cli::CliCommand::Help);
  }

  {
    cgride::cli::CommandLine command_line("cgride", {"version"});

    auto result = cgride::cli::CliOptions::parse(command_line);

    assert(result);
    assert(result.value().command() == cgride::cli::CliCommand::Version);
  }

  {
    cgride::cli::CommandLine command_line("cgride", {"build"});

    auto result = cgride::cli::CliOptions::parse(command_line);

    assert(result);

    auto options = result.value();

    assert(options.command() == cgride::cli::CliCommand::Build);
    assert(options.project_root() == std::filesystem::path("."));
    assert(options.config_path() == std::filesystem::path("cgride.config"));
    assert(options.jobs() == 0);
    assert(!options.release());
    assert(options.valid());
  }

  {
    cgride::cli::CommandLine command_line("cgride", {"run"});

    auto result = cgride::cli::CliOptions::parse(command_line);

    assert(result);
    assert(result.value().command() == cgride::cli::CliCommand::Run);
  }

  {
    cgride::cli::CommandLine command_line(
        "cgride",
        {
            "build",
            "--root",
            "examples/app",
            "--config",
            "project.cgride",
            "--target",
            "app",
            "--jobs",
            "12",
            "--release",
            "--rebuild",
            "--no-cache",
            "--dry-run",
            "--verbose",
        });

    auto result = cgride::cli::CliOptions::parse(command_line);

    assert(result);

    auto options = result.value();

    assert(options.command() == cgride::cli::CliCommand::Build);
    assert(options.project_root() == std::filesystem::path("examples/app"));
    assert(options.config_path() == std::filesystem::path("project.cgride"));
    assert(options.has_target());
    assert(options.target().value() == "app");
    assert(options.jobs() == 12);
    assert(options.release());
    assert(options.rebuild());
    assert(options.no_cache());
    assert(options.dry_run());
    assert(options.verbose());
    assert(options.valid());
  }

  {
    cgride::cli::CommandLine command_line(
        "cgride",
        {
            "build",
            "--root=examples/app",
            "--config=project.cgride",
            "--target=app",
            "--jobs=4",
            "--release",
        });

    auto result = cgride::cli::CliOptions::parse(command_line);

    assert(result);

    auto options = result.value();

    assert(options.command() == cgride::cli::CliCommand::Build);
    assert(options.project_root() == std::filesystem::path("examples/app"));
    assert(options.config_path() == std::filesystem::path("project.cgride"));
    assert(options.target().value() == "app");
    assert(options.jobs() == 4);
    assert(options.release());
  }

  {
    cgride::cli::CommandLine command_line("cgride", {"--help"});

    auto result = cgride::cli::CliOptions::parse(command_line);

    assert(result);
    assert(result.value().command() == cgride::cli::CliCommand::Help);
  }

  {
    cgride::cli::CommandLine command_line("cgride", {"-h"});

    auto result = cgride::cli::CliOptions::parse(command_line);

    assert(result);
    assert(result.value().command() == cgride::cli::CliCommand::Help);
  }

  {
    cgride::cli::CommandLine command_line("cgride", {"--version"});

    auto result = cgride::cli::CliOptions::parse(command_line);

    assert(result);
    assert(result.value().command() == cgride::cli::CliCommand::Version);
  }

  {
    cgride::cli::CommandLine command_line("cgride", {"-V"});

    auto result = cgride::cli::CliOptions::parse(command_line);

    assert(result);
    assert(result.value().command() == cgride::cli::CliCommand::Version);
  }

  {
    cgride::cli::CommandLine command_line("cgride", {"unknown"});

    auto result = cgride::cli::CliOptions::parse(command_line);

    assert(!result);
    assert(result.error().code() == cgride::core::ErrorCode::InvalidArgument);
    assert(result.error().message() == "Unknown command.");
    assert(result.error().detail().has_value());
    assert(result.error().detail().value() == "unknown");
  }

  {
    cgride::cli::CommandLine command_line("cgride", {"build", "extra"});

    auto result = cgride::cli::CliOptions::parse(command_line);

    assert(!result);
    assert(result.error().code() == cgride::core::ErrorCode::InvalidArgument);
    assert(result.error().message() == "Unexpected positional argument.");
    assert(result.error().detail().value() == "extra");
  }

  {
    cgride::cli::CommandLine command_line("cgride", {"build", "--root"});

    auto result = cgride::cli::CliOptions::parse(command_line);

    assert(!result);
    assert(result.error().message() == "Option --root requires a value.");
  }

  {
    cgride::cli::CommandLine command_line("cgride", {"build", "--config"});

    auto result = cgride::cli::CliOptions::parse(command_line);

    assert(!result);
    assert(result.error().message() == "Option --config requires a value.");
  }

  {
    cgride::cli::CommandLine command_line("cgride", {"build", "--target"});

    auto result = cgride::cli::CliOptions::parse(command_line);

    assert(!result);
    assert(result.error().message() == "Option --target requires a value.");
  }

  {
    cgride::cli::CommandLine command_line("cgride", {"build", "--jobs"});

    auto result = cgride::cli::CliOptions::parse(command_line);

    assert(!result);
    assert(result.error().message() == "Option --jobs requires a value.");
  }

  {
    cgride::cli::CommandLine command_line("cgride", {"build", "--jobs", "abc"});

    auto result = cgride::cli::CliOptions::parse(command_line);

    assert(!result);
    assert(result.error().message() == "Option --jobs expects a positive integer.");
    assert(result.error().detail().value() == "abc");
  }

  {
    cgride::cli::CommandLine command_line("cgride", {"build", "--jobs", "0"});

    auto result = cgride::cli::CliOptions::parse(command_line);

    assert(!result);
    assert(result.error().message() == "Option --jobs must be greater than zero.");
    assert(result.error().detail().value() == "0");
  }

  {
    cgride::cli::CommandLine command_line("cgride", {"build", "--jobs", "16x"});

    auto result = cgride::cli::CliOptions::parse(command_line);

    assert(!result);
    assert(result.error().message() == "Option --jobs expects a positive integer.");
    assert(result.error().detail().value() == "16x");
  }

  return 0;
}

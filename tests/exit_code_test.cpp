/**
 *
 *  @file exit_code_test.cpp
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
#include <string_view>

#include <cgride/cli/exit_code.hpp>

int main()
{
  {
    assert(cgride::cli::to_int(cgride::cli::ExitCode::Success) == 0);
    assert(cgride::cli::to_int(cgride::cli::ExitCode::Failure) == 1);
    assert(cgride::cli::to_int(cgride::cli::ExitCode::UsageError) == 2);
    assert(cgride::cli::to_int(cgride::cli::ExitCode::ConfigError) == 3);
    assert(cgride::cli::to_int(cgride::cli::ExitCode::BuildError) == 4);
    assert(cgride::cli::to_int(cgride::cli::ExitCode::InternalError) == 5);
  }

  {
    assert(cgride::cli::to_string(cgride::cli::ExitCode::Success) == std::string_view("success"));
    assert(cgride::cli::to_string(cgride::cli::ExitCode::Failure) == std::string_view("failure"));
    assert(cgride::cli::to_string(cgride::cli::ExitCode::UsageError) == std::string_view("usage_error"));
    assert(cgride::cli::to_string(cgride::cli::ExitCode::ConfigError) == std::string_view("config_error"));
    assert(cgride::cli::to_string(cgride::cli::ExitCode::BuildError) == std::string_view("build_error"));
    assert(cgride::cli::to_string(cgride::cli::ExitCode::InternalError) == std::string_view("internal_error"));
  }

  {
    assert(cgride::cli::succeeded(cgride::cli::ExitCode::Success));
    assert(!cgride::cli::failed(cgride::cli::ExitCode::Success));

    assert(!cgride::cli::succeeded(cgride::cli::ExitCode::Failure));
    assert(cgride::cli::failed(cgride::cli::ExitCode::Failure));

    assert(!cgride::cli::succeeded(cgride::cli::ExitCode::UsageError));
    assert(cgride::cli::failed(cgride::cli::ExitCode::UsageError));

    assert(!cgride::cli::succeeded(cgride::cli::ExitCode::ConfigError));
    assert(cgride::cli::failed(cgride::cli::ExitCode::ConfigError));

    assert(!cgride::cli::succeeded(cgride::cli::ExitCode::BuildError));
    assert(cgride::cli::failed(cgride::cli::ExitCode::BuildError));

    assert(!cgride::cli::succeeded(cgride::cli::ExitCode::InternalError));
    assert(cgride::cli::failed(cgride::cli::ExitCode::InternalError));
  }

  return 0;
}

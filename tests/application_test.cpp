/**
 *
 *  @file application_test.cpp
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
#include <sstream>
#include <string>

#include <cgride/cli/application.hpp>
#include <cgride/cli/command_line.hpp>
#include <cgride/cli/exit_code.hpp>
#include <cgride/cli/terminal.hpp>
#include <cgride/cli/version.hpp>

namespace
{
  [[nodiscard]] bool contains(
      const std::string &text,
      const std::string &needle)
  {
    return text.find(needle) != std::string::npos;
  }

} // namespace

int main()
{
  {
    std::ostringstream output;
    std::ostringstream error;

    cgride::cli::Terminal terminal(output, error);
    cgride::cli::Application application(terminal);

    auto result = application.run(cgride::cli::CommandLine("cgride", {}));

    assert(result == cgride::cli::ExitCode::Success);
    assert(contains(output.str(), "Cgride"));
    assert(contains(output.str(), "Usage:"));
    assert(contains(output.str(), "Commands:"));
    assert(error.str().empty());
  }

  {
    std::ostringstream output;
    std::ostringstream error;

    cgride::cli::Terminal terminal(output, error);
    cgride::cli::Application application(terminal);

    auto result = application.run(cgride::cli::CommandLine("cgride", {"help"}));

    assert(result == cgride::cli::ExitCode::Success);
    assert(contains(output.str(), "Usage:"));
    assert(contains(output.str(), "build"));
    assert(contains(output.str(), "run"));
    assert(error.str().empty());
  }

  {
    std::ostringstream output;
    std::ostringstream error;

    cgride::cli::Terminal terminal(output, error);
    cgride::cli::Application application(terminal);

    auto result = application.run(cgride::cli::CommandLine("cgride", {"--help"}));

    assert(result == cgride::cli::ExitCode::Success);
    assert(contains(output.str(), "Usage:"));
    assert(error.str().empty());
  }

  {
    std::ostringstream output;
    std::ostringstream error;

    cgride::cli::Terminal terminal(output, error);
    cgride::cli::Application application(terminal);

    auto result = application.run(cgride::cli::CommandLine("cgride", {"version"}));

    assert(result == cgride::cli::ExitCode::Success);
    assert(output.str() == "cgride " + std::string(cgride::cli::version_string) + "\n");
    assert(error.str().empty());
  }

  {
    std::ostringstream output;
    std::ostringstream error;

    cgride::cli::Terminal terminal(output, error);
    cgride::cli::Application application(terminal);

    auto result = application.run(cgride::cli::CommandLine("cgride", {"--version"}));

    assert(result == cgride::cli::ExitCode::Success);
    assert(output.str() == "cgride " + std::string(cgride::cli::version_string) + "\n");
    assert(error.str().empty());
  }

  {
    std::ostringstream output;
    std::ostringstream error;

    cgride::cli::Terminal terminal(output, error);
    cgride::cli::Application application(terminal);

    auto result = application.run(cgride::cli::CommandLine("cgride", {"unknown"}));

    assert(result == cgride::cli::ExitCode::UsageError);
    assert(contains(error.str(), "error: Unknown command."));
    assert(contains(error.str(), "detail: unknown"));
    assert(contains(output.str(), "Usage:"));
  }

  {
    std::ostringstream output;
    std::ostringstream error;

    cgride::cli::Terminal terminal(output, error);
    cgride::cli::Application application(terminal);

    auto result = application.run(
        cgride::cli::CommandLine("cgride", {"build", "extra"}));

    assert(result == cgride::cli::ExitCode::UsageError);
    assert(contains(error.str(), "error: Unexpected positional argument."));
    assert(contains(error.str(), "detail: extra"));
    assert(contains(output.str(), "Usage:"));
  }

  {
    std::ostringstream output;
    std::ostringstream error;

    cgride::cli::Terminal terminal(output, error);
    cgride::cli::Application application(terminal);

    auto result = application.run(
        cgride::cli::CommandLine("cgride", {"build", "--jobs", "0"}));

    assert(result == cgride::cli::ExitCode::UsageError);
    assert(contains(error.str(), "error: Option --jobs must be greater than zero."));
    assert(contains(error.str(), "detail: 0"));
    assert(contains(output.str(), "Usage:"));
  }

  {
    std::ostringstream output;
    std::ostringstream error;

    cgride::cli::Terminal terminal(output, error);
    cgride::cli::Application application(terminal);

    auto result = application.run(
        cgride::cli::CommandLine("cgride", {"build", "--config"}));

    assert(result == cgride::cli::ExitCode::UsageError);
    assert(contains(error.str(), "error: Option --config requires a value."));
    assert(contains(output.str(), "Usage:"));
  }

  {
    std::ostringstream output;
    std::ostringstream error;

    cgride::cli::Terminal terminal(output, error);
    cgride::cli::Application application(terminal);

    char program[] = "cgride";
    char version[] = "--version";

    char *argv[] = {
        program,
        version,
    };

    auto result = application.run(2, argv);

    assert(result == cgride::cli::ExitCode::Success);
    assert(output.str() == "cgride " + std::string(cgride::cli::version_string) + "\n");
    assert(error.str().empty());
  }

  {
    std::ostringstream output;
    std::ostringstream error;

    cgride::cli::Terminal terminal(output, error);
    cgride::cli::Application application(terminal);

    assert(&application.terminal().output_stream() == &output);
    assert(&application.terminal().error_stream() == &error);
  }

  return 0;
}

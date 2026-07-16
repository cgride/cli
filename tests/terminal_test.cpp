/**
 *
 *  @file terminal_test.cpp
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

#include <cgride/cli/terminal.hpp>

int main()
{
  {
    cgride::cli::Terminal terminal;

    assert(!terminal.quiet());
    assert(!terminal.verbose());
  }

  {
    std::ostringstream output;
    std::ostringstream error;

    cgride::cli::Terminal terminal(output, error);

    terminal
        .write("hello")
        .writeln(" world")
        .writeln();

    assert(output.str() == "hello world\n\n");
    assert(error.str().empty());
  }

  {
    std::ostringstream output;
    std::ostringstream error;

    cgride::cli::Terminal terminal(output, error);

    terminal
        .write_error("bad")
        .writeln_error(" error")
        .writeln_error();

    assert(output.str().empty());
    assert(error.str() == "bad error\n\n");
  }

  {
    std::ostringstream output;
    std::ostringstream error;

    cgride::cli::Terminal terminal(output, error);

    terminal
        .print_info("loading")
        .print_success("done")
        .print_warning("careful")
        .print_error("failed");

    assert(output.str() == "info: loading\nsuccess: done\nwarning: careful\n");
    assert(error.str() == "error: failed\n");
  }

  {
    std::ostringstream output;
    std::ostringstream error;

    cgride::cli::Terminal terminal(output, error);

    terminal.print_verbose("hidden");

    assert(output.str().empty());

    terminal.verbose(true);
    terminal.print_verbose("visible");

    assert(output.str() == "verbose: visible\n");
    assert(terminal.verbose());
  }

  {
    std::ostringstream output;
    std::ostringstream error;

    cgride::cli::Terminal terminal(output, error);

    terminal
        .quiet(true)
        .write("hidden")
        .writeln("hidden")
        .print_info("hidden")
        .print_success("hidden")
        .print_warning("hidden")
        .print_verbose("hidden")
        .print_error("visible");

    assert(terminal.quiet());
    assert(output.str().empty());
    assert(error.str() == "error: visible\n");
  }

  {
    std::ostringstream output_a;
    std::ostringstream error_a;
    std::ostringstream output_b;
    std::ostringstream error_b;

    cgride::cli::Terminal terminal(output_a, error_a);

    terminal.writeln("a");
    terminal.writeln_error("ea");

    terminal
        .output_stream(output_b)
        .error_stream(error_b);

    terminal.writeln("b");
    terminal.writeln_error("eb");

    assert(output_a.str() == "a\n");
    assert(error_a.str() == "ea\n");
    assert(output_b.str() == "b\n");
    assert(error_b.str() == "eb\n");
  }

  {
    std::ostringstream output;
    std::ostringstream error;

    cgride::cli::Terminal terminal(output, error);

    assert(&terminal.output_stream() == &output);
    assert(&terminal.error_stream() == &error);
  }

  return 0;
}

/**
 *
 *  @file main.cpp
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
#include <cgride/cli/exit_code.hpp>

int main(
    int argc,
    char **argv)
{
  cgride::cli::Application application;

  const auto exit_code = application.run(argc, argv);

  return cgride::cli::to_int(exit_code);
}

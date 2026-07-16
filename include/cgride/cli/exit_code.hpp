/**
 *
 *  @file exit_code.hpp
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
#ifndef CGRIDE_CLI_EXIT_CODE_HPP
#define CGRIDE_CLI_EXIT_CODE_HPP

#include <string_view>

namespace cgride::cli
{
  /**
   * @enum ExitCode
   * @brief Stable process exit codes returned by the CLI.
   */
  enum class ExitCode
  {
    Success = 0,
    Failure = 1,
    UsageError = 2,
    ConfigError = 3,
    BuildError = 4,
    InternalError = 5
  };

  /**
   * @brief Convert an exit code to an integer process status.
   *
   * @param code Exit code.
   * @return Integer process status.
   */
  [[nodiscard]] int to_int(ExitCode code) noexcept;

  /**
   * @brief Convert an exit code to a stable string.
   *
   * @param code Exit code.
   * @return Stable string representation.
   */
  [[nodiscard]] std::string_view to_string(ExitCode code) noexcept;

  /**
   * @brief Return true when the exit code represents success.
   *
   * @param code Exit code.
   * @return True when the code is Success.
   */
  [[nodiscard]] bool succeeded(ExitCode code) noexcept;

  /**
   * @brief Return true when the exit code represents failure.
   *
   * @param code Exit code.
   * @return True when the code is not Success.
   */
  [[nodiscard]] bool failed(ExitCode code) noexcept;

} // namespace cgride::cli

#endif // CGRIDE_CLI_EXIT_CODE_HPP

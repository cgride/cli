/**
 *
 *  @file exit_code.cpp
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
#include <cgride/cli/exit_code.hpp>

namespace cgride::cli
{
  int to_int(ExitCode code) noexcept
  {
    return static_cast<int>(code);
  }

  std::string_view to_string(ExitCode code) noexcept
  {
    switch (code)
    {
    case ExitCode::Success:
      return "success";

    case ExitCode::Failure:
      return "failure";

    case ExitCode::UsageError:
      return "usage_error";

    case ExitCode::ConfigError:
      return "config_error";

    case ExitCode::BuildError:
      return "build_error";

    case ExitCode::InternalError:
      return "internal_error";
    }

    return "failure";
  }

  bool succeeded(ExitCode code) noexcept
  {
    return code == ExitCode::Success;
  }

  bool failed(ExitCode code) noexcept
  {
    return !succeeded(code);
  }

} // namespace cgride::cli

/**
 *
 *  @file project_loader.hpp
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
#ifndef CGRIDE_CLI_PROJECT_LOADER_HPP
#define CGRIDE_CLI_PROJECT_LOADER_HPP

#include <filesystem>
#include <optional>

#include <cgride/cli/command_context.hpp>
#include <cgride/core/result.hpp>
#include <cgride/engine/build_options.hpp>
#include <cgride/project/project.hpp>
#include <cgride/project/target.hpp>
#include <cgride/toolchains/toolchain.hpp>

namespace cgride::cli
{
  struct LoadedProject
  {
    cgride::project::Project project;
    std::filesystem::path project_root;
    std::filesystem::path project_file;
    std::optional<cgride::toolchains::Toolchain> toolchain{};
    bool legacy_config{false};
  };

  [[nodiscard]] cgride::core::Result<LoadedProject> load_project(
      CommandContext &context);

  [[nodiscard]] cgride::core::Result<cgride::toolchains::Toolchain> discover_cli_toolchain(
      CommandContext &context);

  [[nodiscard]] cgride::engine::BuildOptions make_build_options(
      const CommandContext &context,
      const std::filesystem::path &project_root);

  [[nodiscard]] const cgride::project::Target *select_executable_target(
      const cgride::project::Project &project,
      const std::optional<std::string> &requested_target);

  [[nodiscard]] std::filesystem::path executable_path_for(
      const std::filesystem::path &project_root,
      const cgride::engine::BuildOptions &options,
      const cgride::project::Target &target);

} // namespace cgride::cli

#endif // CGRIDE_CLI_PROJECT_LOADER_HPP

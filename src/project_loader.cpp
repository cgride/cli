/**
 *
 *  @file project_loader.cpp
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
#include "project_loader.hpp"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include <cgride/config/config_loader.hpp>
#include <cgride/config/project_reader.hpp>
#include <cgride/core/command.hpp>
#include <cgride/core/error.hpp>
#include <cgride/core/platform.hpp>
#include <cgride/executor/execution_options.hpp>
#include <cgride/executor/process.hpp>
#include <cgride/project/target_kind.hpp>
#include <cgride/toolchains/compiler_kind.hpp>
#include <cgride/toolchains/discovery.hpp>

namespace cgride::cli
{
  namespace
  {
    using cgride::core::Error;
    using cgride::core::ErrorCode;

    constexpr std::string_view cpp_project_file = "cgride.cpp";
    constexpr std::string_view legacy_config_file = "cgride.config";

    [[nodiscard]] std::filesystem::path absolute_path(
        const std::filesystem::path &base,
        const std::filesystem::path &path)
    {
      if (path.is_absolute())
      {
        return path;
      }

      return base / path;
    }

    [[nodiscard]] bool exists_regular_file(const std::filesystem::path &path)
    {
      std::error_code error;
      return std::filesystem::exists(path, error) &&
             std::filesystem::is_regular_file(path, error);
    }

    [[nodiscard]] bool contains_glob_pattern(std::string_view value) noexcept
    {
      return value.find_first_of("*?[") != std::string_view::npos;
    }

    [[nodiscard]] std::filesystem::path rooted_path(
        const std::filesystem::path &project_root,
        const std::filesystem::path &path)
    {
      if (path.is_absolute())
      {
        return path;
      }

      return project_root / path;
    }

    [[nodiscard]] bool is_regex_special(char character) noexcept
    {
      switch (character)
      {
      case '.':
      case '+':
      case '(':
      case ')':
      case '^':
      case '$':
      case '|':
      case '{':
      case '}':
      case '[':
      case ']':
      case '\\':
        return true;

      default:
        return false;
      }
    }

    [[nodiscard]] std::string glob_to_regex(std::string_view pattern)
    {
      std::string output = "^";

      for (std::size_t index = 0; index < pattern.size(); ++index)
      {
        const auto character = pattern[index];

        if (character == '\\')
        {
          output.push_back('/');
          continue;
        }

        if (character == '*')
        {
          if (index + 2 < pattern.size() && pattern[index + 1] == '*' &&
              (pattern[index + 2] == '/' || pattern[index + 2] == '\\'))
          {
            output += "(?:.*/)?";
            index += 2;
            continue;
          }

          if (index + 1 < pattern.size() && pattern[index + 1] == '*')
          {
            output += ".*";
            ++index;
            continue;
          }

          output += "[^/]*";
          continue;
        }

        if (character == '?')
        {
          output += "[^/]";
          continue;
        }

        if (character == '/')
        {
          output.push_back('/');
          continue;
        }

        if (is_regex_special(character))
        {
          output.push_back('\\');
        }

        output.push_back(character);
      }

      output.push_back('$');
      return output;
    }

    [[nodiscard]] std::vector<std::filesystem::path> expand_sources(
        const std::filesystem::path &project_root,
        std::string_view pattern)
    {
      if (!contains_glob_pattern(pattern))
      {
        return {rooted_path(project_root, std::filesystem::path(pattern))};
      }

      std::vector<std::filesystem::path> sources;
      const std::regex matcher(glob_to_regex(pattern));
      std::error_code error;

      for (std::filesystem::recursive_directory_iterator iterator(project_root, error), end;
           iterator != end && !error;
           iterator.increment(error))
      {
        if (!iterator->is_regular_file(error))
        {
          continue;
        }

        auto relative = std::filesystem::relative(iterator->path(), project_root, error);

        if (error)
        {
          error.clear();
          continue;
        }

        if (std::regex_match(relative.generic_string(), matcher))
        {
          sources.push_back(iterator->path());
        }
      }

      std::sort(sources.begin(), sources.end());
      return sources;
    }

    [[nodiscard]] cgride::project::Project normalize_project_paths(
        cgride::project::Project project,
        const std::filesystem::path &project_root)
    {
      cgride::project::Project normalized(project.name());

      for (const auto &source_target : project.targets())
      {
        const auto &source = *source_target;
        auto &target = normalized.target(source.name(), source.kind());

        target
            .cpp_standard(source.cpp_standard())
            .build_profile(source.build_profile());

        for (const auto &entry : source.source_set().entries())
        {
          if (!entry.valid())
          {
            continue;
          }

          if (entry.kind() == cgride::project::SourceKind::File)
          {
            target.source(rooted_path(project_root, entry.path()));
            continue;
          }

          for (auto &expanded : expand_sources(project_root, entry.pattern()))
          {
            target.source(std::move(expanded));
          }
        }

        for (const auto &requirement : source.requirements().entries())
        {
          if (!requirement.valid())
          {
            continue;
          }

          switch (requirement.kind())
          {
          case cgride::project::RequirementKind::IncludeDirectory:
            target.include_directory(rooted_path(project_root, requirement.path()), requirement.visibility());
            break;

          case cgride::project::RequirementKind::CompileDefinition:
            target.compile_definition(requirement.value(), requirement.visibility());
            break;

          case cgride::project::RequirementKind::CompileOption:
            target.compile_option(requirement.value(), requirement.visibility());
            break;

          case cgride::project::RequirementKind::LinkOption:
            target.link_option(requirement.value(), requirement.visibility());
            break;

          case cgride::project::RequirementKind::LinkLibrary:
            target.link_library(requirement.value(), requirement.visibility());
            break;
          }
        }

        for (const auto &link : source.target_links())
        {
          if (link.valid())
          {
            target.link_named(link.target_name(), link.visibility());
          }
        }
      }

      return normalized;
    }

    [[nodiscard]] std::optional<std::filesystem::path> find_upwards(
        std::filesystem::path start,
        std::string_view filename)
    {
      std::error_code error;
      start = std::filesystem::weakly_canonical(start, error);

      if (error || start.empty())
      {
        start = ".";
      }

      if (exists_regular_file(start))
      {
        start = start.parent_path();
      }

      while (!start.empty())
      {
        const auto candidate = start / filename;

        if (exists_regular_file(candidate))
        {
          return candidate;
        }

        const auto parent = start.parent_path();

        if (parent == start || parent.empty())
        {
          break;
        }

        start = parent;
      }

      return std::nullopt;
    }

    [[nodiscard]] std::filesystem::path search_start(const CommandContext &context)
    {
      return absolute_path(
          context.working_directory(),
          context.options().project_root());
    }

    [[nodiscard]] std::string escape_config_value(std::string_view value)
    {
      std::string output;
      output.reserve(value.size() + 2);
      output.push_back('"');

      for (const auto character : value)
      {
        switch (character)
        {
        case '\\':
          output += "\\\\";
          break;

        case '"':
          output += "\\\"";
          break;

        case '\n':
          output += "\\n";
          break;

        case '\r':
          output += "\\r";
          break;

        case '\t':
          output += "\\t";
          break;

        default:
          output.push_back(character);
          break;
        }
      }

      output.push_back('"');
      return output;
    }

    [[nodiscard]] std::string config_helper_source()
    {
      return R"cpp(
#include <cgride/project.hpp>

#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

void cgride_configure(cgride::project::Project &project);

namespace
{
  [[nodiscard]] std::string escape_value(std::string_view value)
  {
    std::string output;
    output.reserve(value.size() + 2);
    output.push_back('"');

    for (const auto character : value)
    {
      switch (character)
      {
      case '\\':
        output += "\\\\";
        break;

      case '"':
        output += "\\\"";
        break;

      case '\n':
        output += "\\n";
        break;

      case '\r':
        output += "\\r";
        break;

      case '\t':
        output += "\\t";
        break;

      default:
        output.push_back(character);
        break;
      }
    }

    output.push_back('"');
    return output;
  }

  void emit_list(const char *key, const std::vector<std::string> &values)
  {
    if (values.empty())
    {
      return;
    }

    std::cout << key << " = ";

    for (std::size_t index = 0; index < values.size(); ++index)
    {
      if (index != 0)
      {
        std::cout << ", ";
      }

      std::cout << values[index];
    }

    std::cout << '\n';
  }

  [[nodiscard]] const char *kind_name(cgride::project::TargetKind kind)
  {
    switch (kind)
    {
    case cgride::project::TargetKind::StaticLibrary:
      return "static_library";

    case cgride::project::TargetKind::InterfaceLibrary:
      return "interface_library";

    case cgride::project::TargetKind::Executable:
    case cgride::project::TargetKind::SharedLibrary:
      return "executable";
    }

    return "executable";
  }

  void emit_target(const cgride::project::Target &target)
  {
    std::vector<std::string> sources;
    std::vector<std::string> include_dirs;
    std::vector<std::string> public_include_dirs;
    std::vector<std::string> definitions;
    std::vector<std::string> compile_options;
    std::vector<std::string> link_options;
    std::vector<std::string> libraries;
    std::vector<std::string> links;

    for (const auto &entry : target.source_set().entries())
    {
      if (!entry.valid())
      {
        continue;
      }

      if (entry.kind() == cgride::project::SourceKind::File)
      {
        sources.push_back(entry.path().generic_string());
      }
      else
      {
        sources.push_back(entry.pattern());
      }
    }

    for (const auto &requirement : target.requirements().entries())
    {
      if (!requirement.valid())
      {
        continue;
      }

      switch (requirement.kind())
      {
      case cgride::project::RequirementKind::IncludeDirectory:
        if (requirement.visibility() == cgride::project::Visibility::Public)
        {
          public_include_dirs.push_back(requirement.path().generic_string());
        }
        else
        {
          include_dirs.push_back(requirement.path().generic_string());
        }
        break;

      case cgride::project::RequirementKind::CompileDefinition:
        definitions.push_back(requirement.value());
        break;

      case cgride::project::RequirementKind::CompileOption:
        compile_options.push_back(requirement.value());
        break;

      case cgride::project::RequirementKind::LinkOption:
        link_options.push_back(requirement.value());
        break;

      case cgride::project::RequirementKind::LinkLibrary:
        libraries.push_back(requirement.value());
        break;
      }
    }

    for (const auto &link : target.target_links())
    {
      if (link.valid())
      {
        links.push_back(link.target_name());
      }
    }

    std::cout << "\n[target." << target.name() << "]\n";
    std::cout << "kind = " << kind_name(target.kind()) << '\n';
    emit_list("sources", sources);
    emit_list("include_dirs", include_dirs);
    emit_list("public_include_dirs", public_include_dirs);
    emit_list("definitions", definitions);
    emit_list("compile_options", compile_options);
    emit_list("link_options", link_options);
    emit_list("libraries", libraries);
    emit_list("links", links);
  }
}

int main()
{
  cgride::project::Project project;
  cgride_configure(project);

  std::cout << "[project]\n";
  std::cout << "name = " << escape_value(project.name().empty() ? "cgride_project" : project.name()) << '\n';

  for (const auto &target : project.targets())
  {
    emit_target(*target);
  }

  return 0;
}
)cpp";
    }

    [[nodiscard]] std::string generated_cmake_source(
        const std::filesystem::path &project_file)
    {
      std::ostringstream output;
      output << "cmake_minimum_required(VERSION 3.22)\n";
      output << "project(cgride_user_config LANGUAGES CXX)\n";
      output << "find_package(cgride CONFIG REQUIRED)\n";
      output << "add_executable(cgride_config cgride_config_main.cpp ";
      output << escape_config_value(project_file.generic_string()) << ")\n";
      output << "target_link_libraries(cgride_config PRIVATE cgride::cgride)\n";
      output << "target_compile_features(cgride_config PRIVATE cxx_std_23)\n";
      output << "set_target_properties(cgride_config PROPERTIES CXX_EXTENSIONS OFF)\n";
      output << "set_target_properties(cgride_config PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)\n";
      output << "foreach(config DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)\n";
      output << "  set_target_properties(cgride_config PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${config} ${CMAKE_BINARY_DIR}/bin)\n";
      output << "endforeach()\n";
      return output.str();
    }

    [[nodiscard]] cgride::core::Result<void> write_text_file(
        const std::filesystem::path &path,
        const std::string &content)
    {
      std::error_code error;
      std::filesystem::create_directories(path.parent_path(), error);

      if (error)
      {
        return Error(
            ErrorCode::IoError,
            "Failed to create generated configuration directory.",
            error.message(),
            path.parent_path());
      }

      std::ofstream stream(path, std::ios::binary);

      if (!stream)
      {
        return Error(
            ErrorCode::IoError,
            "Failed to write generated configuration file.",
            path);
      }

      stream << content;

      if (!stream)
      {
        return Error(
            ErrorCode::IoError,
            "Failed to finish writing generated configuration file.",
            path);
      }

      return cgride::core::Result<void>::ok();
    }

    [[nodiscard]] std::filesystem::path config_executable_path(
        const std::filesystem::path &config_build_dir)
    {
      return config_build_dir /
             "bin" /
             (std::string("cgride_config") +
              std::string(cgride::core::executable_extension(cgride::core::host_platform())));
    }

    [[nodiscard]] cgride::core::Result<void> run_internal_process(
        cgride::core::Command command,
        std::string_view failure_message,
        CommandContext &context)
    {
      cgride::executor::ExecutionOptions execution_options;
      execution_options.capture_output(true);

      if (context.options().verbose())
      {
        context.terminal().print_verbose(cgride::executor::describe_command(command));
      }

      auto result = cgride::executor::run_process(command, execution_options);

      if (!result)
      {
        return result.error();
      }

      const auto &process = result.value();

      if (process.exit_code().value_or(1) != 0)
      {
        return Error(
            ErrorCode::ProcessFailed,
            std::string(failure_message),
            process.standard_error().empty() ? process.standard_output() : process.standard_error());
      }

      return cgride::core::Result<void>::ok();
    }

    [[nodiscard]] cgride::core::Result<std::string> run_config_executable(
        const std::filesystem::path &executable,
        const std::filesystem::path &project_root)
    {
      cgride::core::Command command(executable.string());
      command.cwd(project_root);
      command.search_in_path(false);

      cgride::executor::ExecutionOptions options;
      options.capture_output(true);

      auto result = cgride::executor::run_process(command, options);

      if (!result)
      {
        return result.error();
      }

      const auto &process = result.value();

      if (process.exit_code().value_or(1) != 0)
      {
        return Error(
            ErrorCode::ProcessFailed,
            "Generated cgride.cpp configuration program failed.",
            process.standard_error().empty() ? process.standard_output() : process.standard_error(),
            executable);
      }

      return process.standard_output();
    }

    [[nodiscard]] cgride::core::Result<cgride::project::Project> load_cpp_project(
        const std::filesystem::path &project_file,
        const std::filesystem::path &project_root,
        const cgride::toolchains::Toolchain &toolchain,
        CommandContext &context)
    {
      const auto config_dir = project_root / ".cgride" / "config";
      const auto config_build_dir = config_dir / "build";
      const auto helper_source = config_dir / "cgride_config_main.cpp";
      const auto cmake_source = config_dir / "CMakeLists.txt";

      auto wrote_helper = write_text_file(helper_source, config_helper_source());

      if (!wrote_helper)
      {
        return wrote_helper.error();
      }

      auto wrote_cmake = write_text_file(cmake_source, generated_cmake_source(project_file));

      if (!wrote_cmake)
      {
        return wrote_cmake.error();
      }

      cgride::core::Command configure("cmake");
      configure
          .arg("-S")
          .arg(config_dir.string())
          .arg("-B")
          .arg(config_build_dir.string());

      if (toolchain.cxx_compiler().has_value())
      {
        configure.arg("-DCMAKE_CXX_COMPILER=" + toolchain.cxx_compiler().value().string());
      }

      const auto *prefix_path = std::getenv("CGRIDE_PREFIX_PATH");

      if (prefix_path != nullptr && std::string_view(prefix_path).size() != 0)
      {
        configure.arg(std::string("-DCMAKE_PREFIX_PATH=") + prefix_path);
      }

      auto configured = run_internal_process(
          std::move(configure),
          "Failed to configure cgride.cpp project loader.",
          context);

      if (!configured)
      {
        return configured.error();
      }

      cgride::core::Command build("cmake");
      build
          .arg("--build")
          .arg(config_build_dir.string())
          .arg("--parallel");

      auto built = run_internal_process(
          std::move(build),
          "Failed to build cgride.cpp project loader.",
          context);

      if (!built)
      {
        return built.error();
      }

      auto emitted = run_config_executable(
          config_executable_path(config_build_dir),
          project_root);

      if (!emitted)
      {
        return emitted.error();
      }

      cgride::config::ProjectReader reader;
      return reader.read_string(emitted.value());
    }

    [[nodiscard]] cgride::core::Result<LoadedProject> load_legacy_config(
        const std::filesystem::path &config_path)
    {
      cgride::config::ConfigOptions options;
      options
          .project_root(config_path.parent_path())
          .config_path(config_path.filename())
          .strict(true)
          .allow_missing(false);

      cgride::config::ConfigLoader loader(options);
      auto document = loader.load();

      if (!document)
      {
        return document.error();
      }

      cgride::config::ProjectReader reader;
      auto project = reader.read(document.value());

      if (!project)
      {
        return project.error();
      }

      LoadedProject loaded;
      loaded.project = normalize_project_paths(std::move(project.value()), config_path.parent_path());
      loaded.project_root = config_path.parent_path();
      loaded.project_file = config_path;
      loaded.legacy_config = true;

      return loaded;
    }

  } // namespace

  cgride::core::Result<LoadedProject> load_project(
      CommandContext &context)
  {
    const auto start = search_start(context);
    const auto cpp_file = find_upwards(start, cpp_project_file);

    if (cpp_file.has_value())
    {
      auto toolchain = discover_cli_toolchain(context);

      if (!toolchain)
      {
        return toolchain.error();
      }

      auto project = load_cpp_project(
          cpp_file.value(),
          cpp_file->parent_path(),
          toolchain.value(),
          context);

      if (!project)
      {
        return project.error();
      }

      LoadedProject loaded;
      loaded.project = normalize_project_paths(std::move(project.value()), cpp_file->parent_path());
      loaded.project_root = cpp_file->parent_path();
      loaded.project_file = cpp_file.value();
      loaded.legacy_config = false;

      return loaded;
    }

    const auto legacy_file = find_upwards(start, legacy_config_file);

    if (legacy_file.has_value())
    {
      return load_legacy_config(legacy_file.value());
    }

    return Error(
        ErrorCode::NotFound,
        "Cgride expects a cgride.cpp project file.",
        "Create cgride.cpp in the project root. cgride.config is a legacy internal fallback only.",
        start / cpp_project_file);
  }

  cgride::core::Result<cgride::toolchains::Toolchain> discover_cli_toolchain(
      CommandContext &context)
  {
    context.terminal().print_verbose("Discovering C++ toolchain.");

    return cgride::toolchains::discover_toolchain(
        cgride::toolchains::CompilerKind::Unknown);
  }

  cgride::engine::BuildOptions make_build_options(
      const CommandContext &context,
      const std::filesystem::path &project_root)
  {
    const auto &cli_options = context.options();

    cgride::engine::BuildOptions build_options;

    build_options
        .build_directory(project_root / ".cgride" / "build")
        .mode(cli_options.release()
                  ? cgride::engine::BuildMode::Release
                  : cgride::engine::BuildMode::Debug)
        .jobs(cli_options.jobs())
        .rebuild(cli_options.rebuild())
        .use_cache(!cli_options.no_cache())
        .dry_run(cli_options.dry_run())
        .verbose(cli_options.verbose());

    if (cli_options.has_target())
    {
      build_options.target(cli_options.target().value());
    }

    return build_options;
  }

  const cgride::project::Target *select_executable_target(
      const cgride::project::Project &project,
      const std::optional<std::string> &requested_target)
  {
    if (requested_target.has_value())
    {
      const auto *target = project.find_target(requested_target.value());

      if (target != nullptr && target->kind() == cgride::project::TargetKind::Executable)
      {
        return target;
      }

      return nullptr;
    }

    for (const auto &target : project.targets())
    {
      if (target->kind() == cgride::project::TargetKind::Executable)
      {
        return target.get();
      }
    }

    return nullptr;
  }

  std::filesystem::path executable_path_for(
      const std::filesystem::path &project_root,
      const cgride::engine::BuildOptions &options,
      const cgride::project::Target &target)
  {
    (void)options;

    return project_root /
           ".cgride" /
           "build" /
           "bin" /
           (target.name() + std::string(cgride::core::executable_extension(cgride::core::host_platform())));
  }

} // namespace cgride::cli

cmake_minimum_required(VERSION 3.22)

project(
  cgride-cli
  VERSION 0.1.1
  DESCRIPTION "Command line interface for Cgride."
  HOMEPAGE_URL "https://github.com/cgride/cli"
  LANGUAGES CXX
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(CGRIDE_CLI_BUILD_TESTS "Build Cgride CLI tests" ON)
option(CGRIDE_CLI_INSTALL "Install Cgride CLI" ON)

if(NOT TARGET cgride::core)
  find_package(cgride-core CONFIG REQUIRED)
endif()

if(NOT TARGET cgride::config)
  find_package(cgride-config CONFIG REQUIRED)
endif()

if(NOT TARGET cgride::toolchains)
  find_package(cgride-toolchains CONFIG REQUIRED)
endif()

if(NOT TARGET cgride::engine)
  find_package(cgride-engine CONFIG REQUIRED)
endif()

if(NOT TARGET cgride::executor)
  find_package(cgride-executor CONFIG REQUIRED)
endif()

add_library(cgride_cli
  src/exit_code.cpp
  src/command_line.cpp
  src/cli_options.cpp
  src/terminal.cpp
  src/command_context.cpp
  src/project_loader.cpp
  src/build_command.cpp
  src/run_command.cpp
  src/application.cpp
)

add_library(cgride::cli ALIAS cgride_cli)

target_compile_features(cgride_cli
  PUBLIC
    cxx_std_23
)

target_include_directories(cgride_cli
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(cgride_cli
  PUBLIC
    cgride::core
    cgride::config
    cgride::toolchains
    cgride::engine
    cgride::executor
)

set_target_properties(cgride_cli
  PROPERTIES
    OUTPUT_NAME cgride_cli
    EXPORT_NAME cli
    CXX_EXTENSIONS OFF
)

add_executable(cgride_cli_bin
  src/main.cpp
)

target_compile_features(cgride_cli_bin
  PRIVATE
    cxx_std_23
)

target_link_libraries(cgride_cli_bin
  PRIVATE
    cgride::cli
)

set_target_properties(cgride_cli_bin
  PROPERTIES
    OUTPUT_NAME cgride
    CXX_EXTENSIONS OFF
)

if(CGRIDE_CLI_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

if(CGRIDE_CLI_INSTALL)
  install(
    TARGETS cgride_cli
    EXPORT cgride-cli-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )

  install(
    TARGETS cgride_cli_bin
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )

  install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )

  install(
    EXPORT cgride-cli-targets
    NAMESPACE cgride::
    FILE cgride-cli-targets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cgride-cli
  )

  configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cgride-cli-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cgride-cli-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cgride-cli
  )

  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/cgride-cli-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
  )

  install(
    FILES
      ${CMAKE_CURRENT_BINARY_DIR}/cgride-cli-config.cmake
      ${CMAKE_CURRENT_BINARY_DIR}/cgride-cli-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cgride-cli
  )
endif()

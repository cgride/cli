# Cgride CLI

`cgride-cli` provides the command line interface for Cgride.

It is the user-facing layer that parses command-line arguments, prints terminal output, loads the project configuration, discovers the C++ toolchain, creates a build request, and delegates build execution to `cgride-engine`.

The CLI does not own the build graph, command execution, cache storage, or build planning logic.

## Module boundary

This module depends on:

- `cgride::core`
- `cgride::config`
- `cgride::toolchains`
- `cgride::engine`

It must not depend directly on:

- `cgride::graph`
- `cgride::executor`
- `cgride::cache`

Those modules are used behind the engine boundary.

## Main responsibilities

The CLI is responsible for:

- parsing `argc` and `argv`
- selecting the requested command
- validating user-facing options
- printing help and version information
- loading `cgride.config`
- reading the project model
- discovering the local C++ toolchain
- creating engine build options
- calling the build engine
- returning stable process exit codes

## Commands

### Help

```bash
cgride help
```

Or:

```bash
cgride --help
```

### Version

```bash
cgride version
```

Or:

```bash
cgride --version
```

### Build

```bash
cgride build
```

Build with a specific config file:

```bash
cgride build --config project.cgride
```

Build from a specific project root:

```bash
cgride build --root examples/app
```

Build a specific target:

```bash
cgride build --target app
```

Build in release mode:

```bash
cgride build --release
```

Force a rebuild:

```bash
cgride build --rebuild
```

Disable cache usage:

```bash
cgride build --no-cache
```

Prepare the build without executing it:

```bash
cgride build --dry-run
```

Enable verbose output:

```bash
cgride build --verbose
```

### Run

```bash
cgride run
```

The current `run` command builds the project first. Launching the produced executable will be added when executable artifact mapping is exposed by the engine.

## Options

```text
--root <path>      Project root directory
--config <path>    Config file path
--target <name>    Build a specific target
--release          Build in release mode
--jobs <count>     Number of build jobs
--rebuild          Force a rebuild
--no-cache         Disable build cache usage
--dry-run          Prepare the build without executing it
--verbose          Print verbose output
--help, -h         Show help
--version, -V      Show version information
```

## Exit codes

The CLI returns stable process exit codes:

```text
0  success
1  failure
2  usage_error
3  config_error
4  build_error
5  internal_error
```

## Components

### CommandLine

`CommandLine` wraps raw `argc` and `argv`.

It provides helpers for:

- program name
- raw arguments
- positional arguments
- flags
- option values

Example:

```cpp
auto command_line = cgride::cli::CommandLine::from_argv(argc, argv);

if (command_line.has("--help"))
{
  // print help
}
```

### CliOptions

`CliOptions` stores parsed user-facing options.

It supports:

- command selection
- project root
- config path
- target name
- release mode
- jobs
- rebuild mode
- no-cache mode
- dry-run mode
- verbose mode

Example:

```cpp
auto options = cgride::cli::CliOptions::parse(command_line);

if (!options)
{
  return cgride::cli::ExitCode::UsageError;
}
```

### Terminal

`Terminal` centralizes terminal output.

It wraps output and error streams so commands can be tested without writing directly to `std::cout` or `std::cerr`.

Example:

```cpp
cgride::cli::Terminal terminal;

terminal.print_info("Building project.");
terminal.print_success("Build finished.");
terminal.print_error("Build failed.");
```

### CommandContext

`CommandContext` stores shared command state.

It contains:

- parsed CLI options
- terminal output wrapper
- working directory

It can also convert CLI options into `cgride::config::ConfigOptions`.

### BuildCommand

`BuildCommand` implements:

```bash
cgride build
```

It performs the CLI-side build flow:

1. Load the config file.
2. Read the project model.
3. Discover the C++ toolchain.
4. Create build options.
5. Create a build request.
6. Call `cgride::engine::BuildEngine`.

### RunCommand

`RunCommand` implements:

```bash
cgride run
```

It currently runs the build step first. Executable launching will be added later.

### Application

`Application` is the main CLI dispatcher.

It handles:

- help
- version
- usage errors
- build command dispatch
- run command dispatch

### main.cpp

`main.cpp` only creates the application, runs it, and returns the integer exit code.

## Build

From the module directory:

```bash
cmake -S . -B build
cmake --build build
```

Disable tests:

```bash
cmake -S . -B build -DCGRIDE_CLI_BUILD_TESTS=OFF
cmake --build build
```

## Run tests

```bash
ctest --test-dir build --output-on-failure
```

Or run tests directly:

```bash
./build/tests/cgride_cli_exit_code_test
./build/tests/cgride_cli_command_line_test
./build/tests/cgride_cli_cli_options_test
./build/tests/cgride_cli_terminal_test
./build/tests/cgride_cli_application_test
```

## Install

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --install build
```

The install step provides:

- the `cgride` executable
- the `cgride::cli` CMake target
- public headers
- CMake package files

## CMake package

After installation:

```cmake
find_package(cgride-cli CONFIG REQUIRED)

target_link_libraries(my_target
  PRIVATE
    cgride::cli
)
```

The package config loads these dependencies:

```cmake
include(CMakeFindDependencyMacro)

find_dependency(cgride-core CONFIG REQUIRED)
find_dependency(cgride-config CONFIG REQUIRED)
find_dependency(cgride-toolchains CONFIG REQUIRED)
find_dependency(cgride-engine CONFIG REQUIRED)
```

## Current status

This module currently provides:

- CLI argument container
- CLI option parser
- terminal output wrapper
- stable exit codes
- command context
- build command
- run command placeholder
- application dispatcher
- executable entry point
- CMake package integration
- unit tests for the public CLI API

The next step is to connect executable artifact mapping so `cgride run` can build the project and launch the selected target.

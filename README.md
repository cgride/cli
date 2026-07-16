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

From the module directory, use the Vix workflow:

```bash
vix build
```

For a release build:

```bash
vix build --preset release
```

For detailed build output:

```bash
vix build -v
```

## Run tests

```bash
vix check --tests
```

Or run the test command directly:

```bash
vix tests
```

## Install

```bash
vix install
```

The install step provides the `cgride` executable, the `cgride::cli` integration target, public headers, and package metadata.

## Integration

After installation, C++ integrations can use the installed Cgride CLI library target from their project build configuration.


/**
 *
 *  @file terminal.cpp
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
#include <cgride/cli/terminal.hpp>

#include <iostream>

namespace cgride::cli
{
  Terminal::Terminal()
      : output_stream_(&std::cout),
        error_stream_(&std::cerr)
  {
  }

  Terminal::Terminal(
      std::ostream &output,
      std::ostream &error)
      : output_stream_(&output),
        error_stream_(&error)
  {
  }

  Terminal &Terminal::output_stream(std::ostream &stream) noexcept
  {
    output_stream_ = &stream;
    return *this;
  }

  Terminal &Terminal::error_stream(std::ostream &stream) noexcept
  {
    error_stream_ = &stream;
    return *this;
  }

  Terminal &Terminal::quiet(bool value) noexcept
  {
    quiet_ = value;
    return *this;
  }

  Terminal &Terminal::verbose(bool value) noexcept
  {
    verbose_ = value;
    return *this;
  }

  std::ostream &Terminal::output_stream() const noexcept
  {
    return *output_stream_;
  }

  std::ostream &Terminal::error_stream() const noexcept
  {
    return *error_stream_;
  }

  bool Terminal::quiet() const noexcept
  {
    return quiet_;
  }

  bool Terminal::verbose() const noexcept
  {
    return verbose_;
  }

  Terminal &Terminal::write(std::string_view text)
  {
    if (!quiet_)
    {
      output_stream() << text;
    }

    return *this;
  }

  Terminal &Terminal::writeln(std::string_view text)
  {
    if (!quiet_)
    {
      output_stream() << text << '\n';
    }

    return *this;
  }

  Terminal &Terminal::write_error(std::string_view text)
  {
    error_stream() << text;
    return *this;
  }

  Terminal &Terminal::writeln_error(std::string_view text)
  {
    error_stream() << text << '\n';
    return *this;
  }

  Terminal &Terminal::print_info(std::string_view message)
  {
    if (!quiet_)
    {
      output_stream() << "info: " << message << '\n';
    }

    return *this;
  }

  Terminal &Terminal::print_success(std::string_view message)
  {
    if (!quiet_)
    {
      output_stream() << "success: " << message << '\n';
    }

    return *this;
  }

  Terminal &Terminal::print_warning(std::string_view message)
  {
    if (!quiet_)
    {
      output_stream() << "warning: " << message << '\n';
    }

    return *this;
  }

  Terminal &Terminal::print_error(std::string_view message)
  {
    error_stream() << "error: " << message << '\n';
    return *this;
  }

  Terminal &Terminal::print_verbose(std::string_view message)
  {
    if (!quiet_ && verbose_)
    {
      output_stream() << "verbose: " << message << '\n';
    }

    return *this;
  }

} // namespace cgride::cli

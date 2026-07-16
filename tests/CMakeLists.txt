function(cgride_cli_add_test name source)
  add_executable(${name}
    ${source}
  )

  target_link_libraries(${name}
    PRIVATE
      cgride::cli
  )

  target_compile_features(${name}
    PRIVATE
      cxx_std_23
  )

  set_target_properties(${name}
    PROPERTIES
      CXX_EXTENSIONS OFF
  )

  add_test(
    NAME ${name}
    COMMAND ${name}
  )
endfunction()

cgride_cli_add_test(cgride_cli_exit_code_test exit_code_test.cpp)
cgride_cli_add_test(cgride_cli_command_line_test command_line_test.cpp)
cgride_cli_add_test(cgride_cli_cli_options_test cli_options_test.cpp)
cgride_cli_add_test(cgride_cli_terminal_test terminal_test.cpp)
cgride_cli_add_test(cgride_cli_application_test application_test.cpp)

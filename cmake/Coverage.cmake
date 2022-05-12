function(EnableCoverage target)
  if (CMAKE_BUILD_TYPE STREQUAL Debug)
    target_compile_options(${target}
      PUBLIC
        --coverage -fno-inline -fprofile-arcs -ftest-coverage
    )
    
    target_link_options(${target}
      PUBLIC
        --coverage -fprofile-arcs -ftest-coverage
    )
  endif()
endfunction()

function(CleanCoverage target)
  add_custom_command(TARGET ${target} PRE_BUILD
    COMMAND find ${CMAKE_BINARY_DIR} -type f -name '*.gcda' -exec rm {} +)
endfunction()

function(AddCoverage target)
  find_program(LCOV_PATH lcov REQUIRED )
  find_program(GENHTML_PATH genhtml REQUIRED )

  add_custom_target(coverage-${target}
    COMMAND ${LCOV_PATH} --directory . --zerocounters
    COMMAND $<TARGET_FILE:${target}>
    COMMAND ${LCOV_PATH} --capture --directory . -o coverage.info
    COMMAND ${LCOV_PATH} -r coverage.info '/usr/include/*' '*gtest*' -o filtered.info
    COMMAND ${GENHTML_PATH} -o coverage-${target} filtered.info --legend
    COMMAND rm -rf coverage.info filtered.info
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endfunction()

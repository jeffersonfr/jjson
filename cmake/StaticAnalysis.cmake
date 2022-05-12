function(AddChecker target)
  find_program(CHECKER cppcheck REQUIRED)

  set_target_properties(${target}
    PROPERTIES CXX_CPPCHECK "${CHECKER}"
  )
endfunction()

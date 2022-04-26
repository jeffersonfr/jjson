cmake_minimum_required (VERSION 3.0)

include(FetchContent)

FetchContent_Declare(googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.11.0
  )

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

macro(module_test)
  set(id ${ARGV0}_test)

  add_executable(${id} ${id}.cpp)
  add_test(${id} ${id} COMMAND $<TARGET_FILE:${id}>)
  target_link_libraries(${id}
    PRIVATE
      jjson
      gtest_main
  )

  unset(id)
endmacro()

module_test(parser)

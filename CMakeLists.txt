cmake_minimum_required (VERSION 3.0)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "In-source build is not allowed")
endif()

project(jjson
  LANGUAGES CXX
  HOMEPAGE_URL https://github.com/jeffersonfr/jjson
  DESCRIPTION "A json parser"
  VERSION 1.0.0
)

option(TESTS "Enable unit tests" OFF)
option(CHECKER "Enable static code analysing" OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CHECKER)
   set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=-*,android-*,bugprone-*,clang-analyzer-*,concurrency-*,hicpp-*,misc-*,modernize-*,portability-*)
endif()

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

find_package(Threads REQUIRED)

add_library(${PROJECT_NAME} INTERFACE
  )

target_include_directories(${PROJECT_NAME}
  INTERFACE 
    ${PROJECT_SOURCE_DIR}/include
  )

enable_testing()

if (TESTS)
  add_subdirectory(tests)
endif()

configure_file(
  ${PROJECT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}.pc.in
  ${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.pc
  @ONLY
)

include(Doxygen)
Doxygen(include docs)

install(DIRECTORY include/${CMAKE_PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/include)

# generate pc file
configure_file(jjson.pc.in jjson.pc @ONLY)

install(FILES ${CMAKE_BINARY_DIR}/jjson.pc DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)

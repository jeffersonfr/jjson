cmake_minimum_required (VERSION 3.0)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "In-source build is not allowed")
endif()

project (jjson CXX)

option(TESTS "Enable unit tests" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

install(DIRECTORY include/${CMAKE_PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/include)

# generate jjson.pc
configure_file(${PROJECT_SOURCE_DIR}/jjson.pc.in ${PROJECT_BINARY_DIR}/jjson.pc.gen @ONLY)
file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/jjson.pc INPUT "${CMAKE_BINARY_DIR}/jjson.pc.gen")
install(FILES ${CMAKE_BINARY_DIR}/jjson.pc DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)

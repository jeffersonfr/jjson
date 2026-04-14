# CMake generated Testfile for 
# Source directory: /home/jeff/Projects/jjson/tests
# Build directory: /home/jeff/Projects/jjson/build/tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(parser_test "parser_test" "COMMAND" "$<TARGET_FILE:parser_test>")
set_tests_properties(parser_test PROPERTIES  _BACKTRACE_TRIPLES "/home/jeff/Projects/jjson/tests/CMakeLists.txt;23;add_test;/home/jeff/Projects/jjson/tests/CMakeLists.txt;36;module_test;/home/jeff/Projects/jjson/tests/CMakeLists.txt;0;")
subdirs("../_deps/googletest-build")

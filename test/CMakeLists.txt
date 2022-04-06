find_package(Catch2 REQUIRED)

include(CTest)
include(Catch)

add_library(catch_main OBJECT catch_main.cpp)
target_link_libraries(catch_main PUBLIC Catch2::Catch2)
target_link_libraries(catch_main PRIVATE project_options)

# Provide a simple smoke test to make sure that the CLI works and can display a --help message
add_test(NAME cli.has_help COMMAND solve --help)

# Provide a test to verify that the version being reported from the application
# matches the version given to CMake. This will be important once you package
# your program. Real world shows that this is the kind of simple mistake that is easy
# to make, but also easy to test for.
add_test(NAME cli.version_matches COMMAND solve --version)

set_tests_properties(cli.version_matches PROPERTIES PASS_REGULAR_EXPRESSION "${PROJECT_VERSION}")

add_test(NAME cli.dimacs_not_found COMMAND solve --exhaustive --dimacs=nonexistent.dimacs
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set_tests_properties(cli.dimacs_not_found PROPERTIES PASS_REGULAR_EXPRESSION "c solving nonexistent.dimacs\nc Could not open file nonexistent.dimacs\ns UNKNOWN")

add_test(NAME cli.trivial_unsat COMMAND solve --exhaustive --dimacs=test_files/trivial_unsat.dimacs
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set_tests_properties(cli.trivial_unsat PROPERTIES PASS_REGULAR_EXPRESSION "s UNSATISFIABLE")

add_test(NAME cli.empty COMMAND solve --exhaustive --dimacs=test_files/empty.dimacs
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set_tests_properties(cli.empty PROPERTIES PASS_REGULAR_EXPRESSION
                     "c Unhandled exception in main: Invalid dimacs input format - all lines are either empty or commented out\ns UNKNOWN")

add_test(NAME cli.trivial_sat COMMAND solve --exhaustive --dimacs=test_files/trivial_sat.dimacs
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set_tests_properties(cli.trivial_sat PROPERTIES PASS_REGULAR_EXPRESSION "s SATISFIABLE\nv 1 -2 0")

add_executable(tests tests.cpp)
target_link_libraries(tests PRIVATE solver project_warnings project_options catch_main)

# automatically discover tests that are defined in catch based test files you can modify the unittests. Set TEST_PREFIX
# to whatever you want, or use different for different binaries
catch_discover_tests(
  tests
  TEST_PREFIX
  "unittests."
  PROPERTIES VERBOSE 1 TIMEOUT 10
  REPORTER
  xml
  OUTPUT_DIR
  .
  OUTPUT_PREFIX
  "unittests."
  OUTPUT_SUFFIX
  .xml)

# Add a file containing a set of constexpr tests
add_executable(constexpr_tests constexpr_tests.cpp)
target_link_libraries(constexpr_tests PRIVATE project_options project_warnings catch_main)

catch_discover_tests(
  constexpr_tests
  TEST_PREFIX
  "constexpr."
  REPORTER
  xml
  OUTPUT_DIR
  .
  OUTPUT_PREFIX
  "constexpr."
  OUTPUT_SUFFIX
  .xml)

# Disable the constexpr portion of the test, and build again this allows us to have an executable that we can debug when
# things go wrong with the constexpr testing
add_executable(relaxed_constexpr_tests constexpr_tests.cpp)
target_link_libraries(relaxed_constexpr_tests PRIVATE project_options project_warnings catch_main)
target_compile_definitions(relaxed_constexpr_tests PRIVATE -DCATCH_CONFIG_RUNTIME_STATIC_REQUIRE)

catch_discover_tests(
  relaxed_constexpr_tests
  TEST_PREFIX
  "relaxed_constexpr."
  REPORTER
  xml
  OUTPUT_DIR
  .
  OUTPUT_PREFIX
  "relaxed_constexpr."
  OUTPUT_SUFFIX
  .xml)

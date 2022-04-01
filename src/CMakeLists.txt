find_package(fmt CONFIG)
find_package(spdlog CONFIG)
find_package(docopt CONFIG)


add_library(solver STATIC dimacs_parser.cpp)
target_link_libraries(
  solver
  PRIVATE project_options
          project_warnings
          docopt::docopt
          fmt::fmt
)

# Generic test that uses conan libs
add_executable(solve main.cpp)
target_link_libraries(
  solve
  PRIVATE project_options
          project_warnings
          docopt::docopt
          fmt::fmt
          solver
          spdlog::spdlog)

target_include_directories(solve PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")


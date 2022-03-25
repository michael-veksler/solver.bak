find_package(fmt CONFIG)
find_package(spdlog CONFIG)
find_package(docopt CONFIG)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS 1)

add_library(solver SHARED dimacs_parser.cpp)
target_link_libraries(
  solver
  PRIVATE project_options
          project_warnings
          docopt::docopt
          fmt::fmt
)

# Generic test that uses conan libs
add_executable(intro main.cpp)
target_link_libraries(
  intro
  PRIVATE project_options
          project_warnings
          docopt::docopt
          fmt::fmt
          spdlog::spdlog)

target_include_directories(intro PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")


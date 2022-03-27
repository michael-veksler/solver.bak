#include <fmt/format.h>
#include <vector>
#include <string>
#include <sstream>
#include "../src/dimacs_parser.hpp"


// Fuzzer that attempts to invoke undefined behavior for signed integer overflow
// cppcheck-suppress unusedFunction symbolName=LLVMFuzzerTestOneInput
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
  std::string str(reinterpret_cast<const char*>(data), size);
  std::istringstream is(str);

  unsigned n_variables = 0;
  unsigned n_clauses = 0;
    std::vector<std::vector<int>> clauses;
  auto construct_problem = [&](unsigned read_vars, unsigned read_clauses) {
      n_variables = read_vars;
      n_clauses = read_clauses;
  };
  auto register_clause = [&](const std::vector<int> &read_clauses) { clauses.push_back(read_clauses); };
  solver::parse_dimacs(is, construct_problem, register_clause);
  fmt::print("n_variables: {}, n_clauses{} size(clauses)\n", n_variables, n_clauses, std::size(clauses));
  return 0;
}

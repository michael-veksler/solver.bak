#include "../src/dimacs_parser.hpp"
#include <fmt/format.h>
#include <sstream>
#include <string>
#include <vector>


// Fuzzer that attempts to invoke undefined behavior for signed integer overflow
// cppcheck-suppress unusedFunction symbolName=LLVMFuzzerTestOneInput
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    std::string str;
    str.reserve(size);
    bool add_prefix = false;
    bool add_long_prefix = false;
    if (size > 2) {
        add_prefix = *data % 3 != 0;
        add_long_prefix = *data % 3 == 1;
    }
    if (add_prefix) {
        data = std::next(data, 1);
        str = add_long_prefix ? "p cnf " : "p ";
        --size;
    }
    str.insert(str.end(), data, std::next(data, static_cast<ptrdiff_t>(size)));
    std::istringstream is(str);

    unsigned n_variables = 0;
    unsigned n_clauses = 0;
    std::vector<std::vector<int>> clauses;
    auto construct_problem = [&](unsigned read_vars, unsigned read_clauses) {
        n_variables = read_vars;
        n_clauses = read_clauses;
    };
    auto register_clause = [&](const std::vector<int> &read_clauses) { clauses.push_back(read_clauses); };
    try {
        solver::parse_dimacs(is, construct_problem, register_clause);
        fmt::print("n_variables: {}, n_clauses{} size(clauses)\n", n_variables, n_clauses, std::size(clauses));
    } catch (std::runtime_error &ex) {
        fmt::print("caught exception {}\n", ex.what());
    }
    return 0;
}

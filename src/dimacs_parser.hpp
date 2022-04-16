#pragma once
#include <functional>
#include <iosfwd>

namespace solver {
void parse_dimacs(std::istream &in_stream,
    const std::function<void(unsigned, unsigned)> &construct_problem,
    const std::function<void(const std::vector<int> &)> &register_clause);
}
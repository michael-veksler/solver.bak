#include "dimacs_parser.hpp"
#include <cassert>
#include <fmt/format.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace solver {

std::string_view lstrip(std::string_view sv)
{
    sv.remove_prefix(std::min(sv.find_first_not_of("\t "), sv.size()));
    return sv;
}


void parse_dimacs_header(std::istream &in,
    unsigned &line_num,
    const std::function<void(unsigned, unsigned)> &construct_problem)
{
    for (std::string line; std::getline(in, line); ++line_num) {
        if (line_num == std::numeric_limits<unsigned>::max()) {
            throw std::runtime_error("Potentially infinite loop in parse_dimacs_header");
        }
        std::string_view line_view = lstrip(line);
        if (line_view.empty() || line_view[0] == 'c') {
            continue;
        }
        std::istringstream str{ std::string(line_view) };
        std::string cmd;
        std::string format;
        str >> cmd >> format;
        if (!str || cmd != "p" || format != "cnf") {
            throw std::runtime_error(fmt::format(
                "{}: Invalid dimacs input format, expecting a line prefix 'p cnf ' but got '{}'", line_num, line_view));
        }

        int variables = 0;
        int clauses = 0;
        str >> variables >> clauses;
        if (!str || variables < 0 || clauses < 0) {
            throw std::runtime_error(
                fmt::format("{}: Invalid dimacs input format, expecting a header 'p cnf <variables: unsigned int> "
                            "<clauses: unsigned int>' but got '{}'",
                    line_num,
                    line_view));
        }

        line.clear();
        str >> line;
        if (!line.empty()) {
            throw std::runtime_error(
                fmt::format("{}: Invalid dimacs input format, junk after header '{}'", line_num, line));
        }
        construct_problem(static_cast<unsigned>(variables), static_cast<unsigned>(clauses));
        return;
    }
    throw std::runtime_error("Invalid dimacs input format - all lines are either empty or commented out");
}

void parse_dimacs(std::istream &in,
    const std::function<void(unsigned, unsigned)> &construct_problem,
    const std::function<void(const std::vector<int> &)> &register_clause)
{
    unsigned line_num = 1;
    parse_dimacs_header(in, line_num, construct_problem);
    std::string line;
    std::vector<int> literals;
    for (++line_num; std::getline(in, line); ++line_num) {
        if (line_num == std::numeric_limits<unsigned>::max()) {
            throw std::runtime_error("Potentially infinite loop in parse_dimacs");
        }
        std::string_view line_view = lstrip(line);
        if (line_view.empty() || line_view[0] == 'c') {
            continue;
        }

        std::istringstream str{ std::string(line_view) };
        literals.clear();
        bool found_zero = false;
        while (true) {
            int literal = 0;
            if (!(str >> literal)) {
                if (found_zero) {
                    break;
                }
                throw std::runtime_error(
                    fmt::format("{}: Missing 0 at the end of the line for line '{}'", line_num, line_view));
            }
            if (found_zero) {
                throw std::runtime_error(
                    fmt::format("{}: 0 should be only at the end for the line '{}'", line_num, line_view));
            }
            if (literal == 0) {
                found_zero = true;
                continue;
            }
            literals.push_back(literal);
        }
        register_clause(literals);
    }
}
}// namespace solver
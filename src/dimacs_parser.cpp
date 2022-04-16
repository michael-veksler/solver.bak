#include "dimacs_parser.hpp"
#include <array>
#include <cassert>
#include <cstdio>
#include <fmt/format.h>
#include <sstream>
#include <string>
#include <vector>

namespace solver {

std::string_view lstrip(std::string_view view)
{
    view.remove_prefix(std::min(view.find_first_not_of("\t "), view.size()));
    return view;
}

void parse_dimacs_header(std::istream &in_stream,
    unsigned &line_num,
    const std::function<void(unsigned, unsigned)> &construct_problem)
{
    for (std::string line; std::getline(in_stream, line); ++line_num) {
        std::string_view line_view = lstrip(line);
        if (line_view.empty() || line_view[0] == 'c') {
            continue;
        }
        std::istringstream line_stream{ std::string(line_view) };
        std::string cmd;
        std::string format;
        line_stream >> cmd >> format;
        if (!line_stream || cmd != "p" || format != "cnf") {
            throw std::runtime_error(fmt::format(
                "{}: Invalid dimacs input format, expecting a line prefix 'p cnf ' but got '{}'", line_num, line_view));
        }

        int variables = 0;
        int clauses = 0;
        line_stream >> variables >> clauses;
        if (!line_stream || variables < 0 || clauses < 0) {
            throw std::runtime_error(
                fmt::format("{}: Invalid dimacs input format, expecting a header 'p cnf <variables: unsigned int> "
                            "<clauses: unsigned int>' but got '{}'",
                    line_num,
                    line_view));
        }

        std::string tail;
        line_stream >> tail;
        if (!tail.empty()) {
            throw std::runtime_error(
                fmt::format("{}: Invalid dimacs input format, junk after header '{}'", line_num, tail));
        }
        construct_problem(static_cast<unsigned>(variables), static_cast<unsigned>(clauses));
        return;
    }
    throw std::runtime_error("Invalid dimacs input format - all lines are either empty or commented out");
}

void parse_dimacs(std::istream &in_stream,
    const std::function<void(unsigned, unsigned)> &construct_problem,
    const std::function<void(const std::vector<int> &)> &register_clause)
{
    unsigned line_num = 1;
    parse_dimacs_header(in_stream, line_num, construct_problem);
    std::string line;
    for (++line_num; std::getline(in_stream, line); ++line_num) {
        std::string_view line_view = lstrip(line);
        if (line_view.empty() || line_view[0] == 'c') {
            continue;
        }

        std::istringstream str{ std::string(line_view) };
        std::vector<int> literals;
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
#include "dimacs_parser.hpp"
#include <array>
#include <cassert>
#include <fmt/format.h>
#include <sstream>
#include <string>
#include <vector>

namespace solver {

std::string_view lstrip(std::string_view sv)
{
    sv.remove_prefix(std::min(sv.find_first_not_of("\t "), sv.size()));
    return sv;
}

bool is_whitespace(char ch) { return ch == ' ' || ch == '\t' || ch == '\n'; }

// In MSVC with coverage & debug & developer mode `istream >> std::ws` hangs,
// so I had to implement it myself, to overcome the issue.
std::istream &skip_whitespace(std::istream &is)
{
    for (char ch = '\0'; is.get(ch);) {
        if (!is_whitespace(ch)) {
            is.unget();

            break;
        }
    }
    return is;
}

// In MSVC with coverage & debug & developer mode `istream >> string` hangs,
// so I had to implement it myself, to overcome the issue.
std::string get_string(std::istream &is)
{
    std::string ret;
    skip_whitespace(is);
    for (char ch = '\0'; is.get(ch) && !is_whitespace(ch);) {
        ret.push_back(ch);
    }

    return ret;
}

void parse_dimacs_header(std::istream &in,
    unsigned &line_num,
    const std::function<void(unsigned, unsigned)> &construct_problem)
{
    for (std::string line; std::getline(in, line); ++line_num) {
        std::string_view line_view = lstrip(line);
        if (line_view.empty() || line_view[0] == 'c') {
            continue;
        }
        std::istringstream is{ std::string(line_view) };
        std::string cmd = get_string(is);
        std::string format = get_string(is);
        if (!is || cmd != "p" || format != "cnf") {
            throw std::runtime_error(fmt::format(
                "{}: Invalid dimacs input format, expecting a line prefix 'p cnf ' but got '{}'", line_num, line_view));
        }

        int variables = 0;
        int clauses = 0;
        is >> variables >> clauses;
        if (!is || variables < 0 || clauses < 0) {
            throw std::runtime_error(
                fmt::format("{}: Invalid dimacs input format, expecting a header 'p cnf <variables: unsigned int> "
                            "<clauses: unsigned int>' but got '{}'",
                    line_num,
                    line_view));
        }

        std::string tail = get_string(is);
        if (!tail.empty()) {
            throw std::runtime_error(
                fmt::format("{}: Invalid dimacs input format, junk after header '{}'", line_num, tail));
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
#include <array>
#include <catch2/catch.hpp>
#include <sstream>
#include <string>
#include <string_view>

#include "../src/binary_clause.hpp"
#include "../src/dimacs_parser.hpp"
#include "test_constraints.hpp"

using namespace solver::test;

TEST_CASE("get_value() over non-assigned error", "[test_constraint_state]")
{
    const std::set<bool> unset{ { false, true } };

    test_constraint_state<> state{ .m_variables = { unset }, .m_watches = {} };
    test_constraint_state<>::parameter_t param(0);
    REQUIRE(state.get_domain(param) == unset);
    REQUIRE_THROWS_AS(state.get_value(param), std::domain_error);
}


struct dimacs_parse_case
{
    explicit dimacs_parse_case(const std::string &text)
    {
        std::istringstream text_stream{ text };
        auto construct_problem = [this](unsigned read_vars, unsigned read_clauses) {
            this->n_variables = read_vars;
            this->n_clauses = read_clauses;
        };
        auto register_clause = [this](const std::vector<int> &read_clauses) { clauses.push_back(read_clauses); };

        solver::parse_dimacs(text_stream, construct_problem, register_clause);
    }
    std::vector<std::vector<int>> clauses;
    unsigned n_clauses = 0;
    unsigned n_variables = 0;
};

TEST_CASE("dimacs empty input", "[dimacs_parser]")
{
    REQUIRE_THROWS_MATCHES(dimacs_parse_case(""),
        std::runtime_error,
        Catch::Matchers::Message("Invalid dimacs input format - all lines are either empty or commented out"));
}

TEST_CASE("dimacs bad header prefix", "[dimacs_parser]")
{
    REQUIRE_THROWS_MATCHES(dimacs_parse_case("p cn 2 3"),
        std::runtime_error,
        Catch::Matchers::Message(
            "1: Invalid dimacs input format, expecting a line prefix 'p cnf ' but got 'p cn 2 3'"));
}

TEST_CASE("dimacs header prefix numbers", "[dimacs_parser]")
{
    REQUIRE_THROWS_MATCHES(dimacs_parse_case(R"(c foo
                                                p cnf -3 2)"),
        std::runtime_error,
        Catch::Matchers::Message("2: Invalid dimacs input format, expecting a header 'p cnf <variables: unsigned int> "
                                 "<clauses: unsigned int>' but got 'p cnf -3 2'"));
}

TEST_CASE("dimacs junk at header end", "[dimacs_parser]")
{
    REQUIRE_THROWS_MATCHES(dimacs_parse_case(R"(p cnf 2 3 4
                                                1 2 0)"),
        std::runtime_error,
        Catch::Matchers::Message("1: Invalid dimacs input format, junk after header '4'"));
}

TEST_CASE("dimacs n_variables overflow", "[dimacs_parser]")
{
    REQUIRE_THROWS_MATCHES(dimacs_parse_case(R"(p cnf 2147483648 3
                                                1 2 0)"),
        std::runtime_error,
        Catch::Matchers::Message("1: Invalid dimacs input format, expecting a header 'p cnf <variables: unsigned int> "
                                 "<clauses: unsigned int>' but got 'p cnf 2147483648 3'"));
}

TEST_CASE("dimacs n_variables almost overflow", "[dimacs_parser]")
{
    dimacs_parse_case parse_result = dimacs_parse_case(R"(p cnf 2147483647 3
                                                         1 2 0)");
    REQUIRE(parse_result.n_variables == 2147483647U);
}

TEST_CASE("dimacs invalid 0 in clause middle", "[dimacs_parser]")
{
    REQUIRE_THROWS_MATCHES(dimacs_parse_case(R"(
                                                p cnf 10 20
                                                1 -2 0
                                                2 0 3 0)"),
        std::runtime_error,
        Catch::Matchers::Message("4: 0 should be only at the end for the line '2 0 3 0'"));
}

TEST_CASE("dimacs missing 0 at clause end", "[dimacs_parser]")
{
    REQUIRE_THROWS_MATCHES(dimacs_parse_case(R"(p      cnf  10  20
                                                1 -2 3
                                                2 2 3 0)"),
        std::runtime_error,
        Catch::Matchers::Message("2: Missing 0 at the end of the line for line '1 -2 3'"));
}

TEST_CASE("dimacs parse", "[dimacs_parser]")
{
    dimacs_parse_case parse_result = dimacs_parse_case(R"(
        p cnf 4 5
        1 -2 3 0
        2 3 0
        -1 2 -3 4 0
        1 -2 -3 -4 0
    )");
    CHECK(parse_result.n_variables == 4);
    CHECK(parse_result.n_clauses == 5);
    CHECK(parse_result.clauses
          == std::vector<std::vector<int>>{ { 1, -2, 3 }, { 2, 3 }, { -1, 2, -3, 4 }, { 1, -2, -3, -4 } });
}
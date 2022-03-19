#include <catch2/catch.hpp>

#include "../src/binary_clause.hpp"
#include "test_constraints.hpp"

using namespace solver::test;

TEST_CASE("is_satisfied", "[binary_clause]")
{
    const test_constraint_state initial_state{ .m_variables = { { false }, { true }, { false }, { true } },
        .m_watches = {} };
    test_constraint_state state = initial_state;
    solver::binary_clause<test_constraint_state> clause{ { 0, 1, 2, 3 }, { true, false, true, false } };
    CHECK(!clause.is_satisfied(state));
    state.m_variables[0] = { true };
    CHECK(clause.is_satisfied(state));

    state = initial_state;
    state.m_variables[1] = { false };
    CHECK(clause.is_satisfied(state));

    state = initial_state;
    state.m_variables[2] = { true };
    CHECK(clause.is_satisfied(state));

    state = initial_state;
    state.m_variables[3] = { false };
    CHECK(clause.is_satisfied(state));
}


TEST_CASE("propagate", "[binary_clause]")
{
    const test_constraint_state initial_state{ .m_variables = std::vector<std::set<bool>>(4, { false, true }),
        .m_watches = {} };
    solver::binary_clause<test_constraint_state> clause{ { 0, 1, 2, 3 }, { true, false, true, false } };
    test_constraint_state state = initial_state;
    state.m_variables[0] = { false };
    REQUIRE(clause.propagate(state, 0) == solver::propagation_result_t::CONSISTENT);
    REQUIRE(state.m_watches == std::set<unsigned>{2});
    state.m_variables[2] = { true };
    REQUIRE(clause.propagate(state, 2) == solver::propagation_result_t::SAT);
    REQUIRE(state.m_watches == std::set<unsigned>{2});

    state.m_variables = initial_state.m_variables;
    state.m_variables[2] = {false};
    REQUIRE(clause.propagate(state, 2) == solver::propagation_result_t::CONSISTENT);
    REQUIRE(state.m_watches == std::set<unsigned>{3});
    
    state.m_variables[1] = {true};
    REQUIRE(clause.propagate(state, 1) == solver::propagation_result_t::CONSISTENT);
    REQUIRE(state.m_watches == std::set<unsigned>{0,3});
    
    state.m_variables[3] = {true};
    state.m_variables[0] = {false};
    REQUIRE(clause.propagate(state, 1) == solver::propagation_result_t::UNSAT);
    REQUIRE(state.m_watches == std::set<unsigned>{0,3});
    
    state.m_variables = initial_state.m_variables;
    state.m_variables[3] = {true};
    REQUIRE(clause.propagate(state, 3) == solver::propagation_result_t::CONSISTENT);
    
}

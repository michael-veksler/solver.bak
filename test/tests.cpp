#include <catch2/catch.hpp>

#include "../src/binary_clause.hpp"
#include "test_constraints.hpp"

using namespace solver::test;
using solver::propagation_result_t;

TEST_CASE("is_satisfied", "[binary_clause]")
{
    const test_constraint_state<> initial_state{ .m_variables = { { false }, { true }, { false }, { true } },
        .m_watches = {} };
    test_constraint_state<> state = initial_state;
    solver::binary_clause<test_constraint_state<>> clause{ { 0, 1, 2, 3 }, { true, false, true, false } };
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

struct propagation_operation
{
    std::vector<std::set<bool>> variables;
    unsigned trigger_param;
    std::set<unsigned> expect_watches;
    propagation_result_t result;
};


TEST_CASE("propagate", "[binary_clause]")
{
    using std::vector;
    using std::set;
    const set<bool> unset{ { false, true } };
    const vector<propagation_operation> sequence{ { .variables = { { false }, unset, unset, unset },
                                                      .trigger_param = 0,
                                                      .expect_watches = { 2 },
                                                      .result = propagation_result_t::CONSISTENT },
        { .variables = { { false }, unset, { true }, unset },
            .trigger_param = 2,
            .expect_watches = { 2 },
            .result = propagation_result_t::SAT },
        { .variables = { unset, unset, { false }, unset },
            .trigger_param = 2,
            .expect_watches = { 3 },
            .result = propagation_result_t::CONSISTENT },
        { .variables = { unset, { true }, { false }, unset },
            .trigger_param = 1,
            .expect_watches = { 0, 3 },
            .result = propagation_result_t::CONSISTENT },
        { .variables = { { false }, { true }, { false }, { true } },
            .trigger_param = 3,
            .expect_watches = { 0, 3 },
            .result = propagation_result_t::UNSAT },
        { .variables = { unset, unset, unset, { true } },
            .trigger_param = 3,
            .expect_watches = { 0, 1 },
            .result = propagation_result_t::CONSISTENT } };

    auto run_test = [&](auto clause) {
        typename decltype(clause)::state_t state;
        for (const propagation_operation &op : sequence) {
            state.m_variables = op.variables;
            state.m_variables = op.variables;
            REQUIRE(clause.propagate(state, op.trigger_param) == op.result);
            REQUIRE(state.m_watches == op.expect_watches);
        }
    };
    solver::binary_clause<test_constraint_state<>> strict_clause{ { 0, 1, 2, 3 }, { true, false, true, false } };
    run_test(strict_clause);
    solver::binary_clause<test_constraint_state<false>> lax_clause{ { 0, 1, 2, 3 }, { true, false, true, false } };
    run_test(lax_clause);
}

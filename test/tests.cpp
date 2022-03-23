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

TEST_CASE("get_value() over non-assigned error", "[test_constraint_state]")
{
    const std::set<bool> unset{ { false, true } };

    test_constraint_state<> state{ .m_variables = { unset }, .m_watches = {} };
    test_constraint_state<>::parameter_t param(0);
    REQUIRE(state.get_domain(param) == unset);
    REQUIRE_THROWS_AS(state.get_value(param), std::domain_error);
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
            .result = propagation_result_t::CONSISTENT },
        { .variables = { { false }, { true }, { false }, { false } },
            .trigger_param = 0,
            .expect_watches = { 0, 1 },
            .result = propagation_result_t::SAT } };
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

TEST_CASE("propagate set_domain", "[binary_clause]")
{
    using std::set;
    using std::vector;
    const set<bool> unset{ { false, true } };
    auto run_test = [&](auto clause) {
        typename decltype(clause)::state_t state;
        state.m_variables = {{false}, unset, {false}, {true}};
        REQUIRE(clause.propagate(state, 0) == propagation_result_t::SAT);
        REQUIRE(state.m_variables == vector<set<bool>>{{false}, {false}, {false}, {true}});
    };

    solver::binary_clause<test_constraint_state<>> strict_clause{ { 0, 1, 2, 3 }, { true, false, true, false } };
    run_test(strict_clause);
    solver::binary_clause<test_constraint_state<false>> lax_clause{ { 0, 1, 2, 3 }, { true, false, true, false } };
    run_test(lax_clause);
}

TEST_CASE("propagate unwatched only_watch_trigger=false", "[binary_clause]")
{

    const std::set<bool> unset{ { false, true } };
    solver::binary_clause<test_constraint_state<false>> clause{ { 0, 1, 2, 3 }, { true, false, true, false } };
    decltype(clause)::state_t state{ .m_variables = { unset, unset, unset, { true } }, .m_watches = {} };
    REQUIRE(clause.propagate(state, 3) == propagation_result_t::CONSISTENT);
    REQUIRE(state.m_watches == std::set<unsigned>{ 1 });
    REQUIRE_THROWS_AS(clause.propagate(state, 4), std::out_of_range);
}

TEST_CASE("propagate unwatched only_watch_trigger=true", "[binary_clause]")
{

    const std::set<bool> unset{ { false, true } };
    solver::binary_clause<test_constraint_state<true>> clause{ { 0, 1, 2, 3 }, { true, false, true, false } };
    decltype(clause)::state_t state{ .m_variables = { unset, unset, unset, { true } }, .m_watches = {} };
    REQUIRE_THROWS_AS(clause.propagate(state, 3), std::domain_error);
    REQUIRE(state.m_watches.empty());
}

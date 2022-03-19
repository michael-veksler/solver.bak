#include <catch2/catch.hpp>

#include "../src/binary_clause.hpp"
#include "test_constraints.hpp"

using namespace solver::test;

TEST_CASE("concepts", "[parameters]")
{
    STATIC_REQUIRE(solver::constraint<solver::binary_clause<test_constraint_state>>);
    STATIC_REQUIRE(solver::constraint_state<test_constraint_state>);
}

#include "tests/test_harness.hpp"
#include "xenith/model/canonical_model.hpp"

using namespace xenith;
using namespace xenith::model;

XENITH_TEST(TestCanonicalModelEmpty) {
    CanonicalModel model("empty_model");
    XENITH_CHECK_EQ(model.name(), "empty_model");
    XENITH_CHECK_EQ(model.numVariables(), 0);
    XENITH_CHECK_EQ(model.numConstraints(), 0);
    XENITH_CHECK_EQ(static_cast<int>(model.sense()), static_cast<int>(ObjectiveSense::MINIMIZE));
}

XENITH_TEST(TestCanonicalModelAddVariables) {
    CanonicalModel model("var_model");
    Index x1 = model.addVariable("x1", 0.0, 10.0, 3.0, VariableType::CONTINUOUS);
    Index x2 = model.addVariable("x2", -5.0, K_INFINITY, -2.5, VariableType::GENERAL_INTEGER);
    Index x3 = model.addVariable("x3", 0.0, 1.0, 0.0, VariableType::BINARY);

    XENITH_CHECK_EQ(x1, 0);
    XENITH_CHECK_EQ(x2, 1);
    XENITH_CHECK_EQ(x3, 2);
    XENITH_CHECK_EQ(model.numVariables(), 3);

    XENITH_CHECK_NEAR(model.colLower()[0], 0.0, 1e-12);
    XENITH_CHECK_NEAR(model.colUpper()[0], 10.0, 1e-12);
    XENITH_CHECK_NEAR(model.objective()[0], 3.0, 1e-12);
    XENITH_CHECK_EQ(static_cast<int>(model.varTypes()[0]), static_cast<int>(VariableType::CONTINUOUS));

    XENITH_CHECK_EQ(static_cast<int>(model.varTypes()[1]), static_cast<int>(VariableType::GENERAL_INTEGER));
    XENITH_CHECK_EQ(static_cast<int>(model.varTypes()[2]), static_cast<int>(VariableType::BINARY));

    // Name lookup
    auto idx1 = model.getVariableIndex("x1");
    XENITH_CHECK(idx1.has_value());
    XENITH_CHECK_EQ(idx1.value(), 0);

    auto idx_none = model.getVariableIndex("non_existent");
    XENITH_CHECK(!idx_none.has_value());

    // Duplicate variable name exception
    XENITH_CHECK_THROW(model.addVariable("x1", 0.0, 1.0, 1.0), std::invalid_argument);
}

XENITH_TEST(TestCanonicalModelAddConstraints) {
    CanonicalModel model("constraint_model");
    Index r1 = model.addLessOrEqualRow("c_le", 10.0);
    Index r2 = model.addGreaterOrEqualRow("c_ge", 5.0);
    Index r3 = model.addEqualityRow("c_eq", 7.0);
    Index r4 = model.addRangeRow("c_range", 2.0, 8.0);

    XENITH_CHECK_EQ(model.numConstraints(), 4);

    // c_le: -inf <= Ax <= 10.0
    XENITH_CHECK(isNegativeInfinity(model.rowLower()[r1]));
    XENITH_CHECK_NEAR(model.rowUpper()[r1], 10.0, 1e-12);

    // c_ge: 5.0 <= Ax <= +inf
    XENITH_CHECK_NEAR(model.rowLower()[r2], 5.0, 1e-12);
    XENITH_CHECK(isPositiveInfinity(model.rowUpper()[r2]));

    // c_eq: 7.0 <= Ax <= 7.0
    XENITH_CHECK_NEAR(model.rowLower()[r3], 7.0, 1e-12);
    XENITH_CHECK_NEAR(model.rowUpper()[r3], 7.0, 1e-12);

    // c_range: 2.0 <= Ax <= 8.0
    XENITH_CHECK_NEAR(model.rowLower()[r4], 2.0, 1e-12);
    XENITH_CHECK_NEAR(model.rowUpper()[r4], 8.0, 1e-12);

    // Duplicate row name exception
    XENITH_CHECK_THROW(model.addEqualityRow("c_le", 5.0), std::invalid_argument);
}

int main() {
    return xenith::test::TestRunner::instance().runAll();
}

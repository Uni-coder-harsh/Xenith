#include "tests/test_harness.hpp"
#include "xenith/presolve/presolver.hpp"
#include "xenith/model/canonical_model.hpp"

using namespace xenith;
using namespace xenith::presolve;
using namespace xenith::model;

XENITH_TEST(TestPresolverEmptyRowRemoval) {
    CanonicalModel model;
    model.addVariable("x", 0.0, 10.0, 1.0);
    model.addVariable("y", 0.0, 10.0, 1.0);
    model.addRow("r1", 0.0, 5.0);
    model.addRow("empty", -1.0, 1.0); // Will be removed
    
    std::vector<numerics::Triplet> triplets = {
        {0, 0, 1.0}, {0, 1, 1.0}
    };
    model.setMatrixA(numerics::SparseMatrix::fromTriplets(2, 2, triplets));
    
    Presolver presolver;
    auto result = presolver.presolve(model);
    
    XENITH_CHECK(!result.was_infeasible);
    XENITH_CHECK_EQ(result.reduced_model.numConstraints(), 1);
    XENITH_CHECK_EQ(result.constraints_removed, 1);
}

XENITH_TEST(TestPresolverFixedVariable) {
    CanonicalModel model;
    model.addVariable("x", 5.0, 5.0, 1.0); // Fixed
    model.addVariable("y", 0.0, 10.0, 2.0);
    model.addRow("r1", 0.0, 10.0);
    
    std::vector<numerics::Triplet> triplets = {
        {0, 0, 1.0}, {0, 1, 1.0}
    };
    model.setMatrixA(numerics::SparseMatrix::fromTriplets(1, 2, triplets));
    
    Presolver presolver;
    auto result = presolver.presolve(model);
    
    XENITH_CHECK(!result.was_infeasible);
    XENITH_CHECK_EQ(result.reduced_model.numVariables(), 1);
    XENITH_CHECK_EQ(result.vars_removed, 1);
    
    std::vector<double> reduced_sol = {2.0};
    auto full_sol = Presolver::postsolve(reduced_sol, result);
    XENITH_CHECK_EQ(full_sol.size(), 2);
    XENITH_CHECK_NEAR(full_sol[0], 5.0, 1e-9);
    XENITH_CHECK_NEAR(full_sol[1], 2.0, 1e-9);
}

XENITH_TEST(TestPresolverSingletonRow) {
    CanonicalModel model;
    model.addVariable("x", 0.0, 10.0, 1.0);
    model.addRow("r1", 2.0, 8.0); // 2 <= 2*x <= 8 => 1 <= x <= 4
    
    std::vector<numerics::Triplet> triplets = {
        {0, 0, 2.0}
    };
    model.setMatrixA(numerics::SparseMatrix::fromTriplets(1, 1, triplets));
    
    Presolver presolver;
    auto result = presolver.presolve(model);
    
    XENITH_CHECK(!result.was_infeasible);
    XENITH_CHECK_EQ(result.reduced_model.numConstraints(), 0);
    XENITH_CHECK_EQ(result.constraints_removed, 1);
    XENITH_CHECK_NEAR(result.reduced_model.colLower()[0], 1.0, 1e-9);
    XENITH_CHECK_NEAR(result.reduced_model.colUpper()[0], 4.0, 1e-9);
}

XENITH_TEST(TestPresolverEmptyColumn) {
    CanonicalModel model;
    model.addVariable("x", 1.0, 10.0, 2.0);
    model.addRow("r1", 0.0, 10.0);
    
    model.setMatrixA(numerics::SparseMatrix(1, 1, {0, 0}, {}, {}));
    
    Presolver presolver;
    auto result = presolver.presolve(model);
    
    XENITH_CHECK(!result.was_infeasible);
    XENITH_CHECK_EQ(result.reduced_model.numVariables(), 0);
    XENITH_CHECK_EQ(result.vars_removed, 1);
    
    std::vector<double> reduced_sol = {};
    auto full_sol = Presolver::postsolve(reduced_sol, result);
    XENITH_CHECK_NEAR(full_sol[0], 1.0, 1e-9);
}

int main() {
    return xenith::test::TestRunner::instance().runAll();
}

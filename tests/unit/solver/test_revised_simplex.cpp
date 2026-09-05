#include <iostream>
#include <vector>
#include "xenith/model/canonical_model.hpp"
#include "xenith/solver/lp/revised_simplex_solver.hpp"
#include "xenith/solution/lp_solution_validator.hpp"
#include "tests/test_harness.hpp"

using namespace xenith;
using namespace xenith::model;
using namespace xenith::solver;
using namespace xenith::solution;

XENITH_TEST(TestA_SimpleBoundedLP) {
    // max x + y s.t. x + y <= 4, x <= 2, y <= 3, x,y >= 0
    CanonicalModel model("TestA");
    model.setSense(ObjectiveSense::MAXIMIZE);
    Index x = model.addVariable("x", 0.0, 2.0, 1.0);
    Index y = model.addVariable("y", 0.0, 3.0, 1.0);

    // x + y <= 4
    Index r1 = model.addLessOrEqualRow("r1", 4.0);

    std::vector<numerics::Triplet> triplets = {
        {r1, x, 1.0}, {r1, y, 1.0}
    };
    model.setMatrixA(numerics::SparseMatrix::fromTriplets(1, 2, triplets));

    RevisedSimplexSolver solver;
    SolveResult res = solver.solve(model);

    XENITH_CHECK(res.status == ModelStatus::OPTIMAL);
    XENITH_CHECK_NEAR(res.objective_value, 4.0, 1e-6);
    XENITH_CHECK_NEAR(res.primal_solution[x], 2.0, 1e-6);
    XENITH_CHECK_NEAR(res.primal_solution[y], 2.0, 1e-6);

    auto val = LpSolutionValidator::validate(model, res);
    XENITH_CHECK(val.isValid);
}

XENITH_TEST(TestB_EqualityConstrainedLP) {
    // min 2x + 3y s.t. x + y = 5, x,y >= 0
    CanonicalModel model("TestB");
    model.setSense(ObjectiveSense::MINIMIZE);
    Index x = model.addVariable("x", 0.0, K_INFINITY, 2.0);
    Index y = model.addVariable("y", 0.0, K_INFINITY, 3.0);

    Index r1 = model.addEqualityRow("r1", 5.0);
    std::vector<numerics::Triplet> triplets = {
        {r1, x, 1.0}, {r1, y, 1.0}
    };
    model.setMatrixA(numerics::SparseMatrix::fromTriplets(1, 2, triplets));

    RevisedSimplexSolver solver;
    SolveResult res = solver.solve(model);

    XENITH_CHECK(res.status == ModelStatus::OPTIMAL);
    XENITH_CHECK_NEAR(res.objective_value, 10.0, 1e-6);
    XENITH_CHECK_NEAR(res.primal_solution[x], 5.0, 1e-6);
    XENITH_CHECK_NEAR(res.primal_solution[y], 0.0, 1e-6);

    auto val = LpSolutionValidator::validate(model, res);
    XENITH_CHECK(val.isValid);
}

XENITH_TEST(TestC_InfeasibleLP) {
    // min x s.t. x <= 1, x >= 3
    CanonicalModel model("TestC");
    Index x = model.addVariable("x", 0.0, K_INFINITY, 1.0);

    Index r1 = model.addLessOrEqualRow("r1", 1.0);
    Index r2 = model.addGreaterOrEqualRow("r2", 3.0);

    std::vector<numerics::Triplet> triplets = {
        {r1, x, 1.0}, {r2, x, 1.0}
    };
    model.setMatrixA(numerics::SparseMatrix::fromTriplets(2, 1, triplets));

    RevisedSimplexSolver solver;
    SolveResult res = solver.solve(model);

    XENITH_CHECK(res.status == ModelStatus::INFEASIBLE);
}

XENITH_TEST(TestD_UnboundedLP) {
    // max x s.t. x >= 0
    CanonicalModel model("TestD");
    model.setSense(ObjectiveSense::MAXIMIZE);
    model.addVariable("x", 0.0, K_INFINITY, 1.0);

    RevisedSimplexSolver solver;
    SolveResult res = solver.solve(model);

    XENITH_CHECK(res.status == ModelStatus::UNBOUNDED);
}

XENITH_TEST(TestE_FixedVariableLP) {
    // min x + y s.t. x = 2, y >= 1
    CanonicalModel model("TestE");
    Index x = model.addVariable("x", 2.0, 2.0, 1.0);
    Index y = model.addVariable("y", 1.0, K_INFINITY, 1.0);

    RevisedSimplexSolver solver;
    SolveResult res = solver.solve(model);

    XENITH_CHECK(res.status == ModelStatus::OPTIMAL);
    XENITH_CHECK_NEAR(res.objective_value, 3.0, 1e-6);
    XENITH_CHECK_NEAR(res.primal_solution[x], 2.0, 1e-6);
    XENITH_CHECK_NEAR(res.primal_solution[y], 1.0, 1e-6);
}

XENITH_TEST(TestF_FreeVariableLP) {
    // min x + 2y s.t. x + y >= 4, x free, y >= 0
    CanonicalModel model("TestF");
    Index x = model.addVariable("x", -K_INFINITY, K_INFINITY, 1.0);
    Index y = model.addVariable("y", 0.0, K_INFINITY, 2.0);

    Index r1 = model.addGreaterOrEqualRow("r1", 4.0);
    std::vector<numerics::Triplet> triplets = {
        {r1, x, 1.0}, {r1, y, 1.0}
    };
    model.setMatrixA(numerics::SparseMatrix::fromTriplets(1, 2, triplets));

    RevisedSimplexSolver solver;
    SolveResult res = solver.solve(model);



    XENITH_CHECK(res.status == ModelStatus::OPTIMAL);
    XENITH_CHECK_NEAR(res.objective_value, 4.0, 1e-6);
    XENITH_CHECK_NEAR(res.primal_solution[x], 4.0, 1e-6);
    XENITH_CHECK_NEAR(res.primal_solution[y], 0.0, 1e-6);
}

XENITH_TEST(TestG_DegenerateLP) {
    // min x + y s.t. x + y <= 0, x,y >= 0
    CanonicalModel model("TestG");
    Index x = model.addVariable("x", 0.0, K_INFINITY, 1.0);
    Index y = model.addVariable("y", 0.0, K_INFINITY, 1.0);

    Index r1 = model.addLessOrEqualRow("r1", 0.0);
    std::vector<numerics::Triplet> triplets = {
        {r1, x, 1.0}, {r1, y, 1.0}
    };
    model.setMatrixA(numerics::SparseMatrix::fromTriplets(1, 2, triplets));

    RevisedSimplexSolver solver;
    SolveResult res = solver.solve(model);

    XENITH_CHECK(res.status == ModelStatus::OPTIMAL);
    XENITH_CHECK_NEAR(res.objective_value, 0.0, 1e-6);
}

XENITH_TEST(TestI_MaximizeModel) {
    // max 3x + 5y s.t. x + 2y <= 8, 3x + 2y <= 12, x,y >= 0
    // Optimal: x = 2, y = 3, obj = 21.
    CanonicalModel model("TestI");
    model.setSense(ObjectiveSense::MAXIMIZE);
    Index x = model.addVariable("x", 0.0, K_INFINITY, 3.0);
    Index y = model.addVariable("y", 0.0, K_INFINITY, 5.0);

    Index r1 = model.addLessOrEqualRow("r1", 8.0);
    Index r2 = model.addLessOrEqualRow("r2", 12.0);

    std::vector<numerics::Triplet> triplets = {
        {r1, x, 1.0}, {r1, y, 2.0},
        {r2, x, 3.0}, {r2, y, 2.0}
    };
    model.setMatrixA(numerics::SparseMatrix::fromTriplets(2, 2, triplets));

    RevisedSimplexSolver solver;
    SolveResult res = solver.solve(model);

    XENITH_CHECK(res.status == ModelStatus::OPTIMAL);
    XENITH_CHECK_NEAR(res.objective_value, 21.0, 1e-6);
    XENITH_CHECK_NEAR(res.primal_solution[x], 2.0, 1e-6);
    XENITH_CHECK_NEAR(res.primal_solution[y], 3.0, 1e-6);

    auto val = LpSolutionValidator::validate(model, res);
    XENITH_CHECK(val.isValid);
}

XENITH_TEST(TestJ_MinimizeModel) {
    // min 4x + 2y s.t. x + 2y >= 6, 3x + y >= 8, x,y >= 0
    // Optimal: x = 2, y = 2, obj = 12.
    CanonicalModel model("TestJ");
    model.setSense(ObjectiveSense::MINIMIZE);
    Index x = model.addVariable("x", 0.0, K_INFINITY, 4.0);
    Index y = model.addVariable("y", 0.0, K_INFINITY, 2.0);

    Index r1 = model.addGreaterOrEqualRow("r1", 6.0);
    Index r2 = model.addGreaterOrEqualRow("r2", 8.0);

    std::vector<numerics::Triplet> triplets = {
        {r1, x, 1.0}, {r1, y, 2.0},
        {r2, x, 3.0}, {r2, y, 1.0}
    };
    model.setMatrixA(numerics::SparseMatrix::fromTriplets(2, 2, triplets));

    RevisedSimplexSolver solver;
    SolveResult res = solver.solve(model);

    XENITH_CHECK(res.status == ModelStatus::OPTIMAL);
    XENITH_CHECK_NEAR(res.objective_value, 12.0, 1e-6);
    XENITH_CHECK_NEAR(res.primal_solution[x], 2.0, 1e-6);
    XENITH_CHECK_NEAR(res.primal_solution[y], 2.0, 1e-6);

    auto val = LpSolutionValidator::validate(model, res);
    XENITH_CHECK(val.isValid);
}

int main() {
    return xenith::test::TestRunner::instance().runAll();
}

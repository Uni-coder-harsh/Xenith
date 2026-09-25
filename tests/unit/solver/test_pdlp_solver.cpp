#include "tests/test_harness.hpp"
#include "xenith/solver/lp/pdlp_solver.hpp"
#include "xenith/io/mps/mps_reader.hpp"
#include "xenith/solution/lp_solution_validator.hpp"
#include "xenith/common/constants.hpp"
#include <filesystem>
#include <vector>
#include <cmath>
#include <iostream>

namespace fs = std::filesystem;
using namespace xenith;
using namespace xenith::solver;
using namespace xenith::model;

static std::string resolveFixturePath(const std::string& relative_path) {
    std::vector<std::string> candidates = {
        relative_path,
        "../" + relative_path,
        "../../" + relative_path
    };

    for (const auto& candidate : candidates) {
        if (fs::exists(candidate)) {
            return candidate;
        }
    }
    return relative_path;
}

XENITH_TEST(PdlpTrivialModel) {
    CanonicalModel model("trivial");
    model.setObjectiveOffset(42.0);

    PdlpSolver solver;
    PdlpResult result = solver.solve(model);

    XENITH_CHECK(result.status == ModelStatus::OPTIMAL);
    XENITH_CHECK_NEAR(result.objective_value, 42.0, 1e-9);
}

XENITH_TEST(PdlpSimpleLP) {
    // min -2 x1 - 3 x2
    // s.t.
    //   x1 + x2 <= 4
    //   x1 + 2 x2 <= 5
    //   x1, x2 >= 0
    // Optimum at x1 = 3, x2 = 1, obj = -9.0
    CanonicalModel model("simple_lp");
    model.addVariable("x1", 0.0, K_INFINITY, -2.0);
    model.addVariable("x2", 0.0, K_INFINITY, -3.0);

    model.addLessOrEqualRow("c1", 4.0);
    model.addLessOrEqualRow("c2", 5.0);

    std::vector<numerics::Triplet> triplets = {
        {0, 0, 1.0}, {0, 1, 1.0},
        {1, 0, 1.0}, {1, 1, 2.0}
    };
    model.setMatrixA(numerics::SparseMatrix::fromTriplets(2, 2, triplets));

    PdlpOptions opts;
    opts.tolerance = 1e-5;
    opts.verbose = false;

    PdlpSolver solver(opts);
    PdlpResult result = solver.solve(model);

    XENITH_CHECK(result.status == ModelStatus::OPTIMAL);
    XENITH_CHECK_NEAR(result.objective_value, -9.0, 1e-2);
    XENITH_CHECK_NEAR(result.primal_solution[0], 3.0, 1e-2);
    XENITH_CHECK_NEAR(result.primal_solution[1], 1.0, 1e-2);

    auto val = solution::LpSolutionValidator::validate(model, result, 1e-3);
    XENITH_CHECK(val.isValid);
}

XENITH_TEST(PdlpInfeasibleModel) {
    // min x1 s.t. x1 >= 5 and x1 <= 2
    CanonicalModel model("infeasible");
    model.addVariable("x1", 5.0, 2.0, 1.0); // l_x > u_x

    PdlpSolver solver;
    PdlpResult result = solver.solve(model);

    XENITH_CHECK(result.status == ModelStatus::INFEASIBLE);
}

XENITH_TEST(PdlpNetlibAfiro) {
    std::string mps_path = resolveFixturePath("tests/data/mps/afiro.mps");
    auto model = io::mps::MpsReader::readFromFile(mps_path);

    PdlpOptions opts;
    opts.tolerance = 1e-5;
    opts.max_iterations = 20000;
    opts.check_interval = 40;
    opts.verbose = false;

    PdlpSolver solver(opts);
    PdlpResult result = solver.solve(model);

    XENITH_CHECK(result.status == ModelStatus::OPTIMAL);

    // Known optimal objective for Netlib AFIRO is -464.75314286...
    XENITH_CHECK_NEAR(result.objective_value, -464.75314286, 0.5);

    // Validate primal solution against canonical model bounds and constraints
    auto val = solution::LpSolutionValidator::validate(model, result, 1e-2);
    XENITH_CHECK(val.isValid);
}

int main() {
    return xenith::test::TestRunner::instance().runAll();
}

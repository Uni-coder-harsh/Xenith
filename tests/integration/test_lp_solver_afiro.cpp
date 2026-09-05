#include <iostream>
#include <vector>
#include <filesystem>
#include "xenith/io/mps/mps_reader.hpp"
#include "xenith/solver/lp/revised_simplex_solver.hpp"
#include "xenith/solution/lp_solution_validator.hpp"
#include "tests/test_harness.hpp"

namespace fs = std::filesystem;
using namespace xenith;

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

XENITH_TEST(TestSolveAfiroMps) {
    std::string mps_path = resolveFixturePath("tests/data/mps/afiro.mps");
    auto model = io::mps::MpsReader::readFromFile(mps_path);

    solver::RevisedSimplexSolver solver;
    solver::SolveResult result = solver.solve(model);

    XENITH_CHECK(result.status == ModelStatus::OPTIMAL);

    // Known optimal objective for Netlib AFIRO is -464.75314286...
    XENITH_CHECK_NEAR(result.objective_value, -464.75314286, 1e-3);

    // Independent solution validation
    auto val = solution::LpSolutionValidator::validate(model, result);
    XENITH_CHECK(val.isValid);
}

int main() {
    return xenith::test::TestRunner::instance().runAll();
}

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>
#include <filesystem>
#include "xenith/io/mps/mps_reader.hpp"
#include "xenith/solver/lp/revised_simplex_solver.hpp"
#include "xenith/solver/lp/pdlp_solver.hpp"
#include "xenith/solution/lp_solution_validator.hpp"
#include "tests/test_harness.hpp"

namespace fs = std::filesystem;
using namespace xenith;

namespace {

std::string resolveFixturePath(const std::string& filename) {
    std::vector<std::string> candidates = {
        "tests/data/mps/" + filename,
        "../tests/data/mps/" + filename,
        "../../tests/data/mps/" + filename,
        "../../../tests/data/mps/" + filename
    };
    for (const auto& candidate : candidates) {
        if (fs::exists(candidate)) {
            return candidate;
        }
    }
    return "tests/data/mps/" + filename;
}

struct NetlibBenchmarkEntry {
    std::string name;
    Index num_vars{0};
    Index num_rows{0};
    Index num_nonzeros{0};
    double expected_obj{0.0};
    double simplex_time_ms{0.0};
    Index simplex_iters{0};
    double simplex_obj{0.0};
    bool simplex_optimal{false};
    double pdlp_time_ms{0.0};
    Index pdlp_iters{0};
    double pdlp_obj{0.0};
    bool pdlp_optimal{false};
};

std::vector<NetlibBenchmarkEntry> g_benchmark_results;

double computeShiftedGeometricMean(const std::vector<double>& times, double shift = 10.0) {
    if (times.empty()) return 0.0;
    double log_sum = 0.0;
    for (double t : times) {
        log_sum += std::log(std::max(0.0, t) + shift);
    }
    return std::exp(log_sum / static_cast<double>(times.size())) - shift;
}

} // namespace

XENITH_TEST(TestNetlibAfiroCompare) {
    std::string path = resolveFixturePath("afiro.mps");
    auto model = io::mps::MpsReader::readFromFile(path);

    NetlibBenchmarkEntry entry;
    entry.name = "AFIRO";
    entry.num_vars = model.numVariables();
    entry.num_rows = model.numConstraints();
    entry.num_nonzeros = model.matrixA().nonZeros();
    entry.expected_obj = -464.753142857;

    // 1. Solve with Revised Simplex
    solver::RevisedSimplexSolver simplex;
    auto t_s0 = std::chrono::high_resolution_clock::now();
    auto s_res = simplex.solve(model);
    auto t_s1 = std::chrono::high_resolution_clock::now();
    entry.simplex_time_ms = std::chrono::duration<double, std::milli>(t_s1 - t_s0).count();
    entry.simplex_iters = s_res.iterations;
    entry.simplex_obj = s_res.objective_value;
    entry.simplex_optimal = (s_res.status == ModelStatus::OPTIMAL);

    XENITH_CHECK(entry.simplex_optimal);
    XENITH_CHECK_NEAR(s_res.objective_value, entry.expected_obj, 1e-3);

    auto s_val = solution::LpSolutionValidator::validate(model, s_res);
    XENITH_CHECK(s_val.isValid);

    // 2. Solve with PDLP
    solver::PdlpOptions popts;
    popts.tolerance = 1e-4;
    solver::PdlpSolver pdlp(popts);
    auto t_p0 = std::chrono::high_resolution_clock::now();
    auto p_res = pdlp.solve(model);
    auto t_p1 = std::chrono::high_resolution_clock::now();
    entry.pdlp_time_ms = std::chrono::duration<double, std::milli>(t_p1 - t_p0).count();
    entry.pdlp_iters = p_res.iterations;
    entry.pdlp_obj = p_res.objective_value;
    entry.pdlp_optimal = (p_res.status == ModelStatus::OPTIMAL);

    XENITH_CHECK(entry.pdlp_optimal);
    XENITH_CHECK_NEAR(p_res.objective_value, entry.expected_obj, 1e-2);

    auto p_val = solution::LpSolutionValidator::validate(model, p_res, 1e-3);
    XENITH_CHECK(p_val.isValid);

    g_benchmark_results.push_back(entry);
}

XENITH_TEST(TestNetlibSc50aCompare) {
    std::string path = resolveFixturePath("sc50a.mps");
    auto model = io::mps::MpsReader::readFromFile(path);

    NetlibBenchmarkEntry entry;
    entry.name = "SC50A";
    entry.num_vars = model.numVariables();
    entry.num_rows = model.numConstraints();
    entry.num_nonzeros = model.matrixA().nonZeros();
    entry.expected_obj = -64.575077058;

    // 1. Solve with Revised Simplex
    solver::RevisedSimplexSolver simplex;
    auto t_s0 = std::chrono::high_resolution_clock::now();
    auto s_res = simplex.solve(model);
    auto t_s1 = std::chrono::high_resolution_clock::now();
    entry.simplex_time_ms = std::chrono::duration<double, std::milli>(t_s1 - t_s0).count();
    entry.simplex_iters = s_res.iterations;
    entry.simplex_obj = s_res.objective_value;
    entry.simplex_optimal = (s_res.status == ModelStatus::OPTIMAL);

    XENITH_CHECK(entry.simplex_optimal);
    XENITH_CHECK_NEAR(s_res.objective_value, entry.expected_obj, 1e-3);

    // 2. Solve with PDLP
    solver::PdlpOptions popts;
    popts.tolerance = 1e-4;
    solver::PdlpSolver pdlp(popts);
    auto t_p0 = std::chrono::high_resolution_clock::now();
    auto p_res = pdlp.solve(model);
    auto t_p1 = std::chrono::high_resolution_clock::now();
    entry.pdlp_time_ms = std::chrono::duration<double, std::milli>(t_p1 - t_p0).count();
    entry.pdlp_iters = p_res.iterations;
    entry.pdlp_obj = p_res.objective_value;
    entry.pdlp_optimal = (p_res.status == ModelStatus::OPTIMAL);

    XENITH_CHECK(entry.pdlp_optimal);
    XENITH_CHECK_NEAR(p_res.objective_value, entry.expected_obj, 1e-2);

    auto p_val = solution::LpSolutionValidator::validate(model, p_res, 1e-3);
    XENITH_CHECK(p_val.isValid);

    g_benchmark_results.push_back(entry);
}

XENITH_TEST(TestNetlibSc50bCompare) {
    std::string path = resolveFixturePath("sc50b.mps");
    auto model = io::mps::MpsReader::readFromFile(path);

    NetlibBenchmarkEntry entry;
    entry.name = "SC50B";
    entry.num_vars = model.numVariables();
    entry.num_rows = model.numConstraints();
    entry.num_nonzeros = model.matrixA().nonZeros();
    entry.expected_obj = -70.000000000;

    // 1. Solve with Revised Simplex
    solver::RevisedSimplexSolver simplex;
    auto t_s0 = std::chrono::high_resolution_clock::now();
    auto s_res = simplex.solve(model);
    auto t_s1 = std::chrono::high_resolution_clock::now();
    entry.simplex_time_ms = std::chrono::duration<double, std::milli>(t_s1 - t_s0).count();
    entry.simplex_iters = s_res.iterations;
    entry.simplex_obj = s_res.objective_value;
    entry.simplex_optimal = (s_res.status == ModelStatus::OPTIMAL);

    XENITH_CHECK(entry.simplex_optimal);
    XENITH_CHECK_NEAR(s_res.objective_value, entry.expected_obj, 1e-3);

    // 2. Solve with PDLP
    solver::PdlpOptions popts;
    popts.tolerance = 1e-4;
    solver::PdlpSolver pdlp(popts);
    auto t_p0 = std::chrono::high_resolution_clock::now();
    auto p_res = pdlp.solve(model);
    auto t_p1 = std::chrono::high_resolution_clock::now();
    entry.pdlp_time_ms = std::chrono::duration<double, std::milli>(t_p1 - t_p0).count();
    entry.pdlp_iters = p_res.iterations;
    entry.pdlp_obj = p_res.objective_value;
    entry.pdlp_optimal = (p_res.status == ModelStatus::OPTIMAL);

    XENITH_CHECK(entry.pdlp_optimal);
    XENITH_CHECK_NEAR(p_res.objective_value, entry.expected_obj, 1e-2);

    auto p_val = solution::LpSolutionValidator::validate(model, p_res, 5e-3);
    XENITH_CHECK(p_val.isValid);

    g_benchmark_results.push_back(entry);
}

XENITH_TEST(TestNetlibBlendCompare) {
    std::string path = resolveFixturePath("blend.mps");
    auto model = io::mps::MpsReader::readFromFile(path);

    NetlibBenchmarkEntry entry;
    entry.name = "BLEND";
    entry.num_vars = model.numVariables();
    entry.num_rows = model.numConstraints();
    entry.num_nonzeros = model.matrixA().nonZeros();
    entry.expected_obj = -30.812149846;

    // 1. Solve with Revised Simplex
    solver::RevisedSimplexSolver simplex;
    auto t_s0 = std::chrono::high_resolution_clock::now();
    auto s_res = simplex.solve(model);
    auto t_s1 = std::chrono::high_resolution_clock::now();
    entry.simplex_time_ms = std::chrono::duration<double, std::milli>(t_s1 - t_s0).count();
    entry.simplex_iters = s_res.iterations;
    entry.simplex_obj = s_res.objective_value;
    entry.simplex_optimal = (s_res.status == ModelStatus::OPTIMAL);

    XENITH_CHECK(entry.simplex_optimal);
    XENITH_CHECK_NEAR(s_res.objective_value, entry.expected_obj, 1e-3);

    auto s_val = solution::LpSolutionValidator::validate(model, s_res);
    XENITH_CHECK(s_val.isValid);

    // 2. Solve with PDLP
    solver::PdlpOptions popts;
    popts.tolerance = 1e-4;
    solver::PdlpSolver pdlp(popts);
    auto t_p0 = std::chrono::high_resolution_clock::now();
    auto p_res = pdlp.solve(model);
    auto t_p1 = std::chrono::high_resolution_clock::now();
    entry.pdlp_time_ms = std::chrono::duration<double, std::milli>(t_p1 - t_p0).count();
    entry.pdlp_iters = p_res.iterations;
    entry.pdlp_obj = p_res.objective_value;
    entry.pdlp_optimal = (p_res.status == ModelStatus::OPTIMAL);

    XENITH_CHECK(entry.pdlp_optimal);
    XENITH_CHECK_NEAR(p_res.objective_value, entry.expected_obj, 1e-2);

    auto p_val = solution::LpSolutionValidator::validate(model, p_res, 5e-3);
    XENITH_CHECK(p_val.isValid);

    g_benchmark_results.push_back(entry);
}

XENITH_TEST(TestNetlibAdlittlePdlp) {
    std::string path = resolveFixturePath("adlittle.mps");
    auto model = io::mps::MpsReader::readFromFile(path);

    NetlibBenchmarkEntry entry;
    entry.name = "ADLITTLE";
    entry.num_vars = model.numVariables();
    entry.num_rows = model.numConstraints();
    entry.num_nonzeros = model.matrixA().nonZeros();
    entry.expected_obj = 225494.96316;

    solver::PdlpOptions popts;
    popts.tolerance = 1e-4;
    popts.max_iterations = 200000;
    solver::PdlpSolver pdlp(popts);
    auto t_p0 = std::chrono::high_resolution_clock::now();
    auto p_res = pdlp.solve(model);
    auto t_p1 = std::chrono::high_resolution_clock::now();
    entry.pdlp_time_ms = std::chrono::duration<double, std::milli>(t_p1 - t_p0).count();
    entry.pdlp_iters = p_res.iterations;
    entry.pdlp_obj = p_res.objective_value;
    entry.pdlp_optimal = (p_res.status == ModelStatus::OPTIMAL);

    XENITH_CHECK(entry.pdlp_optimal);
    double rel_err = std::abs(p_res.objective_value - entry.expected_obj) / (1.0 + std::abs(entry.expected_obj));
    XENITH_CHECK(rel_err < 1e-3);

    g_benchmark_results.push_back(entry);
}

XENITH_TEST(TestNetlibBenchmarkSummary) {
    std::cout << "\n";
    std::cout << "========================================================================================\n";
    std::cout << "              XENITH MATHEMATICAL OPTIMIZATION ENGINE - NETLIB BENCHMARK SUITE          \n";
    std::cout << "========================================================================================\n";
    std::cout << std::left << std::setw(10) << "Model"
              << std::right << std::setw(8) << "Vars"
              << std::setw(8) << "Rows"
              << std::setw(8) << "NNZ"
              << std::setw(16) << "Expected Obj"
              << std::setw(14) << "Simplex(ms)"
              << std::setw(12) << "PDLP(ms)"
              << std::setw(12) << "Speedup"
              << "\n";
    std::cout << "----------------------------------------------------------------------------------------\n";

    std::vector<double> simplex_times;
    std::vector<double> pdlp_times;

    for (const auto& r : g_benchmark_results) {
        std::string speedup_str = "N/A";
        if (r.simplex_optimal && r.pdlp_optimal && r.pdlp_time_ms > 0.0) {
            double speedup = r.simplex_time_ms / r.pdlp_time_ms;
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2) << speedup << "x";
            speedup_str = oss.str();
            simplex_times.push_back(r.simplex_time_ms);
            pdlp_times.push_back(r.pdlp_time_ms);
        }

        std::string simplex_time_str = r.simplex_optimal ? std::to_string(static_cast<int>(r.simplex_time_ms)) : "N/A";
        std::string pdlp_time_str = r.pdlp_optimal ? std::to_string(static_cast<int>(r.pdlp_time_ms)) : "N/A";

        std::cout << std::left << std::setw(10) << r.name
                  << std::right << std::setw(8) << r.num_vars
                  << std::setw(8) << r.num_rows
                  << std::setw(8) << r.num_nonzeros
                  << std::fixed << std::setprecision(4) << std::setw(16) << r.expected_obj
                  << std::setw(14) << simplex_time_str
                  << std::setw(12) << pdlp_time_str
                  << std::setw(12) << speedup_str
                  << "\n";
    }
    std::cout << "----------------------------------------------------------------------------------------\n";

    if (!simplex_times.empty()) {
        double sgm_simplex = computeShiftedGeometricMean(simplex_times, 10.0);
        double sgm_pdlp = computeShiftedGeometricMean(pdlp_times, 10.0);
        double overall_speedup = (sgm_pdlp > 0.0) ? (sgm_simplex / sgm_pdlp) : 1.0;

        std::cout << "Shifted Geometric Mean (shift=10ms):\n";
        std::cout << "  Simplex SGM: " << std::fixed << std::setprecision(2) << sgm_simplex << " ms\n";
        std::cout << "  PDLP SGM:    " << std::fixed << std::setprecision(2) << sgm_pdlp << " ms\n";
        std::cout << "  Aggregate Speedup: " << std::fixed << std::setprecision(2) << overall_speedup << "x\n";
    }
    std::cout << "========================================================================================\n\n";

    XENITH_CHECK(!g_benchmark_results.empty());
}

int main() {
    return xenith::test::TestRunner::instance().runAll();
}

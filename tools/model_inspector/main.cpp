#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>
#include "xenith/io/mps/mps_reader.hpp"
#include "xenith/model/model_validator.hpp"
#include "xenith/solver/lp/revised_simplex_solver.hpp"
#include "xenith/solution/lp_solution_validator.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "XENITH MPS Inspector & LP Solver CLI\n";
        std::cerr << "Usage:\n";
        std::cerr << "  xenith_mps <path-to-mps-file>            (Inspect & validate model)\n";
        std::cerr << "  xenith_mps --solve <path-to-mps-file>    (Solve LP model using Revised Simplex)\n";
        std::cerr << "  xenith_mps --solve --verbose <path-to-mps-file>\n\n";
        return 1;
    }

    bool solve_mode = false;
    bool verbose_mode = false;
    std::string filepath;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            std::cout << "XENITH MPS Inspector & LP Solver CLI\n";
            std::cout << "Usage:\n";
            std::cout << "  xenith_mps <path-to-mps-file>            (Inspect & validate model)\n";
            std::cout << "  xenith_mps --solve <path-to-mps-file>    (Solve LP model using Revised Simplex)\n";
            std::cout << "  xenith_mps --solve --verbose <path-to-mps-file>\n";
            return 0;
        } else if (arg == "--solve") {
            solve_mode = true;
        } else if (arg == "--verbose" || arg == "-v") {
            verbose_mode = true;
        } else {
            filepath = arg;
        }
    }

    if (filepath.empty()) {
        std::cerr << "Error: No input MPS file path provided.\n";
        return 1;
    }

    // Check if file exists / can be opened
    {
        std::ifstream test_file(filepath);
        if (!test_file.is_open()) {
            std::cerr << "XENITH MPS Reader Error:\n";
            std::cerr << "File: " << filepath << "\n";
            std::cerr << "Message: File does not exist or cannot be opened.\n";
            return 1;
        }
    }

    try {
        auto model = xenith::io::mps::MpsReader::readFromFile(filepath);
        auto model_val = xenith::model::ModelValidator::validate(model);

        if (!solve_mode) {
            // Default Inspection Mode
            std::cout << "XENITH MPS Reader\n";
            std::cout << "-----------------\n";
            std::cout << "File: " << filepath << "\n\n";
            std::cout << "Parse status: SUCCESS\n\n";
            std::cout << "Model:\n";
            std::cout << "  Name: " << model.name() << "\n";
            std::cout << "  Variables: " << model.numVariables() << "\n";
            std::cout << "  Constraints: " << model.numConstraints() << "\n";
            std::cout << "  Nonzeros: " << model.matrixA().nonZeros() << "\n";
            std::cout << "  Objective sense: "
                      << (model.sense() == xenith::ObjectiveSense::MINIMIZE ? "MINIMIZE" : "MAXIMIZE")
                      << "\n";

            std::size_t num_integer = 0;
            std::size_t num_binary = 0;
            for (auto t : model.varTypes()) {
                if (t == xenith::VariableType::GENERAL_INTEGER) num_integer++;
                else if (t == xenith::VariableType::BINARY) num_binary++;
            }
            std::cout << "  Integer variables: " << num_integer << "\n";
            std::cout << "  Binary variables: " << num_binary << "\n\n";

            if (model_val.isValid) {
                std::cout << "Validation: PASSED\n";
                return 0;
            } else {
                std::cout << "Validation: FAILED\n";
                std::cout << model_val.summary() << "\n";
                return 1;
            }
        } else {
            // Solve Mode (--solve)
            if (!model_val.isValid) {
                std::cerr << "Model validation failed before solve:\n";
                std::cerr << model_val.summary() << "\n";
                return 1;
            }

            xenith::solver::SimplexOptions opts;
            opts.verbose = verbose_mode;

            xenith::solver::RevisedSimplexSolver solver(opts);
            auto result = solver.solve(model);

            std::cout << "XENITH LP Solver\n";
            std::cout << "----------------\n\n";
            std::cout << "Model: " << model.name() << "\n";
            std::cout << "Variables: " << model.numVariables() << "\n";
            std::cout << "Constraints: " << model.numConstraints() << "\n";
            std::cout << "Nonzeros: " << model.matrixA().nonZeros() << "\n\n";

            std::cout << "Status: " << xenith::toString(result.status) << "\n\n";

            if (result.status == xenith::ModelStatus::OPTIMAL) {
                std::cout << std::fixed << std::setprecision(6);
                std::cout << "Objective: " << result.objective_value << "\n\n";
                std::cout << "Iterations: " << result.iterations << "\n\n";
                std::cout << "Primal feasibility residual: " << result.primal_residual << "\n\n";

                auto sol_val = xenith::solution::LpSolutionValidator::validate(model, result);
                if (sol_val.isValid) {
                    std::cout << "Solution validation: PASSED\n";
                    return 0;
                } else {
                    std::cout << "Solution validation: FAILED\n";
                    std::cout << sol_val.summary() << "\n";
                    return 1;
                }
            } else {
                std::cout << "Iterations: " << result.iterations << "\n";
                std::cout << "Message: " << result.message << "\n";
                return (result.status == xenith::ModelStatus::INFEASIBLE || result.status == xenith::ModelStatus::UNBOUNDED) ? 0 : 1;
            }
        }

    } catch (const xenith::io::mps::MpsParseException& ex) {
        std::cerr << "XENITH MPS Reader Error:\n";
        std::cerr << "Parse status: FAILED\n";
        std::cerr << "File: " << ex.error().filename << "\n";
        if (ex.error().line_number > 0) {
            std::cerr << "Line: " << ex.error().line_number << "\n";
        }
        if (!ex.error().section_name.empty()) {
            std::cerr << "Section: " << ex.error().section_name << "\n";
        }
        std::cerr << "Message: " << ex.error().message << "\n";
        return 1;
    } catch (const std::exception& ex) {
        std::cerr << "XENITH Error: " << ex.what() << "\n";
        return 1;
    }
}

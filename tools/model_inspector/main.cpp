#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <chrono>
#include "xenith/io/mps/mps_reader.hpp"
#include "xenith/model/model_validator.hpp"
#include "xenith/solver/lp/revised_simplex_solver.hpp"
#include "xenith/solver/lp/pdlp_solver.hpp"
#include "xenith/solution/lp_solution_validator.hpp"
#include "xenith/ui/terminal_ui.hpp"

using namespace xenith::ui;

void printUsage() {
    TerminalUI::printLogo();
    std::cout << TerminalUI::colorize("Usage:", ColorScheme::NEON_CYAN, true) << "\n";
    std::cout << "  xenith_mps <path-to-mps-file>                  " 
              << TerminalUI::colorize("(Inspect & validate model)", ColorScheme::GRAY) << "\n";
    std::cout << "  xenith_mps --solve <path-to-mps-file>          " 
              << TerminalUI::colorize("(Solve LP using primary PDLP engine)", ColorScheme::GRAY) << "\n";
    std::cout << "  xenith_mps --solve --method simplex <path>     " 
              << TerminalUI::colorize("(Solve LP using Revised Simplex engine)", ColorScheme::GRAY) << "\n";
    std::cout << "  xenith_mps --solve --verbose <path>            " 
              << TerminalUI::colorize("(Solve with live progress trace)", ColorScheme::GRAY) << "\n\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    bool solve_mode = false;
    bool verbose_mode = false;
    std::string method = "pdlp";
    std::string filepath;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            printUsage();
            return 0;
        } else if (arg == "--solve") {
            solve_mode = true;
        } else if (arg == "--verbose" || arg == "-v") {
            verbose_mode = true;
        } else if (arg == "--method" && i + 1 < argc) {
            method = argv[++i];
        } else if (arg.rfind("--method=", 0) == 0) {
            method = arg.substr(9);
        } else {
            filepath = arg;
        }
    }

    if (filepath.empty()) {
        TerminalUI::printLogo();
        TerminalUI::printErrorBox("CLI ARGUMENT ERROR", {"No input MPS file path provided."});
        return 1;
    }

    TerminalUI::printLogo();

    // Check if file exists
    {
        std::ifstream test_file(filepath);
        if (!test_file.is_open()) {
            TerminalUI::printErrorBox("FILE NOT FOUND", {
                "File Path: " + filepath,
                "Message:   File does not exist or cannot be opened."
            });
            return 1;
        }
    }

    try {
        int total_steps = solve_mode ? 3 : 2;

        // Step 1: Parse MPS File
        TerminalUI::printStepHeader(1, total_steps, "Ingesting MPS file: " + filepath);
        auto t_start = std::chrono::high_resolution_clock::now();
        
        auto model = xenith::io::mps::MpsReader::readFromFile(filepath);
        
        auto t_parsed = std::chrono::high_resolution_clock::now();
        double parse_ms = std::chrono::duration<double, std::milli>(t_parsed - t_start).count();
        TerminalUI::printStepSuccess(1, total_steps, "Parsed MPS file (" + model.name() + ")", parse_ms);

        // Step 2: Model Invariants Validation
        TerminalUI::printStepHeader(2, total_steps, "Validating canonical model invariants");
        auto t_val_start = std::chrono::high_resolution_clock::now();
        
        auto model_val = xenith::model::ModelValidator::validate(model);
        
        auto t_val_end = std::chrono::high_resolution_clock::now();
        double val_ms = std::chrono::duration<double, std::milli>(t_val_end - t_val_start).count();

        if (!model_val.isValid) {
            TerminalUI::printStepError(2, total_steps, "Model validation", model_val.summary());
            return 1;
        }
        TerminalUI::printStepSuccess(2, total_steps, "Model structural validation", val_ms);

        std::size_t num_integer = 0;
        std::size_t num_binary = 0;
        for (auto t : model.varTypes()) {
            if (t == xenith::VariableType::GENERAL_INTEGER) num_integer++;
            else if (t == xenith::VariableType::BINARY) num_binary++;
        }

        double density = 0.0;
        if (model.numVariables() > 0 && model.numConstraints() > 0) {
            density = 100.0 * static_cast<double>(model.matrixA().nonZeros()) / 
                      static_cast<double>(model.numVariables() * model.numConstraints());
        }

        if (!solve_mode) {
            // Inspection Dashboard
            std::stringstream ss_density;
            ss_density << std::fixed << std::setprecision(2) << density << "%";

            std::vector<TerminalUI::KeyValue> items = {
                {"File Path", filepath, ColorScheme::WHITE},
                {"Model Name", model.name(), ColorScheme::GOLD, true},
                {"Variables", std::to_string(model.numVariables()) + 
                              " (Integer: " + std::to_string(num_integer) + 
                              ", Binary: " + std::to_string(num_binary) + ")", ColorScheme::NEON_CYAN},
                {"Constraints", std::to_string(model.numConstraints()), ColorScheme::NEON_CYAN},
                {"Nonzeros", std::to_string(model.matrixA().nonZeros()) + 
                             " (Density: " + ss_density.str() + ")", ColorScheme::NEON_CYAN},
                {"Objective Sense", (model.sense() == xenith::ObjectiveSense::MINIMIZE ? "MINIMIZE" : "MAXIMIZE"), ColorScheme::PURPLE, true},
                {"Parse Time", std::to_string(parse_ms) + " ms", ColorScheme::GRAY},
                {"Invariants Check", "PASSED ✔", ColorScheme::NEON_GREEN, true}
            };

            std::cout << "\n";
            TerminalUI::printBox("✦ MODEL INSPECTION DASHBOARD", items, ColorScheme::NEON_CYAN);
            return 0;
        } else {
            // Solve Mode (--solve)
            if (method == "simplex") {
                TerminalUI::printStepHeader(3, total_steps, "Executing Revised Simplex Engine");
                
                xenith::solver::SimplexOptions opts;
                opts.verbose = verbose_mode;

                auto t_solve_start = std::chrono::high_resolution_clock::now();
                xenith::solver::RevisedSimplexSolver solver(opts);
                auto result = solver.solve(model);
                auto t_solve_end = std::chrono::high_resolution_clock::now();
                
                double solve_ms = std::chrono::duration<double, std::milli>(t_solve_end - t_solve_start).count();

                if (result.status == xenith::ModelStatus::OPTIMAL) {
                    TerminalUI::printStepSuccess(3, total_steps, "Revised Simplex solve complete", solve_ms);

                    std::stringstream ss_obj, ss_res;
                    ss_obj << std::fixed << std::setprecision(6) << result.objective_value;
                    ss_res << std::scientific << std::setprecision(6) << result.primal_residual;

                    auto sol_val = xenith::solution::LpSolutionValidator::validate(model, result);

                    std::vector<TerminalUI::KeyValue> items = {
                        {"Solver Engine", "Revised Simplex (Phase I/II Exact Vertex)", ColorScheme::PURPLE, true},
                        {"Model Name", model.name(), ColorScheme::WHITE},
                        {"Problem Size", std::to_string(model.numVariables()) + " vars, " + 
                                         std::to_string(model.numConstraints()) + " rows, " + 
                                         std::to_string(model.matrixA().nonZeros()) + " nonzeros", ColorScheme::NEON_CYAN},
                        {"Model Status", xenith::toString(result.status) + " ✔", ColorScheme::NEON_GREEN, true},
                        {"Optimal Objective", ss_obj.str(), ColorScheme::GOLD, true},
                        {"Simplex Iterations", std::to_string(result.iterations), ColorScheme::NEON_CYAN},
                        {"Primal Residual", ss_res.str(), ColorScheme::WHITE},
                        {"Solve Time", std::to_string(solve_ms) + " ms", ColorScheme::GRAY},
                        {"Solution Validation", (sol_val.isValid ? "PASSED ✔" : "FAILED ✖"), 
                                                (sol_val.isValid ? ColorScheme::NEON_GREEN : ColorScheme::RED), true}
                    };

                    std::cout << "\n";
                    TerminalUI::printBox("✦ SIMPLEX SOLVER OPTIMAL RESULT DASHBOARD", items, ColorScheme::NEON_GREEN);
                    return sol_val.isValid ? 0 : 1;
                } else {
                    TerminalUI::printStepError(3, total_steps, "Revised Simplex solve", result.message);

                    std::vector<TerminalUI::KeyValue> items = {
                        {"Solver Engine", "Revised Simplex", ColorScheme::PURPLE},
                        {"Model Name", model.name(), ColorScheme::WHITE},
                        {"Model Status", xenith::toString(result.status), ColorScheme::RED, true},
                        {"Iterations", std::to_string(result.iterations), ColorScheme::NEON_CYAN},
                        {"Message", result.message, ColorScheme::WHITE}
                    };

                    std::cout << "\n";
                    TerminalUI::printBox("✦ SIMPLEX TERMINATION DASHBOARD", items, ColorScheme::RED);
                    return (result.status == xenith::ModelStatus::INFEASIBLE || result.status == xenith::ModelStatus::UNBOUNDED) ? 0 : 1;
                }
            } else {
                // Default: PDLP Engine (Restarted PDHG)
                TerminalUI::printStepHeader(3, total_steps, "Executing Matrix-Free PDLP Engine (Restarted PDHG)");

                xenith::solver::PdlpOptions opts;
                opts.verbose = verbose_mode;

                auto t_solve_start = std::chrono::high_resolution_clock::now();
                xenith::solver::PdlpSolver solver(opts);
                auto result = solver.solve(model);
                auto t_solve_end = std::chrono::high_resolution_clock::now();

                double solve_ms = std::chrono::duration<double, std::milli>(t_solve_end - t_solve_start).count();

                if (result.status == xenith::ModelStatus::OPTIMAL) {
                    TerminalUI::printStepSuccess(3, total_steps, "PDLP solve complete", solve_ms);

                    std::stringstream ss_obj, ss_pres, ss_dres, ss_gap;
                    ss_obj << std::fixed << std::setprecision(6) << result.objective_value;
                    ss_pres << std::scientific << std::setprecision(2) << result.primal_residual;
                    ss_dres << std::scientific << std::setprecision(2) << result.dual_residual;
                    ss_gap << std::scientific << std::setprecision(2) << result.duality_gap;

                    auto sol_val = xenith::solution::LpSolutionValidator::validate(model, result, 1e-3);

                    std::vector<TerminalUI::KeyValue> items = {
                        {"Solver Engine", "PDLP (Restarted PDHG GPU-Ready)", ColorScheme::NEON_CYAN, true},
                        {"Model Name", model.name(), ColorScheme::WHITE},
                        {"Problem Size", std::to_string(model.numVariables()) + " vars, " + 
                                         std::to_string(model.numConstraints()) + " rows, " + 
                                         std::to_string(model.matrixA().nonZeros()) + " nonzeros", ColorScheme::NEON_CYAN},
                        {"Model Status", xenith::toString(result.status) + " ✔", ColorScheme::NEON_GREEN, true},
                        {"Optimal Objective", ss_obj.str(), ColorScheme::GOLD, true},
                        {"PDHG Iterations", std::to_string(result.iterations) + " (" + std::to_string(result.restarts) + " restarts)", ColorScheme::NEON_CYAN},
                        {"Primal Residual", ss_pres.str(), ColorScheme::WHITE},
                        {"Dual Residual", ss_dres.str(), ColorScheme::WHITE},
                        {"Duality Gap", ss_gap.str(), ColorScheme::WHITE},
                        {"Solve Time", std::to_string(solve_ms) + " ms", ColorScheme::GRAY},
                        {"Solution Validation", (sol_val.isValid ? "PASSED ✔" : "FAILED ✖"), 
                                                (sol_val.isValid ? ColorScheme::NEON_GREEN : ColorScheme::RED), true}
                    };

                    std::cout << "\n";
                    TerminalUI::printBox("✦ PDLP SOLVER OPTIMAL RESULT DASHBOARD", items, ColorScheme::NEON_GREEN);
                    return sol_val.isValid ? 0 : 1;
                } else {
                    TerminalUI::printStepError(3, total_steps, "PDLP solve", result.message);

                    std::vector<TerminalUI::KeyValue> items = {
                        {"Solver Engine", "PDLP (Restarted PDHG)", ColorScheme::NEON_CYAN},
                        {"Model Name", model.name(), ColorScheme::WHITE},
                        {"Model Status", xenith::toString(result.status), ColorScheme::RED, true},
                        {"Iterations", std::to_string(result.iterations), ColorScheme::NEON_CYAN},
                        {"Message", result.message, ColorScheme::WHITE}
                    };

                    std::cout << "\n";
                    TerminalUI::printBox("✦ PDLP TERMINATION DASHBOARD", items, ColorScheme::RED);
                    return (result.status == xenith::ModelStatus::INFEASIBLE || result.status == xenith::ModelStatus::UNBOUNDED) ? 0 : 1;
                }
            }
        }
    } catch (const xenith::io::mps::MpsParseException& ex) {
        TerminalUI::printStepError(1, solve_mode ? 3 : 2, "Parsing MPS file", ex.error().message);
        
        std::vector<std::string> details = {
            "File:    " + ex.error().filename,
            "Message: " + ex.error().message
        };
        if (ex.error().line_number > 0) {
            details.push_back("Line:    " + std::to_string(ex.error().line_number));
        }
        if (!ex.error().section_name.empty()) {
            details.push_back("Section: " + ex.error().section_name);
        }

        TerminalUI::printErrorBox("MPS PARSER EXCEPTION", details);
        return 1;
    } catch (const std::exception& ex) {
        TerminalUI::printErrorBox("SYSTEM EXCEPTION", {ex.what()});
        return 1;
    }
}

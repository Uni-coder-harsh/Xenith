#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "xenith/io/mps/mps_reader.hpp"
#include "xenith/model/model_validator.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "XENITH MPS Inspector CLI\n";
        std::cerr << "Usage: xenith_mps <path-to-mps-file>\n\n";
        std::cerr << "Description:\n";
        std::cerr << "  Parses an MPS optimization model file, constructs a CanonicalModel,\n";
        std::cerr << "  validates structural/numerical invariants, and prints model summary statistics.\n";
        return 1;
    }

    std::string arg1 = argv[1];
    if (arg1 == "-h" || arg1 == "--help") {
        std::cout << "XENITH MPS Inspector CLI\n";
        std::cout << "Usage: xenith_mps <path-to-mps-file>\n\n";
        std::cout << "Description:\n";
        std::cout << "  Parses an MPS optimization model file, constructs a CanonicalModel,\n";
        std::cout << "  validates structural/numerical invariants, and prints model summary statistics.\n";
        return 0;
    }

    std::string filepath = arg1;

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
        auto validation = xenith::model::ModelValidator::validate(model);

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
            if (t == xenith::VariableType::GENERAL_INTEGER) {
                num_integer++;
            } else if (t == xenith::VariableType::BINARY) {
                num_binary++;
            }
        }
        std::cout << "  Integer variables: " << num_integer << "\n";
        std::cout << "  Binary variables: " << num_binary << "\n\n";

        if (validation.isValid) {
            std::cout << "Validation: PASSED\n";
            return 0;
        } else {
            std::cout << "Validation: FAILED\n";
            std::cout << validation.summary() << "\n";
            return 1;
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

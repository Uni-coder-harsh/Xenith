#include "xenith/io/mps/mps_reader.hpp"
#include "xenith/model/model_validator.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <unordered_map>
#include <unordered_set>

namespace xenith::io::mps {

namespace {

// Trim leading and trailing whitespace and control characters
std::string trim(const std::string& str) {
    auto first = str.find_first_not_of(" \t\r\n\v\f");
    if (first == std::string::npos) return "";
    auto last = str.find_last_not_of(" \t\r\n\v\f");
    return str.substr(first, (last - first + 1));
}

// Convert string to uppercase
std::string toUpper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    return str;
}

// Tokenize a line into non-empty whitespace-separated strings
std::vector<std::string> tokenizeFreeFormat(const std::string& line) {
    std::vector<std::string> tokens;
    std::istringstream iss(line);
    std::string token;
    while (iss >> token) {
        std::string cleaned = trim(token);
        if (!cleaned.empty()) {
            tokens.push_back(cleaned);
        }
    }
    return tokens;
}

// Parse fixed-format fields from standard MPS line
std::vector<std::string> tokenizeFixedFormat(const std::string& line) {
    std::vector<std::string> tokens;

    auto getSubstring = [&](std::size_t start, std::size_t len) -> std::string {
        if (start >= line.size()) return "";
        return trim(line.substr(start, len));
    };

    std::string f1 = getSubstring(1, 2);
    std::string f2 = getSubstring(4, 8);
    std::string f3 = getSubstring(14, 8);
    std::string f4 = getSubstring(24, 12);
    std::string f5 = getSubstring(38, 8);
    std::string f6 = getSubstring(48, 12);

    if (!f1.empty()) tokens.push_back(f1);
    if (!f2.empty()) tokens.push_back(f2);
    if (!f3.empty()) tokens.push_back(f3);
    if (!f4.empty()) tokens.push_back(f4);
    if (!f5.empty()) tokens.push_back(f5);
    if (!f6.empty()) tokens.push_back(f6);

    if (tokens.empty()) {
        return tokenizeFreeFormat(line);
    }
    return tokens;
}

struct RowDef {
    char type{'N'};
    std::string name;
    Index index{0};
};

struct MpsParseState {
    std::string problem_name{"xenith_mps_model"};
    ObjectiveSense sense{ObjectiveSense::MINIMIZE};
    std::string selected_obj_row;

    std::vector<RowDef> row_defs;
    std::unordered_map<std::string, Index> row_name_to_index;

    std::vector<std::string> var_names;
    std::unordered_map<std::string, Index> var_name_to_index;
    std::vector<VariableType> var_types;
    std::vector<double> obj_coeffs;

    std::vector<numerics::Triplet> matrix_triplets;

    std::unordered_map<std::string, double> rhs_values;
    std::unordered_map<std::string, double> range_values;

    std::unordered_map<std::string, double> var_lower_bounds;
    std::unordered_map<std::string, double> var_upper_bounds;
    std::unordered_set<std::string> var_has_bounds_card;

    bool in_integer_block{false};
    bool has_rows_section{false};
    bool has_columns_section{false};
    bool has_endata_section{false};

    Index getOrAddVariable(const std::string& name) {
        auto it = var_name_to_index.find(name);
        if (it != var_name_to_index.end()) {
            return it->second;
        }
        Index idx = var_names.size();
        var_names.push_back(name);
        var_name_to_index[name] = idx;
        obj_coeffs.push_back(0.0);
        var_types.push_back(in_integer_block ? VariableType::GENERAL_INTEGER : VariableType::CONTINUOUS);
        return idx;
    }
};

} // namespace

model::CanonicalModel MpsReader::readFromFile(const std::string& filepath,
                                            const MpsReaderOptions& options) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        MpsParseError err;
        err.filename = filepath;
        err.line_number = 0;
        err.section_name = "NONE";
        err.message = "Failed to open MPS file for reading: " + filepath;
        throw MpsParseException(err);
    }
    return readFromStream(file, filepath, options);
}

model::CanonicalModel MpsReader::readFromString(const std::string& content,
                                               const std::string& source_name,
                                               const MpsReaderOptions& options) {
    std::istringstream stream(content);
    return readFromStream(stream, source_name, options);
}

model::CanonicalModel MpsReader::readFromStream(std::istream& stream,
                                               const std::string& stream_name,
                                               const MpsReaderOptions& options) {
    MpsParseState state;
    state.sense = options.default_sense;
    state.selected_obj_row = options.objective_name;

    MpsSection current_section = MpsSection::NONE;
    std::string line;
    std::size_t line_number = 0;

    auto makeError = [&](const std::string& msg) -> MpsParseException {
        MpsParseError err;
        err.filename = stream_name;
        err.line_number = line_number;
        err.section_name = toString(current_section);
        err.message = msg;
        return MpsParseException(err);
    };

    while (std::getline(stream, line)) {
        line_number++;
        std::string trimmed_line = trim(line);

        // Skip blank lines and comment lines starting with * or $
        if (trimmed_line.empty() || trimmed_line[0] == '*' || trimmed_line[0] == '$') {
            continue;
        }

        std::vector<std::string> tokens = options.allow_free_format ?
            tokenizeFreeFormat(trimmed_line) : tokenizeFixedFormat(line);

        if (tokens.empty()) continue;

        std::string first_token_upper = toUpper(tokens[0]);

        // Section header recognition
        if (first_token_upper == "NAME") {
            current_section = MpsSection::NAME;
            if (tokens.size() >= 2) {
                state.problem_name = tokens[1];
            }
            continue;
        } else if (first_token_upper == "OBJSENSE") {
            current_section = MpsSection::OBJSENSE;
            if (tokens.size() >= 2) {
                std::string sense_tok = toUpper(tokens[1]);
                if (sense_tok == "MAX" || sense_tok == "MAXIMIZE") {
                    state.sense = ObjectiveSense::MAXIMIZE;
                } else if (sense_tok == "MIN" || sense_tok == "MINIMIZE") {
                    state.sense = ObjectiveSense::MINIMIZE;
                }
            }
            continue;
        } else if (first_token_upper == "ROWS") {
            current_section = MpsSection::ROWS;
            state.has_rows_section = true;
            continue;
        } else if (first_token_upper == "COLUMNS") {
            current_section = MpsSection::COLUMNS;
            state.has_columns_section = true;
            continue;
        } else if (first_token_upper == "RHS") {
            current_section = MpsSection::RHS;
            continue;
        } else if (first_token_upper == "RANGES") {
            current_section = MpsSection::RANGES;
            continue;
        } else if (first_token_upper == "BOUNDS") {
            current_section = MpsSection::BOUNDS;
            continue;
        } else if (first_token_upper == "ENDATA") {
            current_section = MpsSection::ENDATA;
            state.has_endata_section = true;
            break;
        }

        // Section Content Parsing
        switch (current_section) {
            case MpsSection::OBJSENSE: {
                std::string sense_tok = toUpper(tokens[0]);
                if (sense_tok == "MAX" || sense_tok == "MAXIMIZE") {
                    state.sense = ObjectiveSense::MAXIMIZE;
                } else if (sense_tok == "MIN" || sense_tok == "MINIMIZE") {
                    state.sense = ObjectiveSense::MINIMIZE;
                }
                break;
            }

            case MpsSection::ROWS: {
                if (tokens.size() < 2) {
                    throw makeError("Malformed ROWS line, expected: <type> <row_name>");
                }
                char rtype = static_cast<char>(std::toupper(static_cast<unsigned char>(tokens[0][0])));
                std::string rname = tokens[1];

                if (rtype != 'N' && rtype != 'L' && rtype != 'G' && rtype != 'E') {
                    throw makeError("Unknown row type '" + std::string(1, rtype) + "' for row '" + rname + "'");
                }

                if (rtype == 'N') {
                    if (state.selected_obj_row.empty()) {
                        state.selected_obj_row = rname;
                    }
                } else {
                    if (state.row_name_to_index.find(rname) != state.row_name_to_index.end()) {
                        throw makeError("Duplicate constraint row name in ROWS section: " + rname);
                    }
                    Index row_idx = state.row_defs.size();
                    state.row_defs.push_back({rtype, rname, row_idx});
                    state.row_name_to_index[rname] = row_idx;
                }
                break;
            }

            case MpsSection::COLUMNS: {
                if (tokens.size() < 3) {
                    throw makeError("Malformed COLUMNS line, expected: <var_name> <row_name> <value>");
                }
                std::string var_name = tokens[0];
                std::string row_name1 = tokens[1];
                std::string val_str1 = tokens[2];

                // Check for MPS Integer Marker Cards ('MARK0000', 'INTORG', 'INTEND')
                std::string val_str1_upper = toUpper(val_str1);
                std::string row1_upper = toUpper(row_name1);

                if (row1_upper.find("MARK") != std::string::npos || val_str1_upper.find("MARK") != std::string::npos) {
                    std::string combined = toUpper(line);
                    if (combined.find("INTORG") != std::string::npos) {
                        state.in_integer_block = true;
                    } else if (combined.find("INTEND") != std::string::npos) {
                        state.in_integer_block = false;
                    }
                    continue; // Skip creating a variable/triplet for marker lines
                }

                Index var_idx = state.getOrAddVariable(var_name);

                auto processPair = [&](const std::string& rname, const std::string& val_str) {
                    double val = 0.0;
                    try {
                        val = std::stod(val_str);
                    } catch (...) {
                        throw makeError("Invalid numeric coefficient value '" + val_str + "' for variable '" + var_name + "'");
                    }

                    if (rname == state.selected_obj_row) {
                        state.obj_coeffs[var_idx] += val;
                    } else {
                        auto it = state.row_name_to_index.find(rname);
                        if (it != state.row_name_to_index.end()) {
                            state.matrix_triplets.push_back({it->second, var_idx, val});
                        }
                    }
                };

                processPair(row_name1, val_str1);

                if (tokens.size() >= 5) {
                    std::string row_name2 = tokens[3];
                    std::string val_str2 = tokens[4];
                    processPair(row_name2, val_str2);
                }
                break;
            }

            case MpsSection::RHS: {
                if (tokens.size() < 3) {
                    throw makeError("Malformed RHS line, expected: <rhs_set> <row_name> <value>");
                }
                auto processRhsPair = [&](const std::string& rname, const std::string& val_str) {
                    double val = 0.0;
                    try {
                        val = std::stod(val_str);
                    } catch (...) {
                        throw makeError("Invalid numeric RHS value '" + val_str + "' for row '" + rname + "'");
                    }
                    state.rhs_values[rname] = val;
                };

                processRhsPair(tokens[1], tokens[2]);

                if (tokens.size() >= 5) {
                    processRhsPair(tokens[3], tokens[4]);
                }
                break;
            }

            case MpsSection::RANGES: {
                if (tokens.size() < 3) {
                    throw makeError("Malformed RANGES line, expected: <range_set> <row_name> <value>");
                }
                auto processRangePair = [&](const std::string& rname, const std::string& val_str) {
                    double val = 0.0;
                    try {
                        val = std::stod(val_str);
                    } catch (...) {
                        throw makeError("Invalid numeric RANGES value '" + val_str + "' for row '" + rname + "'");
                    }
                    state.range_values[rname] = val;
                };

                processRangePair(tokens[1], tokens[2]);

                if (tokens.size() >= 5) {
                    processRangePair(tokens[3], tokens[4]);
                }
                break;
            }

            case MpsSection::BOUNDS: {
                if (tokens.size() < 3) {
                    throw makeError("Malformed BOUNDS line, expected: <type> <bound_set> <var_name> [val]");
                }
                std::string btype = toUpper(tokens[0]);
                std::string var_name = tokens[2];

                state.var_has_bounds_card.insert(var_name);
                Index var_idx = state.getOrAddVariable(var_name);

                double val = 0.0;
                if (btype != "FR" && btype != "MI" && btype != "PL" && btype != "BV") {
                    if (tokens.size() < 4) {
                        throw makeError("Bound type '" + btype + "' requires value parameter for variable '" + var_name + "'");
                    }
                    try {
                        val = std::stod(tokens[3]);
                    } catch (...) {
                        throw makeError("Invalid numerical bound value '" + tokens[3] + "' for variable '" + var_name + "'");
                    }
                }

                if (btype == "LO") {
                    state.var_lower_bounds[var_name] = val;
                } else if (btype == "UP") {
                    state.var_upper_bounds[var_name] = val;
                } else if (btype == "FX") {
                    state.var_lower_bounds[var_name] = val;
                    state.var_upper_bounds[var_name] = val;
                } else if (btype == "FR") {
                    state.var_lower_bounds[var_name] = -K_INFINITY;
                    state.var_upper_bounds[var_name] = K_INFINITY;
                } else if (btype == "MI") {
                    state.var_lower_bounds[var_name] = -K_INFINITY;
                } else if (btype == "PL") {
                    state.var_upper_bounds[var_name] = K_INFINITY;
                } else if (btype == "BV") {
                    state.var_lower_bounds[var_name] = 0.0;
                    state.var_upper_bounds[var_name] = 1.0;
                    state.var_types[var_idx] = VariableType::BINARY;
                } else if (btype == "LI") {
                    state.var_lower_bounds[var_name] = val;
                    state.var_types[var_idx] = VariableType::GENERAL_INTEGER;
                } else if (btype == "UI") {
                    state.var_upper_bounds[var_name] = val;
                    state.var_types[var_idx] = VariableType::GENERAL_INTEGER;
                } else {
                    throw makeError("Unsupported MPS bound type '" + btype + "' for variable '" + var_name + "'");
                }
                break;
            }

            default:
                break;
        }
    }

    // Section Validation
    if (!state.has_rows_section) {
        MpsParseError err;
        err.filename = stream_name;
        err.line_number = line_number;
        err.section_name = toString(current_section);
        err.message = "Missing required ROWS section in MPS input";
        throw MpsParseException(err);
    }

    if (!state.has_columns_section) {
        MpsParseError err;
        err.filename = stream_name;
        err.line_number = line_number;
        err.section_name = toString(current_section);
        err.message = "Missing required COLUMNS section in MPS input";
        throw MpsParseException(err);
    }

    // Construct CanonicalModel
    model::CanonicalModel model(state.problem_name);
    model.setSense(state.sense);

    // 1. Add Variables
    for (std::size_t j = 0; j < state.var_names.size(); ++j) {
        const std::string& vname = state.var_names[j];
        VariableType vtype = state.var_types[j];

        double lb = 0.0;
        double ub = (vtype == VariableType::BINARY) ? 1.0 : K_INFINITY;

        auto it_lb = state.var_lower_bounds.find(vname);
        if (it_lb != state.var_lower_bounds.end()) {
            lb = it_lb->second;
        }

        auto it_ub = state.var_upper_bounds.find(vname);
        if (it_ub != state.var_upper_bounds.end()) {
            ub = it_ub->second;
        }

        double c_j = state.obj_coeffs[j];
        model.addVariable(vname, lb, ub, c_j, vtype);
    }

    // 2. Add Constraint Rows
    for (const auto& rdef : state.row_defs) {
        double rhs = 0.0;
        auto it_rhs = state.rhs_values.find(rdef.name);
        if (it_rhs != state.rhs_values.end()) {
            rhs = it_rhs->second;
        }

        double rng = 0.0;
        auto it_rng = state.range_values.find(rdef.name);
        if (it_rng != state.range_values.end()) {
            rng = it_rng->second;
        }

        double r_lb = -K_INFINITY;
        double r_ub = K_INFINITY;

        if (rdef.type == 'L') {
            if (rng == 0.0) {
                r_lb = -K_INFINITY;
                r_ub = rhs;
            } else if (rng > 0.0) {
                r_lb = rhs - rng;
                r_ub = rhs;
            } else { // rng < 0
                r_lb = rhs;
                r_ub = rhs - rng;
            }
        } else if (rdef.type == 'G') {
            if (rng == 0.0) {
                r_lb = rhs;
                r_ub = K_INFINITY;
            } else if (rng > 0.0) {
                r_lb = rhs;
                r_ub = rhs + rng;
            } else { // rng < 0
                r_lb = rhs + rng;
                r_ub = rhs;
            }
        } else if (rdef.type == 'E') {
            if (rng == 0.0) {
                r_lb = rhs;
                r_ub = rhs;
            } else if (rng > 0.0) {
                r_lb = rhs;
                r_ub = rhs + rng;
            } else { // rng < 0
                r_lb = rhs + rng;
                r_ub = rhs;
            }
        }

        model.addRow(rdef.name, r_lb, r_ub);
    }

    // 3. Build Sparse Matrix A
    Index m = model.numConstraints();
    Index n = model.numVariables();

    numerics::SparseMatrix matA = numerics::SparseMatrix::fromTriplets(
        m, n, state.matrix_triplets, numerics::SparseStorageFormat::CSC);
    model.setMatrixA(std::move(matA));

    // 4. Validate Model Invariants
    model::ValidationResult val_result = model::ModelValidator::validate(model);
    if (!val_result.isValid) {
        MpsParseError err;
        err.filename = stream_name;
        err.line_number = line_number;
        err.section_name = "VALIDATION";
        err.message = "CanonicalModel failed invariant validation:\n" + val_result.summary();
        throw MpsParseException(err);
    }

    return model;
}

} // namespace xenith::io::mps

#ifndef XENITH_MODEL_MODEL_VALIDATOR_HPP
#define XENITH_MODEL_MODEL_VALIDATOR_HPP

#include <string>
#include <vector>
#include <optional>
#include "xenith/common/types.hpp"
#include "xenith/model/canonical_model.hpp"

namespace xenith::model {

/// Severity of a validation diagnostic message.
enum class DiagnosticSeverity {
    ERROR,
    WARNING,
    INFO
};

/// Individual diagnostic item explaining a validation check result.
struct ValidationDiagnostic {
    DiagnosticSeverity severity{DiagnosticSeverity::ERROR};
    std::string code;
    std::string message;
    std::string component; // "VARIABLE", "CONSTRAINT", "MATRIX", "OBJECTIVE"
    std::optional<Index> index{std::nullopt};

    std::string toString() const;
};

/// Aggregated result of validating a CanonicalModel instance.
struct ValidationResult {
    bool isValid{true};
    std::vector<ValidationDiagnostic> diagnostics;

    /// Formats a complete multi-line summary report of all diagnostics.
    std::string summary() const;
    
    /// Returns count of diagnostics with ERROR severity.
    std::size_t errorCount() const;

    /// Returns count of diagnostics with WARNING severity.
    std::size_t warningCount() const;
};

/**
 * @brief Executable Model Validator for verifying CanonicalModel invariants.
 */
class ModelValidator {
public:
    /**
     * @brief Performs complete validation of dimensional, bound, structural, and numerical invariants.
     * @param model CanonicalModel instance to validate.
     * @return ValidationResult containing boolean validity flag and detailed diagnostics.
     */
    static ValidationResult validate(const CanonicalModel& model);
};

} // namespace xenith::model

#endif // XENITH_MODEL_MODEL_VALIDATOR_HPP

#ifndef XENITH_MODEL_CANONICAL_MODEL_HPP
#define XENITH_MODEL_CANONICAL_MODEL_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <memory>
#include "xenith/common/types.hpp"
#include "xenith/common/constants.hpp"
#include "xenith/numerics/sparse_matrix.hpp"

namespace xenith::model {

/**
 * @brief Central Mathematical Intermediate Representation for XENITH.
 * Represents: min/max c^T x + 0.5 x^T Q x s.t. l_r <= A x <= u_r, l_x <= x <= u_x.
 */
class CanonicalModel {
public:
    explicit CanonicalModel(std::string name = "xenith_model");

    // Problem Name and Optimization Sense
    const std::string& name() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }

    ObjectiveSense sense() const { return m_sense; }
    void setSense(ObjectiveSense sense) { m_sense = sense; }

    double objectiveOffset() const { return m_objectiveOffset; }
    void setObjectiveOffset(double offset) { m_objectiveOffset = offset; }

    // Dimensionality Accessors
    Index numVariables() const { return m_colLower.size(); }
    Index numConstraints() const { return m_rowLower.size(); }

    // Programmatic Variable Construction
    Index addVariable(const std::string& name,
                      double lower_bound = 0.0,
                      double upper_bound = K_INFINITY,
                      double obj_coeff = 0.0,
                      VariableType type = VariableType::CONTINUOUS);

    // Programmatic Constraint Construction via Bounded-Row Representation
    Index addRow(const std::string& name, double lower_bound, double upper_bound);
    Index addLessOrEqualRow(const std::string& name, double rhs);
    Index addGreaterOrEqualRow(const std::string& name, double rhs);
    Index addEqualityRow(const std::string& name, double rhs);
    Index addRangeRow(const std::string& name, double lower_bound, double upper_bound);

    // Matrix Assignment
    void setMatrixA(numerics::SparseMatrix matrix);
    void setMatrixQ(numerics::SparseMatrix matrix);

    // Direct Accessors to Data Vectors
    std::span<const double> objective() const { return m_objective; }
    std::span<double> objective() { return m_objective; }

    std::span<const double> colLower() const { return m_colLower; }
    std::span<double> colLower() { return m_colLower; }

    std::span<const double> colUpper() const { return m_colUpper; }
    std::span<double> colUpper() { return m_colUpper; }

    std::span<const VariableType> varTypes() const { return m_varTypes; }
    std::span<const std::string> varNames() const { return m_varNames; }

    std::span<const double> rowLower() const { return m_rowLower; }
    std::span<double> rowLower() { return m_rowLower; }

    std::span<const double> rowUpper() const { return m_rowUpper; }
    std::span<double> rowUpper() { return m_rowUpper; }

    std::span<const std::string> rowNames() const { return m_rowNames; }

    const numerics::SparseMatrix& matrixA() const { return m_matrixA; }
    const std::optional<numerics::SparseMatrix>& matrixQ() const { return m_matrixQ; }

    // Name-to-Index Lookups
    std::optional<Index> getVariableIndex(const std::string& name) const;
    std::optional<Index> getRowIndex(const std::string& name) const;

private:
    std::string m_name;
    ObjectiveSense m_sense{ObjectiveSense::MINIMIZE};
    double m_objectiveOffset{0.0};

    // Variable data (length n)
    std::vector<double> m_objective;
    std::vector<double> m_colLower;
    std::vector<double> m_colUpper;
    std::vector<VariableType> m_varTypes;
    std::vector<std::string> m_varNames;
    std::unordered_map<std::string, Index> m_varNameToIndex;

    // Constraint data (length m)
    std::vector<double> m_rowLower;
    std::vector<double> m_rowUpper;
    std::vector<std::string> m_rowNames;
    std::unordered_map<std::string, Index> m_rowNameToIndex;

    // Matrices
    numerics::SparseMatrix m_matrixA;
    std::optional<numerics::SparseMatrix> m_matrixQ;
};

} // namespace xenith::model

#endif // XENITH_MODEL_CANONICAL_MODEL_HPP

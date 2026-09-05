#include "xenith/model/canonical_model.hpp"
#include <stdexcept>

namespace xenith::model {

CanonicalModel::CanonicalModel(std::string name)
    : m_name(std::move(name)) {}

Index CanonicalModel::addVariable(const std::string& name,
                                  double lower_bound,
                                  double upper_bound,
                                  double obj_coeff,
                                  VariableType type) {
    if (m_varNameToIndex.find(name) != m_varNameToIndex.end()) {
        throw std::invalid_argument("Duplicate variable name: " + name);
    }
    Index idx = m_varNames.size();
    m_varNames.push_back(name);
    m_varNameToIndex[name] = idx;
    m_colLower.push_back(lower_bound);
    m_colUpper.push_back(upper_bound);
    m_objective.push_back(obj_coeff);
    m_varTypes.push_back(type);
    return idx;
}

Index CanonicalModel::addRow(const std::string& name, double lower_bound, double upper_bound) {
    if (m_rowNameToIndex.find(name) != m_rowNameToIndex.end()) {
        throw std::invalid_argument("Duplicate constraint row name: " + name);
    }
    Index idx = m_rowNames.size();
    m_rowNames.push_back(name);
    m_rowNameToIndex[name] = idx;
    m_rowLower.push_back(lower_bound);
    m_rowUpper.push_back(upper_bound);
    return idx;
}

Index CanonicalModel::addLessOrEqualRow(const std::string& name, double rhs) {
    return addRow(name, -K_INFINITY, rhs);
}

Index CanonicalModel::addGreaterOrEqualRow(const std::string& name, double rhs) {
    return addRow(name, rhs, K_INFINITY);
}

Index CanonicalModel::addEqualityRow(const std::string& name, double rhs) {
    return addRow(name, rhs, rhs);
}

Index CanonicalModel::addRangeRow(const std::string& name, double lower_bound, double upper_bound) {
    return addRow(name, lower_bound, upper_bound);
}

void CanonicalModel::setMatrixA(numerics::SparseMatrix matrix) {
    m_matrixA = std::move(matrix);
}

void CanonicalModel::setMatrixQ(numerics::SparseMatrix matrix) {
    m_matrixQ = std::move(matrix);
}

std::optional<Index> CanonicalModel::getVariableIndex(const std::string& name) const {
    auto it = m_varNameToIndex.find(name);
    if (it != m_varNameToIndex.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::optional<Index> CanonicalModel::getRowIndex(const std::string& name) const {
    auto it = m_rowNameToIndex.find(name);
    if (it != m_rowNameToIndex.end()) {
        return it->second;
    }
    return std::nullopt;
}

} // namespace xenith::model

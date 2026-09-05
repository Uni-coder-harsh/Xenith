#ifndef XENITH_IO_MPS_MPS_TYPES_HPP
#define XENITH_IO_MPS_MPS_TYPES_HPP

#include <string>
#include <vector>
#include <sstream>
#include "xenith/common/types.hpp"

namespace xenith::io::mps {

/// Recognized MPS file sections.
enum class MpsSection {
    NONE,
    NAME,
    OBJSENSE,
    ROWS,
    COLUMNS,
    RHS,
    RANGES,
    BOUNDS,
    ENDATA
};

/// Helper to convert MpsSection to string name.
inline std::string toString(MpsSection section) {
    switch (section) {
        case MpsSection::NONE: return "NONE";
        case MpsSection::NAME: return "NAME";
        case MpsSection::OBJSENSE: return "OBJSENSE";
        case MpsSection::ROWS: return "ROWS";
        case MpsSection::COLUMNS: return "COLUMNS";
        case MpsSection::RHS: return "RHS";
        case MpsSection::RANGES: return "RANGES";
        case MpsSection::BOUNDS: return "BOUNDS";
        case MpsSection::ENDATA: return "ENDATA";
    }
    return "UNKNOWN";
}

/// Configuration options for MpsReader.
struct MpsReaderOptions {
    /// Default objective sense if not specified in file (default: MINIMIZE).
    ObjectiveSense default_sense{ObjectiveSense::MINIMIZE};

    /// Name of specific objective row to select if multiple 'N' rows exist (empty = use first 'N' row).
    std::string objective_name;

    /// Allow free-format space-delimited MPS lines (default: true).
    bool allow_free_format{true};

    /// Enforce strict section ordering and error handling (default: false).
    bool strict_mode{false};
};

/// Detailed error diagnostic for MPS syntax or semantic parsing failures.
struct MpsParseError {
    std::string filename;
    std::size_t line_number{0};
    std::string section_name;
    std::string message;

    std::string toString() const {
        std::ostringstream oss;
        oss << "MPS parse error in file '" << filename << "'";
        if (line_number > 0) {
            oss << " at line " << line_number;
        }
        if (!section_name.empty()) {
            oss << " (section: " << section_name << ")";
        }
        oss << ": " << message;
        return oss.str();
    }
};

/// Exception thrown when MpsReader encounters a parsing error.
class MpsParseException : public std::runtime_error {
public:
    explicit MpsParseException(MpsParseError error)
        : std::runtime_error(error.toString()), m_error(std::move(error)) {}

    const MpsParseError& error() const { return m_error; }

private:
    MpsParseError m_error;
};

} // namespace xenith::io::mps

#endif // XENITH_IO_MPS_MPS_TYPES_HPP

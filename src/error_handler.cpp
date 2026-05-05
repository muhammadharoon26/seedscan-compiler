/***************************************************************************
 * error_handler.cpp — Implementation of Centralized Error Reporting
 * -------------------------------------------------------------------------
 * Formats error messages in a standard compiler-style format:
 *   Error [Semantic] at line 10, col 5: Undeclared variable 'x'
 ***************************************************************************/
#include "seedscan/error_handler.hpp"
#include <sstream>

namespace seedscan {

void ErrorHandler::reportError(const std::string& message, int line, int column,
                               const std::string& phase) {
    diagnostics_.emplace_back(ErrorLevel::ERROR, message, line, column, phase);
    errors_++;
}

void ErrorHandler::reportWarning(const std::string& message, int line, int column,
                                 const std::string& phase) {
    diagnostics_.emplace_back(ErrorLevel::WARNING, message, line, column, phase);
    warnings_++;
}

bool ErrorHandler::hasErrors() const {
    return errors_ > 0;
}

int ErrorHandler::errorCount() const {
    return errors_;
}

int ErrorHandler::warningCount() const {
    return warnings_;
}

void ErrorHandler::printAll(std::ostream& out) const {
    for (const auto& diag : diagnostics_) {
        // Format: "Error [Phase] at line L, col C: message"
        out << (diag.level == ErrorLevel::ERROR ? "Error" : "Warning")
            << " [" << diag.phase << "]"
            << " at line " << diag.line
            << ", col " << diag.column
            << ": " << diag.message
            << "\n";
    }

    // Summary line
    if (errors_ > 0 || warnings_ > 0) {
        out << "\n--- " << errors_ << " error(s), "
            << warnings_ << " warning(s) ---\n";
    }
}

void ErrorHandler::clear() {
    diagnostics_.clear();
    errors_ = 0;
    warnings_ = 0;
}

const std::vector<Diagnostic>& ErrorHandler::getDiagnostics() const {
    return diagnostics_;
}

} // namespace seedscan

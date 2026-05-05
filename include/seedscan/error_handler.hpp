/***************************************************************************
 * error_handler.hpp — Centralized Error Reporting for SeedScan Compiler
 * -------------------------------------------------------------------------
 * COMPILER THEORY:
 * A production compiler must provide clear, actionable error messages with
 * source location information. Rather than scattering fprintf(stderr, ...)
 * calls across modules, a centralized ErrorHandler collects all diagnostics
 * (errors and warnings) and formats them consistently.
 *
 * This enables:
 *   1. Counting total errors to decide whether to proceed to next phase
 *   2. Consistent formatting: "Error [line 10, col 5]: message"
 *   3. Potential future extension: error recovery, "did you mean?" hints
 ***************************************************************************/
#ifndef SEEDSCAN_ERROR_HANDLER_HPP
#define SEEDSCAN_ERROR_HANDLER_HPP

#include <string>
#include <vector>
#include <iostream>

namespace seedscan {

/*--------------------------------------------------------------------------
 * ErrorLevel — Severity classification for diagnostics.
 *------------------------------------------------------------------------*/
enum class ErrorLevel {
    WARNING,    // Non-fatal: compilation continues
    ERROR       // Fatal: compilation halts after current phase
};

/*--------------------------------------------------------------------------
 * Diagnostic — A single error or warning message with source location.
 *------------------------------------------------------------------------*/
struct Diagnostic {
    ErrorLevel  level;
    std::string message;
    int         line;
    int         column;
    std::string phase;   // "Lexer", "Parser", "Semantic", "IR"

    Diagnostic(ErrorLevel lvl, const std::string& msg, int ln, int col,
               const std::string& ph)
        : level(lvl), message(msg), line(ln), column(col), phase(ph) {}
};

/*--------------------------------------------------------------------------
 * ErrorHandler — Collects and reports all compiler diagnostics.
 *
 * Usage:
 *   ErrorHandler handler;
 *   handler.reportError("Undeclared variable 'x'", 10, 5, "Semantic");
 *   if (handler.hasErrors()) { return; }
 *------------------------------------------------------------------------*/
class ErrorHandler {
public:
    // Report an error at a specific source location
    void reportError(const std::string& message, int line, int column,
                     const std::string& phase);

    // Report a warning at a specific source location
    void reportWarning(const std::string& message, int line, int column,
                       const std::string& phase);

    // Check if any errors were reported
    bool hasErrors() const;

    // Get total error count
    int errorCount() const;

    // Get total warning count
    int warningCount() const;

    // Print all collected diagnostics to stderr
    void printAll(std::ostream& out = std::cerr) const;

    // Clear all diagnostics (useful between test cases)
    void clear();

    // Get all diagnostics (for testing)
    const std::vector<Diagnostic>& getDiagnostics() const;

private:
    std::vector<Diagnostic> diagnostics_;
    int errors_   = 0;
    int warnings_ = 0;
};

} // namespace seedscan

#endif // SEEDSCAN_ERROR_HANDLER_HPP

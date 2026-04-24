#ifndef JSON_PARSE_RESULT_H_
#define JSON_PARSE_RESULT_H_

#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// ParseError – structured error with location information.
// The controller returns this instead of raw strings that the view
// has to regex-parse to recover line/column numbers.
// ---------------------------------------------------------------------------
struct ParseError {
    std::string message;
    int         line   = 0;   // 0 = unknown
    int         column = 0;   // 0 = unknown

    ParseError() = default;
    ParseError(const std::string& msg, int ln = 0, int col = 0)
        : message(msg), line(ln), column(col) {}

    // Convenience: produce "message (line X, col Y)" or just "message"
    std::string toString() const {
        if (line > 0 && column > 0)
            return message + " (line " + std::to_string(line)
                   + ", column " + std::to_string(column) + ")";
        return message;
    }

    // Parse "... at line N, column M" from exception messages produced by
    // JSONTokenizer::formatError() so callers never need a regex.
    static ParseError fromException(const std::string& what) {
        ParseError err;
        err.message = what;

        // Try to extract "at line N, column M"
        const std::string lineTag   = "at line ";
        const std::string columnTag = ", column ";
        auto lp = what.rfind(lineTag);
        if (lp != std::string::npos) {
            try {
                size_t lineStart   = lp + lineTag.size();
                size_t lineEnd     = what.find(',', lineStart);
                err.line           = std::stoi(what.substr(lineStart, lineEnd - lineStart));
                auto cp            = what.rfind(columnTag);
                size_t colStart    = cp + columnTag.size();
                err.column         = std::stoi(what.substr(colStart));
                // Strip location from the human-readable message
                err.message = what.substr(0, lp - 1);
            } catch (...) {
                // If parsing fails, just keep the raw message
            }
        }
        return err;
    }
};

// ---------------------------------------------------------------------------
// OperationResult – used by all controller methods.
// Replaces the (bool success, string& errorMessage) out-parameter pattern.
// ---------------------------------------------------------------------------
struct OperationResult {
    bool                     success  = false;
    std::string              message;           // human-readable status / error
    ParseError               error;             // populated on failure
    std::vector<std::string> warnings;          // non-fatal notices

    static OperationResult ok(const std::string& msg = "",
                              std::vector<std::string> warns = {}) {
        OperationResult r;
        r.success  = true;
        r.message  = msg;
        r.warnings = std::move(warns);
        return r;
    }

    static OperationResult fail(const ParseError& err,
                                std::vector<std::string> warns = {}) {
        OperationResult r;
        r.success  = false;
        r.message  = err.toString();
        r.error    = err;
        r.warnings = std::move(warns);
        return r;
    }

    static OperationResult fail(const std::string& msg) {
        return fail(ParseError(msg));
    }

    explicit operator bool() const { return success; }
};

#endif // JSON_PARSE_RESULT_H_

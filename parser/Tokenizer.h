#ifndef JSON_TOKENIZER_H_
#define JSON_TOKENIZER_H_

#include <string>
#include <functional>
#include <stdexcept>
#include <cctype>
#include <vector>
#include "CommentPreservation.h"

struct Position {
    size_t offset;
    size_t line;
    size_t column;

    Position() : offset(0), line(1), column(1) {}
};

class JSONTokenizer {
private:
    std::string input;
    Position pos;
    bool relaxedMode;
    std::vector<Comment> capturedComments;  // Newly captured comments
    Position lastNonWhitespacePos;          // Track position of last meaningful token

public:
    JSONTokenizer(const std::string& input, bool relaxed = false)
        : input(input), pos(), relaxedMode(relaxed), lastNonWhitespacePos() {
    }

    void skipWhitespace();
    void skipComments(); // For relaxed mode
    char peek() const;
    char next();
    std::string readWhile(std::function<bool(char)> condition);

    Position getPosition() const { return pos; }
    void setPosition(const Position& p) { pos = p; }
    bool end() const { return pos.offset >= input.length(); }

    // Error reporting helper
    std::string formatError(const std::string& message) const;

    bool isRelaxedMode() const { return relaxedMode; }

    // Comment capture methods
    std::vector<Comment> extractCapturedComments() {
        std::vector<Comment> result = std::move(capturedComments);
        capturedComments.clear();
        return result;
    }

    bool hasCapturedComments() const {
        return !capturedComments.empty();
    }

    void markNonWhitespacePosition() {
        lastNonWhitespacePos = pos;
    }

private:
    void skipLineComment();
    void skipBlockComment();
    void captureLineComment();
    void captureBlockComment();
};

#endif // JSON_TOKENIZER_H_

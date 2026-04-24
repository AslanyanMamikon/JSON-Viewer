#include "Tokenizer.h"
#include <sstream>

void JSONTokenizer::skipWhitespace() {
    while (pos.offset < input.length() &&
           std::isspace(static_cast<unsigned char>(input[pos.offset]))) {
        if (input[pos.offset] == '\n') {
            pos.line++;
            pos.column = 1;
        }
        else {
            pos.column++;
        }
        pos.offset++;
    }

    if (relaxedMode) {
        skipComments();
    }
}

void JSONTokenizer::skipComments() {
    while (pos.offset < input.length()) {
        if (pos.offset + 1 < input.length() &&
            input[pos.offset] == '/' && input[pos.offset + 1] == '/') {
            captureLineComment();
            skipWhitespace();
        }
        else if (pos.offset + 1 < input.length() &&
                 input[pos.offset] == '/' && input[pos.offset + 1] == '*') {
            captureBlockComment();
            skipWhitespace();
        }
        else {
            break;
        }
    }
}

void JSONTokenizer::captureLineComment() {
    Position commentStart = pos;

    // Skip '//'
    pos.offset += 2;
    pos.column += 2;

    // Capture comment text (including leading space if present)
    std::string commentText;
    while (pos.offset < input.length() && input[pos.offset] != '\n') {
        commentText += input[pos.offset];
        pos.offset++;
        pos.column++;
    }

    // Determine if this is a trailing comment
    // A comment is trailing ONLY if:
    // 1. It's on the same line as the last non-whitespace token
    // 2. There was actual content before it on that line
    bool isTrailing = (commentStart.line == lastNonWhitespacePos.line &&
                       lastNonWhitespacePos.column > 1);

    // Store the comment
    capturedComments.emplace_back(
        Comment::Type::LINE,
        commentText,
        commentStart.line,
        commentStart.column,
        isTrailing
        );

    // Skip the newline
    if (pos.offset < input.length() && input[pos.offset] == '\n') {
        pos.offset++;
        pos.line++;
        pos.column = 1;
    }
}

void JSONTokenizer::captureBlockComment() {
    Position commentStart = pos;
    size_t startLine = pos.line;

    // Skip '/*'
    pos.offset += 2;
    pos.column += 2;

    // Capture comment text
    std::string commentText;
    while (pos.offset + 1 < input.length()) {
        if (input[pos.offset] == '*' && input[pos.offset + 1] == '/') {
            // Skip '*/' but don't include in text
            pos.offset += 2;
            pos.column += 2;
            break;
        }

        commentText += input[pos.offset];

        if (input[pos.offset] == '\n') {
            pos.line++;
            pos.column = 1;
        }
        else {
            pos.column++;
        }
        pos.offset++;
    }

    // Determine if this is a trailing comment
    // Block comment is trailing only if it starts and ends on the same line as the last token
    bool isTrailing = (startLine == lastNonWhitespacePos.line &&
                       pos.line == startLine &&
                       lastNonWhitespacePos.column > 1);

    // Store the comment
    capturedComments.emplace_back(
        Comment::Type::BLOCK,
        commentText,
        commentStart.line,
        commentStart.column,
        isTrailing
        );
}

void JSONTokenizer::skipLineComment() {
    captureLineComment();
}

void JSONTokenizer::skipBlockComment() {
    captureBlockComment();
}

char JSONTokenizer::peek() const {
    if (pos.offset >= input.length()) {
        throw std::runtime_error(formatError("Unexpected end of input"));
    }
    return input[pos.offset];
}

char JSONTokenizer::next() {
    if (pos.offset >= input.length()) {
        throw std::runtime_error(formatError("Unexpected end of input"));
    }
    char c = input[pos.offset++];
    if (c == '\n') {
        pos.line++;
        pos.column = 1;
    }
    else {
        pos.column++;
    }
    return c;
}

std::string JSONTokenizer::readWhile(std::function<bool(char)> condition) {
    size_t start = pos.offset;
    while (pos.offset < input.length() && condition(input[pos.offset])) {
        if (input[pos.offset] == '\n') {
            pos.line++;
            pos.column = 1;
        }
        else {
            pos.column++;
        }
        pos.offset++;
    }
    return input.substr(start, pos.offset - start);
}

std::string JSONTokenizer::formatError(const std::string& message) const {
    std::ostringstream oss;
    oss << message << " at line " << pos.line << ", column " << pos.column;
    return oss.str();
}

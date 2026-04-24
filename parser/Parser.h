#ifndef JSON_PARSER_H_
#define JSON_PARSER_H_

#include "Value.h"
#include "ValueBuilder.h"
#include "Tokenizer.h"
#include "CommentPreservation.h"
#include <stdexcept>
#include <memory>
#include <fstream>
#include <sstream>
#include <set>
#include <stack>

enum class ParseMode {
    STANDARD,
    STRICT_MODE,
    RELAXED
};

// Base Parser interface
class IJSONParser {
public:
    virtual ~IJSONParser() = default;
    virtual std::shared_ptr<JSONValue> parse() = 0;
    virtual ParseMode getMode() const = 0;
    virtual std::shared_ptr<CommentStore> getCommentStore() const = 0;
};

// Parser implementation
class JSONParser : public IJSONParser {
public:
    JSONParser(const std::string& input, ParseMode mode = ParseMode::STANDARD);
    std::shared_ptr<JSONValue> parse() override;
    ParseMode getMode() const override { return mode; }
    std::shared_ptr<CommentStore> getCommentStore() const override { return commentStore; }

private:
    std::unique_ptr<JSONTokenizer> tokenizer;
    ParseMode mode;
    std::shared_ptr<CommentStore> commentStore;
    std::stack<std::string> pathStack;  // Track current path in JSON tree

    static constexpr size_t MAX_DEPTH = 100;
    size_t currentDepth;

private:
    std::shared_ptr<JSONValue> parseValue();
    std::shared_ptr<JSONValue> parseObject();
    std::shared_ptr<JSONValue> parseArray();
    std::shared_ptr<JSONValue> parseString();
    std::shared_ptr<JSONValue> parseNumber();
    std::shared_ptr<JSONValue> parseBoolean();
    std::shared_ptr<JSONValue> parseNull();

    // Helper for strict mode
    void validateNoLeadingZeros(const std::string& numStr);

    // Helper for relaxed mode
    std::string parseUnquotedKey();

    void enterScope();
    void exitScope();

    // Comment handling
    void collectCommentsBeforeValue();
    void collectCommentsAfterValue();
    std::string getCurrentPath() const;
    void pushPath(const std::string& segment);
    void popPath();
};

#endif // !JSON_PARSER_H_

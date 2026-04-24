#include "Parser.h"

// JSONParser Constructor
JSONParser::JSONParser(const std::string& input, ParseMode mode)
    : tokenizer(std::make_unique<JSONTokenizer>(input, mode == ParseMode::RELAXED)),
    mode(mode), commentStore(std::make_shared<CommentStore>()), currentDepth(0)
{
    pathStack.push("root");
}

std::string JSONParser::getCurrentPath() const {
    if (pathStack.empty()) return "";
    std::stack<std::string> temp = pathStack;
    std::vector<std::string> parts;
    while (!temp.empty()) {
        parts.push_back(temp.top());
        temp.pop();
    }
    std::reverse(parts.begin(), parts.end());
    std::string result;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) result += ".";
        result += parts[i];
    }
    return result;
}

void JSONParser::pushPath(const std::string& segment) {
    pathStack.push(segment);
}

void JSONParser::popPath() {
    if (!pathStack.empty()) {
        pathStack.pop();
    }
}

void JSONParser::collectCommentsBeforeValue() {
    if (mode == ParseMode::RELAXED && tokenizer->hasCapturedComments()) {
        auto comments = tokenizer->extractCapturedComments();
        std::string path = getCurrentPath();
        for (const auto& comment : comments) {
            // Only add non-trailing comments as "before" comments
            if (!comment.isTrailing) {
                commentStore->addBeforeComment(path, comment);
            }
        }
    }
}

void JSONParser::collectCommentsAfterValue() {
    if (mode == ParseMode::RELAXED) {
        tokenizer->skipWhitespace();
        if (tokenizer->hasCapturedComments()) {
            auto comments = tokenizer->extractCapturedComments();
            std::string path = getCurrentPath();
            for (const auto& comment : comments) {
                if (comment.isTrailing) {
                    commentStore->addTrailingComment(path, comment);
                } else {
                    commentStore->addAfterComment(path, comment);
                }
            }
        }
    }
}

void JSONParser::enterScope() {
    if (++currentDepth > MAX_DEPTH) {
        throw std::runtime_error(tokenizer->formatError(
            "Maximum nesting depth of " + std::to_string(MAX_DEPTH) + " exceeded"));
    }
}

void JSONParser::exitScope() {
    --currentDepth;
}

std::shared_ptr<JSONValue> JSONParser::parseValue() {
    // Don't collect comments here - they should be collected in parseObject/parseArray
    // for specific fields/elements

    tokenizer->skipWhitespace();
    tokenizer->markNonWhitespacePosition();
    char current = tokenizer->peek();

    std::shared_ptr<JSONValue> result;

    if (current == '{') {
        result = parseObject();
    }
    else if (current == '[') {
        result = parseArray();
    }
    else if (current == '"' || (mode == ParseMode::RELAXED && current == '\'')) {
        result = parseString();
    }
    else if (std::isdigit(static_cast<unsigned char>(current)) || current == '-') {
        result = parseNumber();
    }
    else if (current == 't' || current == 'f') {
        result = parseBoolean();
    }
    else if (current == 'n') {
        result = parseNull();
    }
    else {
        throw std::runtime_error(tokenizer->formatError(
            "Unexpected character: " + std::string(1, current)));
    }

    tokenizer->markNonWhitespacePosition();
    return result;
}

std::shared_ptr<JSONValue> JSONParser::parseObject() {
    enterScope();

    JSONValueBuilder builder;
    builder.startObject();
    std::set<std::string> keys;

    tokenizer->next(); // Skip '{'
    tokenizer->markNonWhitespacePosition();
    tokenizer->skipWhitespace();

    if (tokenizer->peek() == '}') {
        tokenizer->next();
        tokenizer->markNonWhitespacePosition();
        builder.endObject();
        exitScope();
        return builder.build();
    }

    while (true) {
        // Collect comments BEFORE this specific field
        tokenizer->skipWhitespace();

        std::string key;

        if (tokenizer->peek() == '"' || (mode == ParseMode::RELAXED && tokenizer->peek() == '\'')) {
            auto keyValue = parseString();
            key = keyValue->asString();
        }
        else if (mode == ParseMode::RELAXED && std::isalpha(static_cast<unsigned char>(tokenizer->peek()))) {
            key = parseUnquotedKey();
        }
        else {
            exitScope();
            throw std::runtime_error(tokenizer->formatError(
                "Expected property name"));
        }

        if (mode == ParseMode::STRICT_MODE) {
            if (keys.find(key) != keys.end()) {
                exitScope();
                throw std::runtime_error(tokenizer->formatError(
                    "Duplicate key '" + key + "' in strict mode"));
            }
            keys.insert(key);
        }

        pushPath(key);

        // NOW collect comments that appeared before this field
        collectCommentsBeforeValue();

        tokenizer->skipWhitespace();
        if (tokenizer->peek() != ':') {
            popPath();
            exitScope();
            throw std::runtime_error(tokenizer->formatError("Expected ':'"));
        }
        tokenizer->next();
        tokenizer->markNonWhitespacePosition();

        auto value = parseValue();
        builder.addProperty(key, value);

        // Collect comments after this field's value
        collectCommentsAfterValue();
        popPath();

        tokenizer->skipWhitespace();

        if (tokenizer->peek() == '}') {
            tokenizer->next();
            tokenizer->markNonWhitespacePosition();
            builder.endObject();
            exitScope();
            return builder.build();
        }

        if (tokenizer->peek() == ',') {
            tokenizer->next();
            tokenizer->markNonWhitespacePosition();
            continue;
        }
        exitScope();
        throw std::runtime_error(tokenizer->formatError("Expected ',' or '}'"));
    }
}

std::shared_ptr<JSONValue> JSONParser::parseArray() {
    enterScope();

    JSONValueBuilder builder;
    builder.startArray();

    tokenizer->next(); // Skip '['
    tokenizer->markNonWhitespacePosition();
    tokenizer->skipWhitespace();

    if (tokenizer->peek() == ']') {
        tokenizer->next();
        tokenizer->markNonWhitespacePosition();
        builder.endArray();
        exitScope();
        return builder.build();
    }

    size_t index = 0;
    while (true) {
        pushPath("[" + std::to_string(index) + "]");

        // Collect comments before this array element
        collectCommentsBeforeValue();

        auto element = parseValue();
        builder.addElement(element);

        // Collect comments after this array element
        collectCommentsAfterValue();
        popPath();

        tokenizer->skipWhitespace();

        if (tokenizer->peek() == ']') {
            tokenizer->next();
            tokenizer->markNonWhitespacePosition();
            builder.endArray();
            exitScope();
            return builder.build();
        }

        if (tokenizer->peek() == ',') {
            tokenizer->next();
            tokenizer->markNonWhitespacePosition();
            index++;
            continue;
        }
        exitScope();
        throw std::runtime_error(tokenizer->formatError("Expected ',' or ']'"));
    }
}

std::shared_ptr<JSONValue> JSONParser::parseString() {
    char quote = tokenizer->next();

    if (mode != ParseMode::RELAXED && quote == '\'') {
        throw std::runtime_error(tokenizer->formatError(
            "Single quotes not allowed"));
    }

    std::string result;

    while (!tokenizer->end() && tokenizer->peek() != quote) {
        if (tokenizer->peek() == '\\') {
            tokenizer->next();
            if (tokenizer->end()) {
                throw std::runtime_error(tokenizer->formatError("Unexpected end"));
            }
            switch (tokenizer->next()) {
            case '"': result += '"'; break;
            case '\'': result += '\''; break;
            case '\\': result += '\\'; break;
            case '/': result += '/'; break;
            case 'b': result += '\b'; break;
            case 'f': result += '\f'; break;
            case 'n': result += '\n'; break;
            case 'r': result += '\r'; break;
            case 't': result += '\t'; break;
            default:
                throw std::runtime_error(tokenizer->formatError("Invalid escape"));
            }
        }
        else {
            result += tokenizer->next();
        }
    }

    if (tokenizer->end()) {
        throw std::runtime_error(tokenizer->formatError("Unterminated string"));
    }
    tokenizer->next();
    return std::make_shared<JSONValue>(result);
}

void JSONParser::validateNoLeadingZeros(const std::string& numStr) {
    if (numStr.length() > 1 && numStr[0] == '0' &&
        std::isdigit(static_cast<unsigned char>(numStr[1]))) {
        throw std::runtime_error(tokenizer->formatError("Leading zeros not allowed"));
    }
    if (numStr.length() > 2 && numStr[0] == '-' && numStr[1] == '0' &&
        std::isdigit(static_cast<unsigned char>(numStr[2]))) {
        throw std::runtime_error(tokenizer->formatError("Leading zeros not allowed"));
    }
}

std::shared_ptr<JSONValue> JSONParser::parseNumber() {
    std::string numStr;
    bool isFloat = false;

    if (tokenizer->peek() == '-') {
        numStr += tokenizer->next();
    }

    if (tokenizer->end() || !std::isdigit(static_cast<unsigned char>(tokenizer->peek()))) {
        throw std::runtime_error(tokenizer->formatError("Invalid number"));
    }

    while (!tokenizer->end() && std::isdigit(static_cast<unsigned char>(tokenizer->peek()))) {
        numStr += tokenizer->next();
    }

    if (mode == ParseMode::STRICT_MODE) {
        validateNoLeadingZeros(numStr);
    }

    if (!tokenizer->end() && tokenizer->peek() == '.') {
        isFloat = true;
        numStr += tokenizer->next();

        if (tokenizer->end() || !std::isdigit(static_cast<unsigned char>(tokenizer->peek()))) {
            throw std::runtime_error(tokenizer->formatError("Invalid number"));
        }

        while (!tokenizer->end() && std::isdigit(static_cast<unsigned char>(tokenizer->peek()))) {
            numStr += tokenizer->next();
        }
    }

    if (!tokenizer->end() && (tokenizer->peek() == 'e' || tokenizer->peek() == 'E')) {
        isFloat = true;
        numStr += tokenizer->next();

        if (!tokenizer->end() && (tokenizer->peek() == '+' || tokenizer->peek() == '-')) {
            numStr += tokenizer->next();
        }

        if (tokenizer->end() || !std::isdigit(static_cast<unsigned char>(tokenizer->peek()))) {
            throw std::runtime_error(tokenizer->formatError("Invalid number"));
        }

        while (!tokenizer->end() && std::isdigit(static_cast<unsigned char>(tokenizer->peek()))) {
            numStr += tokenizer->next();
        }
    }

    try {
        if (isFloat) {
            return std::make_shared<JSONValue>(std::stof(numStr));
        } else {
            return std::make_shared<JSONValue>(std::stoi(numStr));
        }
    }
    catch (const std::exception&) {
        throw std::runtime_error(tokenizer->formatError("Invalid number"));
    }
}

std::shared_ptr<JSONValue> JSONParser::parseBoolean() {
    tokenizer->skipWhitespace();
    std::string token = tokenizer->readWhile([](char c) {
        return std::isalpha(static_cast<unsigned char>(c));
    });

    if (token == "true") {
        return std::make_shared<JSONValue>(true);
    }
    else if (token == "false") {
        return std::make_shared<JSONValue>(false);
    }
    throw std::runtime_error(tokenizer->formatError("Invalid boolean"));
}

std::shared_ptr<JSONValue> JSONParser::parseNull() {
    std::string token = tokenizer->readWhile([](char c) {
        return std::isalpha(static_cast<unsigned char>(c));
    });

    if (token == "null") {
        return std::make_shared<JSONValue>();
    }
    throw std::runtime_error(tokenizer->formatError("Invalid null"));
}

std::string JSONParser::parseUnquotedKey() {
    return tokenizer->readWhile([](char c) {
        return std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '$';
    });
}

std::shared_ptr<JSONValue> JSONParser::parse() {
    // Collect any header comments (before the root element)
    tokenizer->skipWhitespace();
    if (mode == ParseMode::RELAXED && tokenizer->hasCapturedComments()) {
        auto headerComments = tokenizer->extractCapturedComments();
        for (const auto& comment : headerComments) {
            commentStore->addHeaderComment(comment);
        }
    }

    auto result = parseValue();

    // Collect any footer comments (after the root element)
    tokenizer->skipWhitespace();
    if (mode == ParseMode::RELAXED && tokenizer->hasCapturedComments()) {
        auto footerComments = tokenizer->extractCapturedComments();
        for (const auto& comment : footerComments) {
            commentStore->addFooterComment(comment);
        }
    }

    if (!tokenizer->end()) {
        throw std::runtime_error(tokenizer->formatError("Unexpected characters after JSON"));
    }
    return result;
}

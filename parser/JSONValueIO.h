#ifndef JSON_VALUE_IO_H_
#define JSON_VALUE_IO_H_

#include "Value.h"
#include "Parser.h"
#include "ParserFactory.h"
#include "CommentPreservation.h"
#include <fstream>
#include <sstream>
#include <memory>

class JSONValueIO {
public:
    static void saveToFile(const std::shared_ptr<JSONValue>& value,
                           const std::string& filename,
                           bool formatted = true,
                           std::shared_ptr<CommentStore> comments = nullptr) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file for writing: " + filename);
        }

        if (formatted) {
            printWithComments(file, value, comments, 0, "root");
        }
        else {
            value->print(file, 0);
        }

        file.close();
    }

    static std::shared_ptr<JSONValue> loadFromFile(
        const std::string& filename,
        JSONParserFactory::ParserType parserType = JSONParserFactory::ParserType::STANDARD) {

        auto parser = JSONParserFactory::createFromFile(filename, parserType);
        return parser->parse();
    }

    static std::string toString(const std::shared_ptr<JSONValue>& value,
                                bool formatted = true,
                                std::shared_ptr<CommentStore> comments = nullptr) {
        std::ostringstream oss;
        if (formatted && comments) {
            printWithComments(oss, value, comments, 0, "root");
        }
        else if (formatted) {
            value->print(oss, 0);
        }
        else {
            value->print(oss, 0);
            std::string output = oss.str();
            output.erase(std::remove(output.begin(), output.end(), '\n'), output.end());
            return output;
        }
        return oss.str();
    }

    static std::shared_ptr<JSONValue> fromString(
        const std::string& jsonStr,
        JSONParserFactory::ParserType parserType = JSONParserFactory::ParserType::STANDARD) {

        auto parser = JSONParserFactory::createFromString(jsonStr, parserType);
        return parser->parse();
    }

private:
    static void printWithComments(std::ostream& os,
                                  const std::shared_ptr<JSONValue>& value,
                                  std::shared_ptr<CommentStore> comments,
                                  int indent,
                                  const std::string& path) {
        if (!value) return;

        std::string indentStr(indent * 2, ' ');
        std::string nextIndentStr((indent + 1) * 2, ' ');

        // Print header comments (only for root, before opening brace)
        if (path == "root" && comments) {
            for (const auto& comment : comments->getHeaderComments()) {
                printComment(os, comment, "");
            }
        }

        // Print the value
        if (value->isNull()) {
            os << "null";
        }
        else if (value->isInt()) {
            os << value->asInt();
        }
        else if (value->isFloat()) {
            os << value->asFloat();
        }
        else if (value->isBool()) {
            os << (value->asBool() ? "true" : "false");
        }
        else if (value->isString()) {
            os << '"' << value->asString() << '"';
        }
        else if (value->isObject()) {
            const auto& obj = value->asObject();
            if (obj.empty()) {
                os << "{}";
            }
            else {
                os << "{\n";
                auto it = obj.begin();
                auto end = obj.end();
                for (; it != end; ++it) {
                    const auto& [key, val] = *it;
                    std::string fieldPath = path + "." + key;

                    // Print comments BEFORE this field (on their own lines)
                    if (comments) {
                        for (const auto& comment : comments->getBeforeComments(fieldPath)) {
                            os << nextIndentStr;
                            printComment(os, comment, "");
                        }
                    }

                    // Print the field
                    os << nextIndentStr << '"' << key << "\": ";
                    printWithComments(os, val, comments, indent + 1, fieldPath);

                    // Print comma if not last
                    bool isLast = (std::next(it) == end);
                    if (!isLast) {
                        os << ",";
                    }

                    // Print trailing comments (on same line)
                    if (comments) {
                        const auto& trailing = comments->getTrailingComments(fieldPath);
                        if (!trailing.empty()) {
                            os << " ";
                            for (const auto& comment : trailing) {
                                printCommentInline(os, comment);
                            }
                        }
                    }

                    os << "\n";

                    // Print AFTER comments (on their own lines after the field)
                    if (comments) {
                        for (const auto& comment : comments->getAfterComments(fieldPath)) {
                            os << nextIndentStr;
                            printComment(os, comment, "");
                        }
                    }
                }

                os << indentStr << "}";
            }
        }
        else if (value->isArray()) {
            const auto& arr = value->asArray();
            if (arr.empty()) {
                os << "[]";
            }
            else {
                os << "[\n";
                for (size_t i = 0; i < arr.size(); ++i) {
                    std::string elementPath = path + "[" + std::to_string(i) + "]";

                    // Print comments BEFORE this array element (on their own lines)
                    if (comments) {
                        for (const auto& comment : comments->getBeforeComments(elementPath)) {
                            os << nextIndentStr;
                            printComment(os, comment, "");
                        }
                    }

                    // Print the element
                    os << nextIndentStr;
                    printWithComments(os, arr[i], comments, indent + 1, elementPath);

                    // Print comma if not last
                    if (i < arr.size() - 1) {
                        os << ",";
                    }

                    // Print trailing comments (on same line)
                    if (comments) {
                        const auto& trailing = comments->getTrailingComments(elementPath);
                        if (!trailing.empty()) {
                            os << " ";
                            for (const auto& comment : trailing) {
                                printCommentInline(os, comment);
                            }
                        }
                    }

                    os << "\n";

                    // Print AFTER comments (on their own lines after the element)
                    if (comments) {
                        for (const auto& comment : comments->getAfterComments(elementPath)) {
                            os << nextIndentStr;
                            printComment(os, comment, "");
                        }
                    }
                }
                os << indentStr << "]";
            }
        }

        // Print footer comments (only for root, after closing brace)
        if (path == "root" && comments) {
            const auto& footerComments = comments->getFooterComments();
            if (!footerComments.empty()) {
                os << "\n";
                for (const auto& comment : footerComments) {
                    printComment(os, comment, "");
                }
            }
        }
    }

    static void printComment(std::ostream& os, const Comment& comment, const std::string& indent) {
        os << indent;
        if (comment.type == Comment::Type::LINE) {
            os << "//" << comment.text << "\n";
        }
        else {
            os << "/*" << comment.text << "*/" << "\n";
        }
    }

    static void printCommentInline(std::ostream& os, const Comment& comment) {
        if (comment.type == Comment::Type::LINE) {
            os << "//" << comment.text;
        }
        else {
            os << "/*" << comment.text << "*/";
        }
    }
};

#endif // JSON_VALUE_IO_H_

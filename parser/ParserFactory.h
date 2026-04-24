#ifndef JSON_PARSER_FACTORY_H_
#define JSON_PARSER_FACTORY_H_

#include <memory>
#include <string>

class IJSONParser;

class JSONParserFactory {
public:
    enum class ParserType {
        STANDARD,
        STRICT_MODE,
        RELAXED
    };

    static std::unique_ptr<IJSONParser> createParser(
        const std::string& input,
        ParserType type = ParserType::STANDARD
    );

    static std::unique_ptr<IJSONParser> createFromString(
        const std::string& input,
        ParserType type = ParserType::STANDARD
    );

    static std::unique_ptr<IJSONParser> createFromFile(
        const std::string& filename,
        ParserType type = ParserType::STANDARD
    );
};
#endif // JSON_PARSER_FACTORY_H_


#include "ParserFactory.h"
#include "Parser.h"
#include <fstream>
#include <sstream>

// Factory implementation
std::unique_ptr<IJSONParser> JSONParserFactory::createParser(
    const std::string& input,
    ParserType type)
{
    switch (type) {
    case ParserType::STANDARD:
        return std::make_unique<JSONParser>(input, ParseMode::STANDARD);
    case ParserType::STRICT_MODE:
        return std::make_unique<JSONParser>(input, ParseMode::STRICT_MODE);
    case ParserType::RELAXED:
        return std::make_unique<JSONParser>(input, ParseMode::RELAXED);
    default:
        throw std::runtime_error("Unknown parser type");
    }
}

std::unique_ptr<IJSONParser> JSONParserFactory::createFromString(
    const std::string& input,
    ParserType type)
{
    return createParser(input, type);
}

std::unique_ptr<IJSONParser> JSONParserFactory::createFromFile(
    const std::string& filename,
    ParserType type)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return createParser(buffer.str(), type);
}

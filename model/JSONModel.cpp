#include "JSONModel.h"
#include <fstream>
#include <stdexcept>

JSONModel::JSONModel()
    : document(nullptr)
    , commentStore(std::make_shared<CommentStore>())
    , currentFilePath("")
    , modified(false) {}

void JSONModel::loadFromFile(const std::string& filepath, JSONParserFactory::ParserType parserType)
{
    warnings.clear();

    try {
        auto parser = JSONParserFactory::createFromFile(filepath, parserType);
        document = parser->parse();
        commentStore = parser->getCommentStore();
        currentFilePath = filepath;
        modified = false;

        // Add warnings if comments were found in non-relaxed mode
        if (parserType != JSONParserFactory::ParserType::RELAXED &&
            commentStore && !commentStore->isEmpty()) {
            warnings.push_back("Comments found in JSON (enable relaxed mode to preserve them)");
        }
    }
    catch (const std::exception& e) {
        warnings.push_back(std::string("Parse warning: ") + e.what());
        throw;
    }
}

void JSONModel::saveToFile(const std::string& filepath)
{
    if(!document)
        throw std::runtime_error("No document to save");

    JSONValueIO::saveToFile(document, filepath, true, commentStore);
    currentFilePath = filepath;
    modified = false;
}

void JSONModel::parseFromString(const std::string& jsonText, JSONParserFactory::ParserType parserType)
{
    warnings.clear();

    if(jsonText.empty())
    {
        document = nullptr;
        commentStore = std::make_shared<CommentStore>();
        return;
    }

    try {
        auto parser = JSONParserFactory::createFromString(jsonText, parserType);
        document = parser->parse();
        commentStore = parser->getCommentStore();

        // Add warnings if comments were found in non-relaxed mode
        if (parserType != JSONParserFactory::ParserType::RELAXED &&
            commentStore && !commentStore->isEmpty()) {
            warnings.push_back("Comments found in JSON (enable relaxed mode to preserve them)");
        }
    }
    catch (const std::exception& e) {
        warnings.push_back(std::string("Parse warning: ") + e.what());
        throw;
    }
}

std::string JSONModel::formatToString(bool prettyPrint) const
{
    if(!document)
        return "";

    return JSONValueIO::toString(document, prettyPrint, commentStore);
}

bool JSONModel::validate(std::string& errorMessage) const
{
    if(!document){
        errorMessage = "No document loaded";
        return false;
    }

    errorMessage = "Document is valid";
    return true;
}

void JSONModel::setDocument(std::shared_ptr<JSONValue> doc)
{
    document = doc;
    modified = true;
}

void JSONModel::clear()
{
    document = nullptr;
    commentStore = std::make_shared<CommentStore>();
    currentFilePath.clear();
    modified = false;
    warnings.clear();
}

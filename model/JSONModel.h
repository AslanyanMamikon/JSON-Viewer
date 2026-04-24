#ifndef JSONMODEL_H
#define JSONMODEL_H

#include <memory>
#include <string>
#include <vector>
#include "Value.h"
#include "ParserFactory.h"
#include "JSONValueIO.h"
#include "CommentPreservation.h"

class JSONModel
{
public:
    JSONModel();

    void loadFromFile(const std::string& filepath, JSONParserFactory::ParserType parserType);
    void saveToFile(const std::string& filepath);
    void parseFromString(const std::string& jsonText, JSONParserFactory::ParserType parserType);
    std::string formatToString(bool prettyPrint = true) const;

    bool validate(std::string& errorMessage) const;

    std::shared_ptr<JSONValue> getDocument() const {return document;}
    std::string getCurrentFilePath() const { return currentFilePath; }
    bool isModified() const { return modified; }
    bool hasDocument() const { return document != nullptr; }
    std::shared_ptr<CommentStore> getCommentStore() const { return commentStore; }

    // Warnings support
    std::vector<std::string> getWarnings() const { return warnings; }
    void clearWarnings() { warnings.clear(); }
    void addWarning(const std::string& warning) { warnings.push_back(warning); }

    void setDocument(std::shared_ptr<JSONValue> doc);
    void setModified(bool mod) { modified = mod; }
    void setCurrentFilePath(const std::string& path) { currentFilePath = path; }

    void clear();

private:
    std::shared_ptr<JSONValue> document;
    std::shared_ptr<CommentStore> commentStore;
    std::string currentFilePath;
    bool modified;
    std::vector<std::string> warnings;  // Store parsing/validation warnings
};

#endif // JSONMODEL_H

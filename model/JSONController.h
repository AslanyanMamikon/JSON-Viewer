#ifndef JSONCONTROLLER_H
#define JSONCONTROLLER_H

#include <memory>
#include <string>
#include <vector>
#include "JSONModel.h"
#include "ParserFactory.h"
#include "ParseResult.h"
#include "JSONSchema.h"

class JSONController {
public:
    JSONController();

    // ---- File I/O -----------------------------------------------------------
    OperationResult openFile  (const std::string& filepath,
                             JSONParserFactory::ParserType parserType);
    OperationResult saveFile  ();
    OperationResult saveFileAs(const std::string& filepath);

    // ---- Editing ------------------------------------------------------------
    OperationResult parseText (const std::string& jsonText,
                              JSONParserFactory::ParserType parserType);
    OperationResult formatJSON();

    // ---- Validation ---------------------------------------------------------
    // Basic structural validation (always available)
    OperationResult validateJSON();

    // Schema validation: validates the current document against `schema`.
    // Returns errors in OperationResult::warnings if schema is violated.
    OperationResult validateWithSchema(const JSONSchema& schema);

    // ---- Schema management --------------------------------------------------
    // Attach a schema that will be used automatically on every parse/open.
    // Pass nullptr to clear the active schema.
    void setActiveSchema(std::shared_ptr<JSONSchema> schema);
    bool hasActiveSchema() const { return activeSchema != nullptr; }

    // ---- Accessors ----------------------------------------------------------
    std::shared_ptr<JSONValue>       getDocument()       const;
    std::string                      getCurrentFilePath() const;
    bool                             hasUnsavedChanges() const;
    bool                             hasDocument()       const;

    void setParserMode(JSONParserFactory::ParserType mode) { currentParserMode = mode; }
    JSONParserFactory::ParserType getParserMode() const    { return currentParserMode; }

    void markAsModified();
    void clearDocument();

private:
    std::unique_ptr<JSONModel>       model;
    JSONParserFactory::ParserType    currentParserMode;
    std::shared_ptr<JSONSchema>      activeSchema;   // optional, may be nullptr

    // After every successful parse, run the active schema if one is set.
    // Appends schema violations as warnings in the result.
    void appendSchemaWarnings(OperationResult& result) const;
};

#endif // JSONCONTROLLER_H

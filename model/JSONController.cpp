#include "JSONController.h"

JSONController::JSONController()
    : model(std::make_unique<JSONModel>())
    , currentParserMode(JSONParserFactory::ParserType::STANDARD)
    , activeSchema(nullptr)
{}

// ---------------------------------------------------------------------------
// Private helper
// ---------------------------------------------------------------------------
void JSONController::appendSchemaWarnings(OperationResult& result) const {
    if (!activeSchema || !model->hasDocument()) return;

    std::vector<std::string> schemaErrors;
    if (!activeSchema->validate(model->getDocument(), schemaErrors)) {
        for (auto& e : schemaErrors)
            result.warnings.push_back("Schema: " + e);
    }
}

// ---------------------------------------------------------------------------
// File I/O
// ---------------------------------------------------------------------------
OperationResult JSONController::openFile(const std::string& filepath,
                                         JSONParserFactory::ParserType parserType)
{
    try {
        model->loadFromFile(filepath, parserType);
        auto result = OperationResult::ok("File loaded successfully",
                                          model->getWarnings());
        appendSchemaWarnings(result);
        return result;
    } catch (const std::exception& e) {
        return OperationResult::fail(ParseError::fromException(e.what()));
    }
}

OperationResult JSONController::saveFile()
{
    try {
        std::string filepath = model->getCurrentFilePath();
        if (filepath.empty())
            return OperationResult::fail("No file path specified. Use Save As instead.");
        model->saveToFile(filepath);
        return OperationResult::ok("File saved successfully");
    } catch (const std::exception& e) {
        return OperationResult::fail(ParseError::fromException(e.what()));
    }
}

OperationResult JSONController::saveFileAs(const std::string& filepath)
{
    try {
        model->saveToFile(filepath);
        return OperationResult::ok("File saved successfully");
    } catch (const std::exception& e) {
        return OperationResult::fail(ParseError::fromException(e.what()));
    }
}

// ---------------------------------------------------------------------------
// Editing
// ---------------------------------------------------------------------------
OperationResult JSONController::parseText(const std::string& jsonText,
                                          JSONParserFactory::ParserType parserType)
{
    try {
        model->parseFromString(jsonText, parserType);
        model->setModified(true);
        auto result = OperationResult::ok("JSON parsed successfully",
                                          model->getWarnings());
        appendSchemaWarnings(result);
        return result;
    } catch (const std::exception& e) {
        return OperationResult::fail(ParseError::fromException(e.what()));
    }
}

OperationResult JSONController::formatJSON()
{
    if (!model->hasDocument())
        return OperationResult::fail("No document to format");
    try {
        // formatToString is called by the view via getDocument() path;
        // here we just confirm the model is in a formattable state.
        return OperationResult::ok("JSON formatted successfully",
                                   model->getWarnings());
    } catch (const std::exception& e) {
        return OperationResult::fail(ParseError::fromException(e.what()));
    }
}

// ---------------------------------------------------------------------------
// Validation
// ---------------------------------------------------------------------------
OperationResult JSONController::validateJSON()
{
    std::string msg;
    if (!model->validate(msg))
        return OperationResult::fail(msg);
    return OperationResult::ok(msg, model->getWarnings());
}

OperationResult JSONController::validateWithSchema(const JSONSchema& schema)
{
    if (!model->hasDocument())
        return OperationResult::fail("No document loaded");

    std::vector<std::string> errors;
    bool valid = schema.validate(model->getDocument(), errors);

    if (valid) {
        return OperationResult::ok("Schema validation passed");
    } else {
        // Return the first error as the primary message; rest go in warnings.
        ParseError primary(errors.empty() ? "Schema validation failed" : errors.front());
        std::vector<std::string> rest(errors.begin() + (errors.empty() ? 0 : 1), errors.end());
        return OperationResult::fail(primary, rest);
    }
}

// ---------------------------------------------------------------------------
// Schema management
// ---------------------------------------------------------------------------
void JSONController::setActiveSchema(std::shared_ptr<JSONSchema> schema) {
    activeSchema = schema;
}

// ---------------------------------------------------------------------------
// Accessors
// ---------------------------------------------------------------------------
std::shared_ptr<JSONValue> JSONController::getDocument() const {
    return model->getDocument();
}

std::string JSONController::getCurrentFilePath() const {
    return model->getCurrentFilePath();
}

bool JSONController::hasUnsavedChanges() const {
    return model->isModified();
}

bool JSONController::hasDocument() const {
    return model->hasDocument();
}

void JSONController::markAsModified() {
    model->setModified(true);
}

void JSONController::clearDocument() {
    model->clear();
}

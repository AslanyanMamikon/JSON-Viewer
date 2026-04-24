#ifndef JSON_SCHEMA_H_
#define JSON_SCHEMA_H_

#include "Value.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <limits>

enum class JSONType {
    Int,
    Float,
    Bool,
    String,
    Object,
    Array,
    Null,
    Any
};

struct FieldConstraints {
    JSONType type;
    bool required = true;

    // Numeric constraints
    std::optional<double> min;
    std::optional<double> max;

    // String constraints
    std::optional<size_t> minLength;
    std::optional<size_t> maxLength;
    std::optional<std::string> pattern; // For future regex support

    // Array constraints
    std::optional<size_t> minItems;
    std::optional<size_t> maxItems;

    FieldConstraints() = default;
    FieldConstraints(JSONType t) : type(t) {}

};

class JSONSchema {
private:
    std::unordered_map<std::string, FieldConstraints> fields;
    bool allowAdditionalFields;

public:
    JSONSchema(bool allowAdditional = false)
        : allowAdditionalFields(allowAdditional) {
    }

    // Add field constraints
    JSONSchema& addField(const std::string& name, const FieldConstraints& constraints) {
        fields[name] = constraints;
        return *this;
    }

    JSONSchema& addField(const std::string& name, JSONType type, bool required = true) {
        FieldConstraints constraints(type);
        constraints.required = required;
        fields[name] = constraints;
        return *this;
    }

    // Validation
    bool validate(const std::shared_ptr<JSONValue>& value, std::vector<std::string>& errors) const;

    // Convenience method that throws on validation failure
    void validateOrThrow(const std::shared_ptr<JSONValue>& value) const {
        std::vector<std::string> errors;
        if (!validate(value, errors)) {
            std::string errorMsg = "Schema validation failed:\n";
            for (const auto& err : errors) {
                errorMsg += "  - " + err + "\n";
            }
            throw std::runtime_error(errorMsg);
        }
    }

private:
    bool validateField(const std::string& fieldName,
        const std::shared_ptr<JSONValue>& value,
        const FieldConstraints& constraints,
        std::vector<std::string>& errors) const;

    bool checkType(const std::shared_ptr<JSONValue>& value, JSONType expectedType) const;
    std::string typeToString(JSONType type) const;
};

#endif // JSON_SCHEMA_H_

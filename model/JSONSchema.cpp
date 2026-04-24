#include "JSONSchema.h"
#include <sstream>

bool JSONSchema::validate(const std::shared_ptr<JSONValue>& value,
    std::vector<std::string>& errors) const {
    if (!value->isObject()) {
        errors.push_back("Root value must be an object");
        return false;
    }

    const auto& obj = value->asObject();
    bool valid = true;

    // Check required fields and validate present fields
    for (const auto& [fieldName, constraints] : fields) {
        auto it = obj.find(fieldName);

        if (it == obj.end()) {
            if (constraints.required) {
                errors.push_back("Required field '" + fieldName + "' is missing");
                valid = false;
            }
            continue;
        }

        if (!validateField(fieldName, it->second, constraints, errors)) {
            valid = false;
        }
    }

    // Check for additional fields if not allowed
    if (!allowAdditionalFields) {
        for (const auto& [key, val] : obj) {
            if (fields.find(key) == fields.end()) {
                errors.push_back("Additional field '" + key + "' not allowed in schema");
                valid = false;
            }
        }
    }

    return valid;
}

bool JSONSchema::validateField(const std::string& fieldName,
    const std::shared_ptr<JSONValue>& value,
    const FieldConstraints& constraints,
    std::vector<std::string>& errors) const {
    bool valid = true;

    // Type check
    if (constraints.type != JSONType::Any && !checkType(value, constraints.type)) {
        std::ostringstream oss;
        oss << "Field '" << fieldName << "' has wrong type. Expected: "
            << typeToString(constraints.type);
        errors.push_back(oss.str());
        return false;
    }

    // Numeric constraints
    if (constraints.type == JSONType::Int || constraints.type == JSONType::Float) {
        double numValue = 0;
        if (value->isInt()) {
            numValue = static_cast<double>(value->asInt());
        }
        else if (value->isFloat()) {
            numValue = static_cast<double>(value->asFloat());
        }

        if (constraints.min.has_value() && numValue < constraints.min.value()) {
            std::ostringstream oss;
            oss << "Field '" << fieldName << "' value " << numValue
                << " is less than minimum " << constraints.min.value();
            errors.push_back(oss.str());
            valid = false;
        }

        if (constraints.max.has_value() && numValue > constraints.max.value()) {
            std::ostringstream oss;
            oss << "Field '" << fieldName << "' value " << numValue
                << " is greater than maximum " << constraints.max.value();
            errors.push_back(oss.str());
            valid = false;
        }
    }

    // String constraints
    if (constraints.type == JSONType::String && value->isString()) {
        const auto& str = value->asString();

        if (constraints.minLength.has_value() && str.length() < constraints.minLength.value()) {
            std::ostringstream oss;
            oss << "Field '" << fieldName << "' length " << str.length()
                << " is less than minimum length " << constraints.minLength.value();
            errors.push_back(oss.str());
            valid = false;
        }

        if (constraints.maxLength.has_value() && str.length() > constraints.maxLength.value()) {
            std::ostringstream oss;
            oss << "Field '" << fieldName << "' length " << str.length()
                << " is greater than maximum length " << constraints.maxLength.value();
            errors.push_back(oss.str());
            valid = false;
        }
    }

    // Array constraints
    if (constraints.type == JSONType::Array && value->isArray()) {
        size_t arraySize = value->size();

        if (constraints.minItems.has_value() && arraySize < constraints.minItems.value()) {
            std::ostringstream oss;
            oss << "Field '" << fieldName << "' has " << arraySize
                << " items, less than minimum " << constraints.minItems.value();
            errors.push_back(oss.str());
            valid = false;
        }

        if (constraints.maxItems.has_value() && arraySize > constraints.maxItems.value()) {
            std::ostringstream oss;
            oss << "Field '" << fieldName << "' has " << arraySize
                << " items, more than maximum " << constraints.maxItems.value();
            errors.push_back(oss.str());
            valid = false;
        }
    }

    return valid;
}

bool JSONSchema::checkType(const std::shared_ptr<JSONValue>& value, JSONType expectedType) const {
    switch (expectedType) {
    case JSONType::Int: return value->isInt();
    case JSONType::Float: return value->isFloat();
    case JSONType::Bool: return value->isBool();
    case JSONType::String: return value->isString();
    case JSONType::Object: return value->isObject();
    case JSONType::Array: return value->isArray();
    case JSONType::Null: return value->isNull();
    case JSONType::Any: return true;
    default: return false;
    }
}

std::string JSONSchema::typeToString(JSONType type) const {
    switch (type) {
    case JSONType::Int: return "Integer";
    case JSONType::Float: return "Float";
    case JSONType::Bool: return "Boolean";
    case JSONType::String: return "String";
    case JSONType::Object: return "Object";
    case JSONType::Array: return "Array";
    case JSONType::Null: return "Null";
    case JSONType::Any: return "Any";
    default: return "Unknown";
    }
}
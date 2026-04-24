#include "Value.h"

// ---------------------------------------------------------------------------
// Constructors
// ---------------------------------------------------------------------------
JSONValue::JSONValue()                          : value(NullType{}) {}
JSONValue::JSONValue(int v)                     : value(v) {}
JSONValue::JSONValue(float v)                   : value(v) {}
JSONValue::JSONValue(bool v)                    : value(v) {}
JSONValue::JSONValue(const std::string& v)      : value(v) {}
JSONValue::JSONValue(const char* v)             : value(std::string(v)) {}
JSONValue::JSONValue(const JSONObject& v)       : value(v) {}
JSONValue::JSONValue(const JSONArray& v)        : value(v) {}

JSONValue::JSONValue(const JSONValue& other)    : value(other.value) {}
JSONValue::JSONValue(JSONValue&& other) noexcept: value(std::move(other.value)) {}

JSONValue& JSONValue::operator=(const JSONValue& other) {
    if (this != &other) value = other.value;
    return *this;
}
JSONValue& JSONValue::operator=(JSONValue&& other) noexcept {
    if (this != &other) value = std::move(other.value);
    return *this;
}

// ---------------------------------------------------------------------------
// Type checking
// ---------------------------------------------------------------------------
bool JSONValue::isInt()    const { return std::holds_alternative<int>(value); }
bool JSONValue::isFloat()  const { return std::holds_alternative<float>(value); }
bool JSONValue::isBool()   const { return std::holds_alternative<bool>(value); }
bool JSONValue::isString() const { return std::holds_alternative<std::string>(value); }
bool JSONValue::isObject() const { return std::holds_alternative<JSONObject>(value); }
bool JSONValue::isArray()  const { return std::holds_alternative<JSONArray>(value); }
bool JSONValue::isNull()   const { return std::holds_alternative<NullType>(value); }

// ---------------------------------------------------------------------------
// Value retrieval
// ---------------------------------------------------------------------------
int JSONValue::asInt() const {
    if (!isInt()) throw std::runtime_error("Value is not an integer");
    return std::get<int>(value);
}
float JSONValue::asFloat() const {
    if (!isFloat()) throw std::runtime_error("Value is not a float");
    return std::get<float>(value);
}
bool JSONValue::asBool() const {
    if (!isBool()) throw std::runtime_error("Value is not a boolean");
    return std::get<bool>(value);
}
const std::string& JSONValue::asString() const {
    if (!isString()) throw std::runtime_error("Value is not a string");
    return std::get<std::string>(value);
}
const JSONObject& JSONValue::asObject() const {
    if (!isObject()) throw std::runtime_error("Value is not an object");
    return std::get<JSONObject>(value);
}
const JSONArray& JSONValue::asArray() const {
    if (!isArray()) throw std::runtime_error("Value is not an array");
    return std::get<JSONArray>(value);
}

// ---------------------------------------------------------------------------
// Composite helpers
// ---------------------------------------------------------------------------
size_t JSONValue::size() const {
    if (isObject()) return asObject().size();
    if (isArray())  return asArray().size();
    return 0;
}

bool JSONValue::has(const std::string& key) const {
    if (!isObject()) return false;
    return asObject().find(key) != asObject().end();
}

// ---------------------------------------------------------------------------
// print() – plain output, no comment knowledge
// ---------------------------------------------------------------------------
void JSONValue::print(std::ostream& os, int indent) const {
    std::string ind(indent * 2, ' ');
    std::string nextInd((indent + 1) * 2, ' ');

    if (isNull()) {
        os << "null";
    } else if (isInt()) {
        os << asInt();
    } else if (isFloat()) {
        os << asFloat();
    } else if (isBool()) {
        os << (asBool() ? "true" : "false");
    } else if (isString()) {
        os << '"' << asString() << '"';
    } else if (isObject()) {
        const auto& obj = asObject();
        if (obj.empty()) { os << "{}"; return; }
        os << "{\n";
        bool first = true;
        for (const auto& [key, val] : obj) {
            if (!first) os << ",\n";
            os << nextInd << '"' << key << "\": ";
            val->print(os, indent + 1);
            first = false;
        }
        os << "\n" << ind << "}";
    } else if (isArray()) {
        const auto& arr = asArray();
        if (arr.empty()) { os << "[]"; return; }
        os << "[\n";
        for (size_t i = 0; i < arr.size(); ++i) {
            os << nextInd;
            arr[i]->print(os, indent + 1);
            if (i + 1 < arr.size()) os << ",";
            os << "\n";
        }
        os << ind << "]";
    }
}

// ---------------------------------------------------------------------------
// Subscript operators
// ---------------------------------------------------------------------------
std::shared_ptr<JSONValue> JSONValue::operator[](const std::string& key) const {
    if (!isObject()) throw std::runtime_error("Value is not an object");
    const auto& obj = asObject();
    auto it = obj.find(key);
    if (it == obj.end()) throw std::runtime_error("Key not found: " + key);
    return it->second;
}

std::shared_ptr<JSONValue> JSONValue::operator[](size_t index) const {
    if (!isArray()) throw std::runtime_error("Value is not an array");
    const auto& arr = asArray();
    if (index >= arr.size()) throw std::out_of_range("Array index out of bounds");
    return arr[index];
}

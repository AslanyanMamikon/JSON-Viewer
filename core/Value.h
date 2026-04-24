#ifndef JSON_VALUE_H_
#define JSON_VALUE_H_

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <variant>
#include <memory>
#include <stdexcept>

class JSONValue;
using JSONObject = std::map<std::string, std::shared_ptr<JSONValue>>;
using JSONArray  = std::vector<std::shared_ptr<JSONValue>>;
struct NullType  {};

// ---------------------------------------------------------------------------
// JSONValue – pure data node; comments are stored externally in CommentStore
// ---------------------------------------------------------------------------
class JSONValue {
public:
    using ValueType = std::variant<NullType, int, float, bool,
                                   std::string, JSONObject, JSONArray>;

private:
    ValueType value;

public:
    // Constructors
    JSONValue();
    explicit JSONValue(int v);
    explicit JSONValue(float v);
    explicit JSONValue(bool v);
    explicit JSONValue(const std::string& v);
    JSONValue(const char* v);
    explicit JSONValue(const JSONObject& v);
    explicit JSONValue(const JSONArray& v);

    // Copy / move
    JSONValue(const JSONValue& other);
    JSONValue(JSONValue&& other) noexcept;
    JSONValue& operator=(const JSONValue& other);
    JSONValue& operator=(JSONValue&& other) noexcept;

    // Type checking
    bool isInt()    const;
    bool isFloat()  const;
    bool isBool()   const;
    bool isString() const;
    bool isObject() const;
    bool isArray()  const;
    bool isNull()   const;

    // Value retrieval
    int                  asInt()    const;
    float                asFloat()  const;
    bool                 asBool()   const;
    const std::string&   asString() const;
    const JSONObject&    asObject() const;
    const JSONArray&     asArray()  const;

    // Composite helpers
    size_t size() const;                        // objects and arrays
    bool   has(const std::string& key) const;   // objects only

    // Printing (no comment awareness – comments live in CommentStore)
    void print(std::ostream& os, int indent = 0) const;

    std::shared_ptr<JSONValue> operator[](const std::string& key) const;
    std::shared_ptr<JSONValue> operator[](size_t index)           const;
};

inline std::ostream& operator<<(std::ostream& os, const JSONValue& val) {
    val.print(os, 0);
    return os;
}

#endif // JSON_VALUE_H_

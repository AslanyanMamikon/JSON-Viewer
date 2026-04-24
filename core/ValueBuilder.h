#ifndef JSON_VALUE_BUILDER_H_
#define JSON_VALUE_BUILDER_H_

#include <memory>
#include <string>
#include <stack>
#include "Value.h"

// ---------------------------------------------------------------------------
// JSONValueBuilder
//
// Supports arbitrarily nested objects and arrays via an internal scope stack.
// Each scope remembers whether it is an object or an array and holds the
// shared_ptr so that the JSONObject / JSONArray inside it stays alive and
// addressable while children are being added.
//
// Usage example (nested):
//   auto val = JSONValueBuilder{}
//       .startObject()
//           .addProperty("name", "Alice")
//           .addProperty("scores")
//               .startArray()
//                   .addElement(10)
//                   .addElement(20)
//               .endArray()
//       .endObject()
//       .build();
// ---------------------------------------------------------------------------
class JSONValueBuilder {
public:
    JSONValueBuilder();

    // ---- Primitive setters (only valid when no scope is open) ---------------
    JSONValueBuilder& withInt   (int value);
    JSONValueBuilder& withFloat (float value);
    JSONValueBuilder& withBool  (bool value);
    JSONValueBuilder& withString(const std::string& value);
    JSONValueBuilder& withNull  ();

    // ---- Object scope -------------------------------------------------------
    JSONValueBuilder& startObject();
    // Add a pre-built value as a property of the current object scope
    JSONValueBuilder& addProperty(const std::string& key,
                                  std::shared_ptr<JSONValue> value);
    // Convenience overloads
    JSONValueBuilder& addProperty(const std::string& key, int value);
    JSONValueBuilder& addProperty(const std::string& key, float value);
    JSONValueBuilder& addProperty(const std::string& key, bool value);
    JSONValueBuilder& addProperty(const std::string& key,
                                  const std::string& value);
    // Open a nested object/array scope keyed under `key` in the current object
    JSONValueBuilder& startObject(const std::string& key);
    JSONValueBuilder& startArray (const std::string& key);

    JSONValueBuilder& endObject();

    // ---- Array scope --------------------------------------------------------
    JSONValueBuilder& startArray();
    JSONValueBuilder& addElement(std::shared_ptr<JSONValue> value);
    JSONValueBuilder& addElement(int value);
    JSONValueBuilder& addElement(float value);
    JSONValueBuilder& addElement(bool value);
    JSONValueBuilder& addElement(const std::string& value);
    JSONValueBuilder& endArray();

    // ---- Finalise -----------------------------------------------------------
    std::shared_ptr<JSONValue> build();

private:
    enum class ScopeKind { Object, Array };

    struct Scope {
        ScopeKind                  kind;
        std::shared_ptr<JSONValue> node;   // keeps the JSONObject/JSONArray alive
        std::string                parentKey; // set when this scope is a nested child

        Scope(ScopeKind k, std::shared_ptr<JSONValue> n, std::string pk = "")
            : kind(k), node(std::move(n)), parentKey(std::move(pk)) {}
    };

    std::stack<Scope>          scopeStack;
    std::shared_ptr<JSONValue> root;       // set when the outermost scope closes

    // Helper: push a finished child value into the current scope.
    void pushValue(std::shared_ptr<JSONValue> val, const std::string& key = "");

    // Raw accessors into the live containers
    JSONObject& currentObject();
    JSONArray&  currentArray();
};

#endif // JSON_VALUE_BUILDER_H_

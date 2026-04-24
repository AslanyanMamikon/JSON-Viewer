#include "ValueBuilder.h"
#include <stdexcept>

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
JSONValueBuilder::JSONValueBuilder() : root(nullptr) {}

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------
JSONObject& JSONValueBuilder::currentObject() {
    if (scopeStack.empty() || scopeStack.top().kind != ScopeKind::Object)
        throw std::runtime_error("JSONValueBuilder: not currently inside an object scope");
    // asObject() returns a const ref; we need the mutable variant inside the node
    return const_cast<JSONObject&>(scopeStack.top().node->asObject());
}

JSONArray& JSONValueBuilder::currentArray() {
    if (scopeStack.empty() || scopeStack.top().kind != ScopeKind::Array)
        throw std::runtime_error("JSONValueBuilder: not currently inside an array scope");
    return const_cast<JSONArray&>(scopeStack.top().node->asArray());
}

// Push a finished value into whatever scope is currently open.
// `key` is only used when the current scope is an object.
void JSONValueBuilder::pushValue(std::shared_ptr<JSONValue> val,
                                 const std::string& key) {
    if (scopeStack.empty()) {
        // No scope open – this is the root primitive value.
        root = val;
        return;
    }
    auto& top = scopeStack.top();
    if (top.kind == ScopeKind::Object) {
        if (key.empty())
            throw std::runtime_error("JSONValueBuilder: property key must not be empty");
        currentObject()[key] = val;
    } else {
        currentArray().push_back(val);
    }
}

// ---------------------------------------------------------------------------
// Primitive setters (no open scope allowed)
// ---------------------------------------------------------------------------
JSONValueBuilder& JSONValueBuilder::withInt(int value) {
    if (!scopeStack.empty())
        throw std::runtime_error("JSONValueBuilder: withXxx() called inside an open scope; "
                                 "use addProperty/addElement instead");
    root = std::make_shared<JSONValue>(value);
    return *this;
}
JSONValueBuilder& JSONValueBuilder::withFloat(float value) {
    if (!scopeStack.empty())
        throw std::runtime_error("JSONValueBuilder: withXxx() called inside an open scope");
    root = std::make_shared<JSONValue>(value);
    return *this;
}
JSONValueBuilder& JSONValueBuilder::withBool(bool value) {
    if (!scopeStack.empty())
        throw std::runtime_error("JSONValueBuilder: withXxx() called inside an open scope");
    root = std::make_shared<JSONValue>(value);
    return *this;
}
JSONValueBuilder& JSONValueBuilder::withString(const std::string& value) {
    if (!scopeStack.empty())
        throw std::runtime_error("JSONValueBuilder: withXxx() called inside an open scope");
    root = std::make_shared<JSONValue>(value);
    return *this;
}
JSONValueBuilder& JSONValueBuilder::withNull() {
    if (!scopeStack.empty())
        throw std::runtime_error("JSONValueBuilder: withXxx() called inside an open scope");
    root = std::make_shared<JSONValue>();
    return *this;
}

// ---------------------------------------------------------------------------
// Object scope
// ---------------------------------------------------------------------------
JSONValueBuilder& JSONValueBuilder::startObject() {
    auto node = std::make_shared<JSONValue>(JSONObject{});
    scopeStack.emplace(ScopeKind::Object, node);
    return *this;
}

// Open a named object nested inside the current object scope.
JSONValueBuilder& JSONValueBuilder::startObject(const std::string& key) {
    if (scopeStack.empty() || scopeStack.top().kind != ScopeKind::Object)
        throw std::runtime_error("JSONValueBuilder: startObject(key) called outside an object scope");
    auto node = std::make_shared<JSONValue>(JSONObject{});
    scopeStack.emplace(ScopeKind::Object, node, key);
    return *this;
}

// Open an array nested inside the current object scope.
JSONValueBuilder& JSONValueBuilder::startArray(const std::string& key) {
    if (scopeStack.empty() || scopeStack.top().kind != ScopeKind::Object)
        throw std::runtime_error("JSONValueBuilder: startArray(key) called outside an object scope");
    auto node = std::make_shared<JSONValue>(JSONArray{});
    scopeStack.emplace(ScopeKind::Array, node, key);
    return *this;
}

JSONValueBuilder& JSONValueBuilder::addProperty(const std::string& key,
                                                std::shared_ptr<JSONValue> value) {
    currentObject()[key] = value;
    return *this;
}
JSONValueBuilder& JSONValueBuilder::addProperty(const std::string& key, int value) {
    return addProperty(key, std::make_shared<JSONValue>(value));
}
JSONValueBuilder& JSONValueBuilder::addProperty(const std::string& key, float value) {
    return addProperty(key, std::make_shared<JSONValue>(value));
}
JSONValueBuilder& JSONValueBuilder::addProperty(const std::string& key, bool value) {
    return addProperty(key, std::make_shared<JSONValue>(value));
}
JSONValueBuilder& JSONValueBuilder::addProperty(const std::string& key,
                                                const std::string& value) {
    return addProperty(key, std::make_shared<JSONValue>(value));
}

JSONValueBuilder& JSONValueBuilder::endObject() {
    if (scopeStack.empty() || scopeStack.top().kind != ScopeKind::Object)
        throw std::runtime_error("JSONValueBuilder: endObject() without matching startObject()");

    auto finished = scopeStack.top().node;
    auto parentKey = scopeStack.top().parentKey;
    scopeStack.pop();

    if (scopeStack.empty()) {
        root = finished;          // outermost scope → becomes the root
    } else {
        pushValue(finished, parentKey);
    }
    return *this;
}

// ---------------------------------------------------------------------------
// Array scope
// ---------------------------------------------------------------------------
JSONValueBuilder& JSONValueBuilder::startArray() {
    auto node = std::make_shared<JSONValue>(JSONArray{});
    scopeStack.emplace(ScopeKind::Array, node);
    return *this;
}

JSONValueBuilder& JSONValueBuilder::addElement(std::shared_ptr<JSONValue> value) {
    currentArray().push_back(value);
    return *this;
}
JSONValueBuilder& JSONValueBuilder::addElement(int value) {
    return addElement(std::make_shared<JSONValue>(value));
}
JSONValueBuilder& JSONValueBuilder::addElement(float value) {
    return addElement(std::make_shared<JSONValue>(value));
}
JSONValueBuilder& JSONValueBuilder::addElement(bool value) {
    return addElement(std::make_shared<JSONValue>(value));
}
JSONValueBuilder& JSONValueBuilder::addElement(const std::string& value) {
    return addElement(std::make_shared<JSONValue>(value));
}

JSONValueBuilder& JSONValueBuilder::endArray() {
    if (scopeStack.empty() || scopeStack.top().kind != ScopeKind::Array)
        throw std::runtime_error("JSONValueBuilder: endArray() without matching startArray()");

    auto finished = scopeStack.top().node;
    auto parentKey = scopeStack.top().parentKey;
    scopeStack.pop();

    if (scopeStack.empty()) {
        root = finished;
    } else {
        pushValue(finished, parentKey);
    }
    return *this;
}

// ---------------------------------------------------------------------------
// Finalise
// ---------------------------------------------------------------------------
std::shared_ptr<JSONValue> JSONValueBuilder::build() {
    if (!scopeStack.empty())
        throw std::runtime_error("JSONValueBuilder: build() called with unclosed scopes");
    if (!root)
        throw std::runtime_error("JSONValueBuilder: nothing to build");
    auto result = root;
    root = nullptr;
    return result;
}

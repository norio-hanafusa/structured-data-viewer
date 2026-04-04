#include "data_node.h"
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <cmath>

// ---------------------------------------------------------------------------
// Constructors
// ---------------------------------------------------------------------------

DataNode::DataNode() : value(nullptr) {}
DataNode::DataNode(std::nullptr_t) : value(nullptr) {}
DataNode::DataNode(bool v) : value(v) {}
DataNode::DataNode(int64_t v) : value(v) {}
DataNode::DataNode(int v) : value(static_cast<int64_t>(v)) {}
DataNode::DataNode(double v) : value(v) {}
DataNode::DataNode(const std::string& v) : value(v) {}
DataNode::DataNode(const char* v) : value(std::string(v)) {}
DataNode::DataNode(DataArray v) : value(std::move(v)) {}
DataNode::DataNode(DataObject v) : value(std::move(v)) {}

// ---------------------------------------------------------------------------
// Type queries
// ---------------------------------------------------------------------------

NodeType DataNode::type() const {
    static constexpr NodeType table[] = {
        NodeType::Null,     // 0 - nullptr_t
        NodeType::Boolean,  // 1 - bool
        NodeType::Integer,  // 2 - int64_t
        NodeType::Float,    // 3 - double
        NodeType::String,   // 4 - string
        NodeType::Array,    // 5 - DataArray
        NodeType::Object    // 6 - DataObject
    };
    return table[value.index()];
}

const char* DataNode::typeName() const {
    switch (type()) {
        case NodeType::Null:    return "null";
        case NodeType::Boolean: return "boolean";
        case NodeType::Integer: return "number";
        case NodeType::Float:   return "number";
        case NodeType::String:  return "string";
        case NodeType::Array:   return "array";
        case NodeType::Object:  return "object";
    }
    return "unknown";
}

bool DataNode::isNull() const {
    return type() == NodeType::Null;
}

bool DataNode::isContainer() const {
    auto t = type();
    return t == NodeType::Array || t == NodeType::Object;
}

// ---------------------------------------------------------------------------
// Accessors
// ---------------------------------------------------------------------------

#define ACCESSOR_IMPL(Type, Name, Index, Label)                               \
    Type& DataNode::Name() {                                                  \
        if (auto* p = std::get_if<Index>(&value)) return *p;                  \
        throw std::runtime_error("DataNode is not " Label);                   \
    }                                                                         \
    const Type& DataNode::Name() const {                                      \
        if (auto* p = std::get_if<Index>(&value)) return *p;                  \
        throw std::runtime_error("DataNode is not " Label);                   \
    }

ACCESSOR_IMPL(bool,        asBool,   1, "a boolean")
ACCESSOR_IMPL(int64_t,     asInt,    2, "an integer")
ACCESSOR_IMPL(double,      asFloat,  3, "a float")
ACCESSOR_IMPL(std::string, asString, 4, "a string")
ACCESSOR_IMPL(DataArray,   asArray,  5, "an array")
ACCESSOR_IMPL(DataObject,  asObject, 6, "an object")

#undef ACCESSOR_IMPL

// ---------------------------------------------------------------------------
// Display helpers
// ---------------------------------------------------------------------------

std::string DataNode::displayValue() const {
    switch (type()) {
        case NodeType::Null:
            return "null";
        case NodeType::Boolean:
            return asBool() ? "true" : "false";
        case NodeType::Integer:
            return std::to_string(asInt());
        case NodeType::Float: {
            // Avoid trailing zeroes for whole numbers but keep decimal precision
            double d = asFloat();
            if (std::isnan(d)) return "NaN";
            if (std::isinf(d)) return d > 0 ? "Infinity" : "-Infinity";
            std::ostringstream oss;
            oss << d;
            return oss.str();
        }
        case NodeType::String:
            return asString();
        case NodeType::Array: {
            std::string s = "[";
            s += std::to_string(asArray().size());
            s += "]";
            return s;
        }
        case NodeType::Object: {
            std::string s = "{";
            s += std::to_string(asObject().size());
            s += "}";
            return s;
        }
    }
    return "";
}

// ---------------------------------------------------------------------------
// Child count
// ---------------------------------------------------------------------------

size_t DataNode::childCount() const {
    switch (type()) {
        case NodeType::Array:  return asArray().size();
        case NodeType::Object: return asObject().size();
        default:               return 0;
    }
}

// ---------------------------------------------------------------------------
// Deep clone
// ---------------------------------------------------------------------------

DataNode DataNode::clone() const {
    return std::visit([](const auto& v) -> DataNode {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, std::nullptr_t>) {
            return DataNode(nullptr);
        } else if constexpr (std::is_same_v<T, DataArray>) {
            DataArray arr;
            arr.reserve(v.size());
            for (const auto& elem : v) {
                arr.push_back(elem.clone());
            }
            return DataNode(std::move(arr));
        } else if constexpr (std::is_same_v<T, DataObject>) {
            DataObject obj;
            obj.reserve(v.size());
            for (const auto& [key, node] : v) {
                obj.emplace_back(key, node.clone());
            }
            return DataNode(std::move(obj));
        } else {
            return DataNode(v);
        }
    }, value);
}

// ---------------------------------------------------------------------------
// Path-based access
// ---------------------------------------------------------------------------

DataNode* DataNode::getAt(const Path& path) {
    DataNode* current = this;
    for (const auto& seg : path) {
        if (!current) return nullptr;
        if (auto* s = std::get_if<std::string>(&seg)) {
            if (current->type() != NodeType::Object) return nullptr;
            current = current->findKey(*s);
        } else {
            size_t idx = std::get<size_t>(seg);
            if (current->type() != NodeType::Array) return nullptr;
            auto& arr = current->asArray();
            if (idx >= arr.size()) return nullptr;
            current = &arr[idx];
        }
    }
    return current;
}

const DataNode* DataNode::getAt(const Path& path) const {
    const DataNode* current = this;
    for (const auto& seg : path) {
        if (!current) return nullptr;
        if (auto* s = std::get_if<std::string>(&seg)) {
            if (current->type() != NodeType::Object) return nullptr;
            const auto& obj = current->asObject();
            auto it = std::find_if(obj.begin(), obj.end(),
                [&](const auto& p) { return p.first == *s; });
            if (it == obj.end()) return nullptr;
            current = &it->second;
        } else {
            size_t idx = std::get<size_t>(seg);
            if (current->type() != NodeType::Array) return nullptr;
            const auto& arr = current->asArray();
            if (idx >= arr.size()) return nullptr;
            current = &arr[idx];
        }
    }
    return current;
}

void DataNode::setAt(const Path& path, DataNode val) {
    if (path.empty()) {
        value = std::move(val.value);
        return;
    }

    // Navigate to the parent
    Path parentPath(path.begin(), path.end() - 1);
    DataNode* parent = getAt(parentPath);
    if (!parent) {
        throw std::runtime_error("setAt: parent path does not exist");
    }

    const auto& lastSeg = path.back();
    if (auto* s = std::get_if<std::string>(&lastSeg)) {
        if (parent->type() != NodeType::Object) {
            throw std::runtime_error("setAt: parent is not an object");
        }
        auto& obj = parent->asObject();
        auto it = std::find_if(obj.begin(), obj.end(),
            [&](const auto& p) { return p.first == *s; });
        if (it != obj.end()) {
            it->second = std::move(val);
        } else {
            obj.emplace_back(*s, std::move(val));
        }
    } else {
        size_t idx = std::get<size_t>(lastSeg);
        if (parent->type() != NodeType::Array) {
            throw std::runtime_error("setAt: parent is not an array");
        }
        auto& arr = parent->asArray();
        if (idx >= arr.size()) {
            throw std::runtime_error("setAt: index out of range");
        }
        arr[idx] = std::move(val);
    }
}

void DataNode::removeAt(const Path& path) {
    if (path.empty()) {
        throw std::runtime_error("removeAt: cannot remove root");
    }

    Path parentPath(path.begin(), path.end() - 1);
    DataNode* parent = getAt(parentPath);
    if (!parent) {
        throw std::runtime_error("removeAt: parent path does not exist");
    }

    const auto& lastSeg = path.back();
    if (auto* s = std::get_if<std::string>(&lastSeg)) {
        if (parent->type() != NodeType::Object) {
            throw std::runtime_error("removeAt: parent is not an object");
        }
        auto& obj = parent->asObject();
        auto it = std::find_if(obj.begin(), obj.end(),
            [&](const auto& p) { return p.first == *s; });
        if (it != obj.end()) {
            obj.erase(it);
        }
    } else {
        size_t idx = std::get<size_t>(lastSeg);
        if (parent->type() != NodeType::Array) {
            throw std::runtime_error("removeAt: parent is not an array");
        }
        auto& arr = parent->asArray();
        if (idx < arr.size()) {
            arr.erase(arr.begin() + static_cast<ptrdiff_t>(idx));
        }
    }
}

void DataNode::insertAt(const Path& path, DataNode val, const std::string& key) {
    DataNode* target = getAt(path);
    if (!target) {
        throw std::runtime_error("insertAt: target path does not exist");
    }

    if (target->type() == NodeType::Array) {
        target->asArray().push_back(std::move(val));
    } else if (target->type() == NodeType::Object) {
        target->asObject().emplace_back(key, std::move(val));
    } else {
        throw std::runtime_error("insertAt: target is not a container");
    }
}

// ---------------------------------------------------------------------------
// Object key helpers
// ---------------------------------------------------------------------------

DataNode* DataNode::findKey(const std::string& key) {
    if (type() != NodeType::Object) return nullptr;
    auto& obj = asObject();
    auto it = std::find_if(obj.begin(), obj.end(),
        [&](const auto& p) { return p.first == key; });
    return it != obj.end() ? &it->second : nullptr;
}

bool DataNode::hasKey(const std::string& key) const {
    if (type() != NodeType::Object) return false;
    const auto& obj = asObject();
    return std::any_of(obj.begin(), obj.end(),
        [&](const auto& p) { return p.first == key; });
}

// ---------------------------------------------------------------------------
// Path utilities
// ---------------------------------------------------------------------------

std::string pathToString(const DataNode::Path& path) {
    if (path.empty()) return "";

    std::string result;
    for (size_t i = 0; i < path.size(); ++i) {
        const auto& seg = path[i];
        if (auto* s = std::get_if<std::string>(&seg)) {
            if (i > 0) result += '.';
            result += *s;
        } else {
            result += '[';
            result += std::to_string(std::get<size_t>(seg));
            result += ']';
        }
    }
    return result;
}

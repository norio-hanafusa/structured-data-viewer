#pragma once
#include <string>
#include <vector>
#include <variant>
#include <cstdint>
#include <optional>
#include <functional>

// Forward declaration
struct DataNode;

using DataArray  = std::vector<DataNode>;
using DataObject = std::vector<std::pair<std::string, DataNode>>;

enum class NodeType { Null, Boolean, Integer, Float, String, Array, Object };

struct DataNode {
    using Value = std::variant<
        std::nullptr_t,   // Null
        bool,             // Boolean
        int64_t,          // Integer
        double,           // Float
        std::string,      // String
        DataArray,        // Array
        DataObject        // Object
    >;

    Value value;

    // Constructors
    DataNode();                           // Null
    DataNode(std::nullptr_t);
    DataNode(bool v);
    DataNode(int64_t v);
    DataNode(int v);
    DataNode(double v);
    DataNode(const std::string& v);
    DataNode(const char* v);
    DataNode(DataArray v);
    DataNode(DataObject v);

    // Type queries
    NodeType type() const;
    const char* typeName() const;        // "string", "number", "boolean", "null", "array", "object"
    bool isNull() const;
    bool isContainer() const;            // array or object

    // Accessors (throw on wrong type)
    bool& asBool();
    int64_t& asInt();
    double& asFloat();
    std::string& asString();
    DataArray& asArray();
    DataObject& asObject();
    const bool& asBool() const;
    const int64_t& asInt() const;
    const double& asFloat() const;
    const std::string& asString() const;
    const DataArray& asArray() const;
    const DataObject& asObject() const;

    // Display value as string
    std::string displayValue() const;

    // Child count (0 for leaf)
    size_t childCount() const;

    // Deep clone
    DataNode clone() const;

    // Path-based access
    using PathSegment = std::variant<std::string, size_t>;
    using Path = std::vector<PathSegment>;

    DataNode* getAt(const Path& path);
    const DataNode* getAt(const Path& path) const;
    void setAt(const Path& path, DataNode value);
    void removeAt(const Path& path);
    void insertAt(const Path& path, DataNode value, const std::string& key = "");

    // Object key helpers
    DataNode* findKey(const std::string& key);
    bool hasKey(const std::string& key) const;
};

// Path utilities
std::string pathToString(const DataNode::Path& path);

#include "data/yaml_parser.h"
#include <yaml-cpp/yaml.h>
#include <sstream>
#include <stdexcept>
#include <cstdlib>
#include <cerrno>
#include <climits>

namespace parsers {

// ---------------------------------------------------------------------------
// Auto-type a scalar string: try int64, double, bool, null, else string
// ---------------------------------------------------------------------------
static DataNode autoTypeScalar(const std::string& s) {
    // Null
    if (s == "null" || s == "Null" || s == "NULL" || s == "~" || s.empty()) {
        return DataNode(nullptr);
    }

    // Boolean
    if (s == "true" || s == "True" || s == "TRUE") {
        return DataNode(true);
    }
    if (s == "false" || s == "False" || s == "FALSE") {
        return DataNode(false);
    }

    // Integer (try first)
    {
        char* end = nullptr;
        errno = 0;
        long long val = std::strtoll(s.c_str(), &end, 10);
        if (errno == 0 && end != s.c_str() && *end == '\0') {
            return DataNode(static_cast<int64_t>(val));
        }
    }

    // Float
    {
        char* end = nullptr;
        errno = 0;
        double val = std::strtod(s.c_str(), &end);
        if (errno == 0 && end != s.c_str() && *end == '\0') {
            return DataNode(val);
        }
    }

    // String fallback
    return DataNode(s);
}

// ---------------------------------------------------------------------------
// YAML::Node -> DataNode
// ---------------------------------------------------------------------------
static DataNode yamlToDataNode(const YAML::Node& node) {
    switch (node.Type()) {
        case YAML::NodeType::Null:
        case YAML::NodeType::Undefined:
            return DataNode(nullptr);

        case YAML::NodeType::Scalar:
            return autoTypeScalar(node.Scalar());

        case YAML::NodeType::Sequence: {
            DataArray arr;
            arr.reserve(node.size());
            for (size_t i = 0; i < node.size(); ++i) {
                arr.push_back(yamlToDataNode(node[i]));
            }
            return DataNode(std::move(arr));
        }

        case YAML::NodeType::Map: {
            DataObject obj;
            obj.reserve(node.size());
            for (auto it = node.begin(); it != node.end(); ++it) {
                std::string key = it->first.as<std::string>();
                obj.emplace_back(std::move(key), yamlToDataNode(it->second));
            }
            return DataNode(std::move(obj));
        }
    }
    return DataNode(nullptr);
}

// ---------------------------------------------------------------------------
// DataNode -> YAML::Emitter
// ---------------------------------------------------------------------------
static void emitDataNode(YAML::Emitter& out, const DataNode& node) {
    switch (node.type()) {
        case NodeType::Null:
            out << YAML::Null;
            break;
        case NodeType::Boolean:
            out << node.asBool();
            break;
        case NodeType::Integer:
            out << node.asInt();
            break;
        case NodeType::Float:
            out << node.asFloat();
            break;
        case NodeType::String:
            out << node.asString();
            break;
        case NodeType::Array:
            out << YAML::BeginSeq;
            for (const auto& elem : node.asArray()) {
                emitDataNode(out, elem);
            }
            out << YAML::EndSeq;
            break;
        case NodeType::Object:
            out << YAML::BeginMap;
            for (const auto& [key, val] : node.asObject()) {
                out << YAML::Key << key;
                out << YAML::Value;
                emitDataNode(out, val);
            }
            out << YAML::EndMap;
            break;
    }
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

DataNode parseYAML(const std::string& input) {
    YAML::Node root = YAML::Load(input);
    return yamlToDataNode(root);
}

std::string serializeYAML(const DataNode& node) {
    YAML::Emitter out;
    emitDataNode(out, node);
    return std::string(out.c_str());
}

std::string formatYAML(const std::string& input) {
    DataNode node = parseYAML(input);
    return serializeYAML(node);
}

} // namespace parsers

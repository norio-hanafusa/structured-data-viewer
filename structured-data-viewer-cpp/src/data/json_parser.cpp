#include "data/json_parser.h"
#include <nlohmann/json.hpp>
#include <sstream>
#include <stdexcept>

using json = nlohmann::json;

namespace parsers {

// ---------------------------------------------------------------------------
// Internal: nlohmann::json <-> DataNode conversion
// ---------------------------------------------------------------------------

static DataNode jsonToDataNode(const json& j) {
    switch (j.type()) {
        case json::value_t::null:
            return DataNode(nullptr);
        case json::value_t::boolean:
            return DataNode(j.get<bool>());
        case json::value_t::number_integer:
            return DataNode(j.get<int64_t>());
        case json::value_t::number_unsigned:
            return DataNode(static_cast<int64_t>(j.get<uint64_t>()));
        case json::value_t::number_float:
            return DataNode(j.get<double>());
        case json::value_t::string:
            return DataNode(j.get<std::string>());
        case json::value_t::array: {
            DataArray arr;
            arr.reserve(j.size());
            for (const auto& elem : j) {
                arr.push_back(jsonToDataNode(elem));
            }
            return DataNode(std::move(arr));
        }
        case json::value_t::object: {
            DataObject obj;
            obj.reserve(j.size());
            for (auto it = j.begin(); it != j.end(); ++it) {
                obj.emplace_back(it.key(), jsonToDataNode(it.value()));
            }
            return DataNode(std::move(obj));
        }
        default:
            return DataNode(nullptr);
    }
}

static json dataNodeToJson(const DataNode& node) {
    switch (node.type()) {
        case NodeType::Null:
            return json(nullptr);
        case NodeType::Boolean:
            return json(node.asBool());
        case NodeType::Integer:
            return json(node.asInt());
        case NodeType::Float:
            return json(node.asFloat());
        case NodeType::String:
            return json(node.asString());
        case NodeType::Array: {
            json arr = json::array();
            for (const auto& elem : node.asArray()) {
                arr.push_back(dataNodeToJson(elem));
            }
            return arr;
        }
        case NodeType::Object: {
            json obj = json::object();
            for (const auto& [key, val] : node.asObject()) {
                obj[key] = dataNodeToJson(val);
            }
            return obj;
        }
    }
    return json(nullptr);
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

DataNode parseJSON(const std::string& input) {
    json j = json::parse(input);
    return jsonToDataNode(j);
}

DataNode parseJSONL(const std::string& input) {
    DataArray results;
    std::istringstream stream(input);
    std::string line;
    int lineNum = 0;

    while (std::getline(stream, line)) {
        ++lineNum;
        // Trim whitespace
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue; // skip empty lines
        std::string trimmed = line.substr(start);
        size_t end = trimmed.find_last_not_of(" \t\r\n");
        if (end != std::string::npos) {
            trimmed = trimmed.substr(0, end + 1);
        }
        if (trimmed.empty()) continue;

        try {
            json j = json::parse(trimmed);
            results.push_back(jsonToDataNode(j));
        } catch (const json::parse_error& e) {
            throw std::runtime_error(
                "JSONL parse error on line " + std::to_string(lineNum) + ": " + e.what());
        }
    }

    return DataNode(std::move(results));
}

std::string serializeJSON(const DataNode& node, int indent) {
    json j = dataNodeToJson(node);
    return j.dump(indent);
}

std::string serializeJSONL(const DataNode& node) {
    if (node.type() != NodeType::Array) {
        // If not an array, just serialize as single-line JSON
        json j = dataNodeToJson(node);
        return j.dump(-1);
    }

    std::string result;
    const auto& arr = node.asArray();
    for (size_t i = 0; i < arr.size(); ++i) {
        json j = dataNodeToJson(arr[i]);
        result += j.dump(-1);
        if (i + 1 < arr.size()) {
            result += '\n';
        }
    }
    return result;
}

std::string formatJSON(const std::string& input) {
    json j = json::parse(input);
    return j.dump(2);
}

std::string formatJSONL(const std::string& input) {
    DataNode node = parseJSONL(input);
    return serializeJSONL(node);
}

} // namespace parsers

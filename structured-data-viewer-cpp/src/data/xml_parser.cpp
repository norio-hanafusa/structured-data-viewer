#include "data/xml_parser.h"
#include <pugixml.hpp>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <unordered_map>
#include <cstdlib>
#include <cerrno>

namespace parsers {

// ---------------------------------------------------------------------------
// Auto-type for attribute/text values
// ---------------------------------------------------------------------------
static DataNode autoType(const std::string& s) {
    if (s == "true") return DataNode(true);
    if (s == "false") return DataNode(false);
    if (s == "null") return DataNode(nullptr);

    // Try integer
    {
        char* end = nullptr;
        errno = 0;
        long long val = std::strtoll(s.c_str(), &end, 10);
        if (errno == 0 && end != s.c_str() && *end == '\0') {
            return DataNode(static_cast<int64_t>(val));
        }
    }

    // Try double
    {
        char* end = nullptr;
        errno = 0;
        double val = std::strtod(s.c_str(), &end);
        if (errno == 0 && end != s.c_str() && *end == '\0') {
            return DataNode(val);
        }
    }

    return DataNode(s);
}

// ---------------------------------------------------------------------------
// pugixml node -> DataNode  (ports JS xmlNodeToObj)
// ---------------------------------------------------------------------------
static DataNode xmlNodeToDataNode(const pugi::xml_node& node) {
    DataObject obj;

    // Collect attributes into @attributes
    if (node.first_attribute()) {
        DataObject attrs;
        for (pugi::xml_attribute attr = node.first_attribute(); attr; attr = attr.next_attribute()) {
            attrs.emplace_back(attr.name(), autoType(attr.value()));
        }
        obj.emplace_back("@attributes", DataNode(std::move(attrs)));
    }

    // Count child element names to detect arrays (multiple same-name children)
    std::unordered_map<std::string, int> childNameCount;
    for (pugi::xml_node child = node.first_child(); child; child = child.next_sibling()) {
        if (child.type() == pugi::node_element) {
            childNameCount[child.name()]++;
        }
    }

    // Collect text content
    std::string textContent;
    for (pugi::xml_node child = node.first_child(); child; child = child.next_sibling()) {
        if (child.type() == pugi::node_pcdata || child.type() == pugi::node_cdata) {
            textContent += child.value();
        }
    }

    // If this is a leaf-only node (no child elements), return text directly
    bool hasChildElements = false;
    for (pugi::xml_node child = node.first_child(); child; child = child.next_sibling()) {
        if (child.type() == pugi::node_element) {
            hasChildElements = true;
            break;
        }
    }

    if (!hasChildElements && obj.empty()) {
        // No attributes, no child elements -> just the text value
        return autoType(textContent);
    }

    if (!hasChildElements) {
        // Has attributes but no child elements -> add #text
        if (!textContent.empty()) {
            obj.emplace_back("#text", autoType(textContent));
        }
        return DataNode(std::move(obj));
    }

    // Track which child names we've already added as arrays
    std::unordered_map<std::string, DataArray*> arrayMap;

    for (pugi::xml_node child = node.first_child(); child; child = child.next_sibling()) {
        if (child.type() == pugi::node_element) {
            std::string name = child.name();
            DataNode childNode = xmlNodeToDataNode(child);

            if (childNameCount[name] > 1) {
                // Multiple children with the same name -> array
                auto it = arrayMap.find(name);
                if (it == arrayMap.end()) {
                    // First occurrence: create array entry
                    obj.emplace_back(name, DataNode(DataArray{}));
                    // Get pointer to the array we just added
                    DataArray& arr = obj.back().second.asArray();
                    arr.push_back(std::move(childNode));
                    arrayMap[name] = &arr;
                } else {
                    it->second->push_back(std::move(childNode));
                }
            } else {
                obj.emplace_back(name, std::move(childNode));
            }
        } else if (child.type() == pugi::node_pcdata || child.type() == pugi::node_cdata) {
            std::string text = child.value();
            // Trim
            size_t s = text.find_first_not_of(" \t\r\n");
            if (s != std::string::npos) {
                size_t e = text.find_last_not_of(" \t\r\n");
                text = text.substr(s, e - s + 1);
                if (!text.empty()) {
                    obj.emplace_back("#text", autoType(text));
                }
            }
        }
    }

    return DataNode(std::move(obj));
}

// ---------------------------------------------------------------------------
// DataNode -> XML serialization
// ---------------------------------------------------------------------------
static void serializeElement(std::ostream& out, const std::string& tagName,
                             const DataNode& node, int indent, int depth) {
    std::string pad(depth * indent, ' ');

    if (node.type() != NodeType::Object) {
        // Simple value: <tag>value</tag>
        out << pad << "<" << tagName << ">" << node.displayValue()
            << "</" << tagName << ">\n";
        return;
    }

    const auto& obj = node.asObject();

    // Find @attributes
    const DataNode* attrsNode = nullptr;
    for (const auto& [k, v] : obj) {
        if (k == "@attributes") {
            attrsNode = &v;
            break;
        }
    }

    // Open tag
    out << pad << "<" << tagName;
    if (attrsNode && attrsNode->type() == NodeType::Object) {
        for (const auto& [ak, av] : attrsNode->asObject()) {
            out << " " << ak << "=\"" << av.displayValue() << "\"";
        }
    }

    // Check if there's only #text (and possibly @attributes)
    bool hasOnlyText = true;
    std::string textValue;
    for (const auto& [k, v] : obj) {
        if (k == "@attributes") continue;
        if (k == "#text") {
            textValue = v.displayValue();
            continue;
        }
        hasOnlyText = false;
        break;
    }

    if (hasOnlyText && !textValue.empty()) {
        out << ">" << textValue << "</" << tagName << ">\n";
        return;
    }

    // Check if node is empty (only @attributes or nothing)
    bool hasChildren = false;
    for (const auto& [k, v] : obj) {
        if (k != "@attributes") {
            hasChildren = true;
            break;
        }
    }

    if (!hasChildren) {
        out << " />\n";
        return;
    }

    out << ">\n";

    // Serialize children
    for (const auto& [k, v] : obj) {
        if (k == "@attributes") continue;
        if (k == "#text") {
            out << std::string((depth + 1) * indent, ' ') << v.displayValue() << "\n";
            continue;
        }

        if (v.type() == NodeType::Array) {
            // Array: repeated elements
            for (const auto& elem : v.asArray()) {
                serializeElement(out, k, elem, indent, depth + 1);
            }
        } else {
            serializeElement(out, k, v, indent, depth + 1);
        }
    }

    out << pad << "</" << tagName << ">\n";
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

DataNode parseXML(const std::string& input) {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_string(input.c_str(),
        pugi::parse_default | pugi::parse_declaration);

    if (!result) {
        throw std::runtime_error(std::string("XML parse error: ") + result.description());
    }

    // Find the root element
    pugi::xml_node root = doc.first_child();
    // Skip declaration nodes
    while (root && root.type() != pugi::node_element) {
        root = root.next_sibling();
    }

    if (!root) {
        throw std::runtime_error("XML document has no root element");
    }

    DataObject topLevel;
    topLevel.emplace_back(root.name(), xmlNodeToDataNode(root));
    return DataNode(std::move(topLevel));
}

std::string serializeXML(const DataNode& node, int indent) {
    std::ostringstream out;
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";

    if (node.type() == NodeType::Object) {
        const auto& obj = node.asObject();
        for (const auto& [key, val] : obj) {
            serializeElement(out, key, val, indent, 0);
        }
    }

    return out.str();
}

std::string formatXML(const std::string& input) {
    DataNode node = parseXML(input);
    return serializeXML(node);
}

} // namespace parsers

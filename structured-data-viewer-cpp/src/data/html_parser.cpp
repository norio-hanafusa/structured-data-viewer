#include "data/html_parser.h"
#include <pugixml.hpp>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <unordered_map>
#include <cctype>

namespace parsers {

// ---------------------------------------------------------------------------
// pugixml node -> DataNode  (ports JS htmlNodeToObj)
// Uses pugixml to parse HTML as XML (works for well-formed HTML/XHTML).
// ---------------------------------------------------------------------------
static DataNode htmlNodeToDataNode(pugi::xml_node node) {
    DataObject obj;

    // Collect attributes into @attributes
    if (node.first_attribute()) {
        DataObject attrs;
        for (auto attr = node.first_attribute(); attr; attr = attr.next_attribute()) {
            attrs.emplace_back(std::string(attr.name()), DataNode(std::string(attr.value())));
        }
        obj.emplace_back("@attributes", DataNode(std::move(attrs)));
    }

    // Count child element names to detect arrays
    std::unordered_map<std::string, int> childNameCount;
    for (auto child = node.first_child(); child; child = child.next_sibling()) {
        if (child.type() == pugi::node_element) {
            std::string name = child.name();
            std::transform(name.begin(), name.end(), name.begin(),
                [](unsigned char c) { return std::tolower(c); });
            childNameCount[name]++;
        }
    }

    // Track array children already started
    std::unordered_map<std::string, DataArray*> arrayMap;
    std::string textContent;

    for (auto child = node.first_child(); child; child = child.next_sibling()) {
        if (child.type() == pugi::node_pcdata || child.type() == pugi::node_cdata) {
            std::string text = child.value();
            // Trim
            size_t start = text.find_first_not_of(" \t\r\n");
            if (start != std::string::npos) {
                size_t end = text.find_last_not_of(" \t\r\n");
                textContent += text.substr(start, end - start + 1);
            }
            continue;
        }

        if (child.type() == pugi::node_element) {
            std::string name = child.name();
            std::transform(name.begin(), name.end(), name.begin(),
                [](unsigned char c) { return std::tolower(c); });

            DataNode childNode = htmlNodeToDataNode(child);

            if (childNameCount[name] > 1) {
                auto it = arrayMap.find(name);
                if (it == arrayMap.end()) {
                    obj.emplace_back(name, DataNode(DataArray{}));
                    DataArray& arr = obj.back().second.asArray();
                    arr.push_back(std::move(childNode));
                    arrayMap[name] = &arr;
                } else {
                    it->second->push_back(std::move(childNode));
                }
            } else {
                obj.emplace_back(name, std::move(childNode));
            }
        }
    }

    // Add text content
    if (!textContent.empty()) {
        bool hasChildElements = false;
        for (const auto& [k, v] : obj) {
            if (k != "@attributes") {
                hasChildElements = true;
                break;
            }
        }
        if (!hasChildElements && obj.empty()) {
            return DataNode(textContent);
        }
        obj.emplace_back("#text", DataNode(textContent));
    }

    return DataNode(std::move(obj));
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

DataNode parseHTML(const std::string& input) {
    pugi::xml_document doc;
    // Parse as HTML (lenient mode)
    auto result = doc.load_string(input.c_str(),
        pugi::parse_default | pugi::parse_ws_pcdata_single | pugi::parse_trim_pcdata);

    if (!result) {
        // Retry wrapping in a root element
        std::string wrapped = "<html_root>" + input + "</html_root>";
        result = doc.load_string(wrapped.c_str(),
            pugi::parse_default | pugi::parse_ws_pcdata_single | pugi::parse_trim_pcdata);
        if (!result) {
            throw std::runtime_error(std::string("HTML parse error: ") + result.description());
        }
        // Unwrap: use the children of html_root
        auto root = doc.child("html_root");
        DataNode inner = htmlNodeToDataNode(root);
        DataObject topLevel;
        topLevel.emplace_back("html", std::move(inner));
        return DataNode(std::move(topLevel));
    }

    // Find the root element
    auto root = doc.first_child();
    if (!root) {
        throw std::runtime_error("Empty HTML document");
    }

    DataNode rootNode = htmlNodeToDataNode(root);

    // Wrap in {"html": ...} if root tag is html
    std::string rootName = root.name();
    std::transform(rootName.begin(), rootName.end(), rootName.begin(),
        [](unsigned char c) { return std::tolower(c); });

    if (rootName == "html") {
        DataObject topLevel;
        topLevel.emplace_back("html", std::move(rootNode));
        return DataNode(std::move(topLevel));
    }

    return rootNode;
}

std::string formatHTML(const std::string& input) {
    std::string result;
    std::string cleaned;
    cleaned.reserve(input.size());

    // Remove whitespace between tags
    bool inTag = false;
    bool lastWasClose = false;
    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '<') {
            inTag = true;
            cleaned += '<';
        } else if (input[i] == '>') {
            inTag = false;
            lastWasClose = true;
            cleaned += '>';
        } else if (!inTag && lastWasClose) {
            // Skip whitespace between tags
            if (input[i] == ' ' || input[i] == '\t' || input[i] == '\r' || input[i] == '\n') {
                // Check if next non-whitespace is a tag
                size_t j = i;
                while (j < input.size() && (input[j] == ' ' || input[j] == '\t' || input[j] == '\r' || input[j] == '\n')) j++;
                if (j < input.size() && input[j] == '<') {
                    i = j - 1;
                    continue;
                }
            }
            lastWasClose = false;
            cleaned += input[i];
        } else {
            cleaned += input[i];
        }
    }

    int indent = 0;
    const int indentSize = 2;
    size_t i = 0;

    while (i < cleaned.size()) {
        if (cleaned[i] == '<') {
            size_t tagEnd = cleaned.find('>', i);
            if (tagEnd == std::string::npos) {
                result += cleaned.substr(i);
                break;
            }

            std::string tag = cleaned.substr(i, tagEnd - i + 1);

            bool isClosing = (tag.size() > 1 && tag[1] == '/');
            bool isSelfClosing = (tag.size() > 1 && tag[tag.size() - 2] == '/');
            bool isVoid = false;
            {
                static const char* voids[] = {
                    "area", "base", "br", "col", "embed", "hr", "img", "input",
                    "link", "meta", "param", "source", "track", "wbr", nullptr
                };
                size_t nameStart = isClosing ? 2 : 1;
                size_t nameEnd = tag.find_first_of(" \t\r\n/>", nameStart);
                if (nameEnd == std::string::npos) nameEnd = tag.size();
                std::string tagName = tag.substr(nameStart, nameEnd - nameStart);
                std::transform(tagName.begin(), tagName.end(), tagName.begin(),
                    [](unsigned char c) { return std::tolower(c); });
                for (const char** v = voids; *v; ++v) {
                    if (tagName == *v) { isVoid = true; break; }
                }
            }

            bool isDeclaration = (tag.size() > 1 && tag[1] == '!');
            bool isPI = (tag.size() > 1 && tag[1] == '?');

            if (isClosing) {
                indent = std::max(0, indent - 1);
                result += std::string(indent * indentSize, ' ') + tag + "\n";
            } else if (isSelfClosing || isVoid || isDeclaration || isPI) {
                result += std::string(indent * indentSize, ' ') + tag + "\n";
            } else {
                result += std::string(indent * indentSize, ' ') + tag + "\n";
                indent++;
            }

            i = tagEnd + 1;
        } else {
            size_t textEnd = cleaned.find('<', i);
            if (textEnd == std::string::npos) textEnd = cleaned.size();
            std::string text = cleaned.substr(i, textEnd - i);

            size_t s = text.find_first_not_of(" \t\r\n");
            if (s != std::string::npos) {
                size_t e = text.find_last_not_of(" \t\r\n");
                text = text.substr(s, e - s + 1);
                if (!text.empty()) {
                    result += std::string(indent * indentSize, ' ') + text + "\n";
                }
            }

            i = textEnd;
        }
    }

    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }

    return result;
}

} // namespace parsers

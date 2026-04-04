#include "data/format_detect.h"
#include <algorithm>
#include <sstream>
#include <cctype>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
static std::string toLower(const std::string& s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return out;
}

static std::string trimLeft(const std::string& s) {
    size_t pos = s.find_first_not_of(" \t\r\n");
    if (pos == std::string::npos) return "";
    return s.substr(pos);
}

static bool startsWith(const std::string& s, const std::string& prefix) {
    return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

static std::string getExtension(const std::string& filename) {
    size_t dot = filename.rfind('.');
    if (dot == std::string::npos) return "";
    return toLower(filename.substr(dot));
}

// ---------------------------------------------------------------------------
// Format detection from content
// ---------------------------------------------------------------------------
Format detectFormat(const std::string& input) {
    std::string trimmed = trimLeft(input);
    std::string lower = toLower(trimmed.substr(0, std::min(trimmed.size(), size_t(100))));

    // HTML detection
    if (startsWith(lower, "<!doctype html") ||
        startsWith(lower, "<html") ||
        startsWith(lower, "<body") ||
        startsWith(lower, "<div") ||
        startsWith(lower, "<head")) {
        return Format::HTML;
    }

    // XML detection
    if (!trimmed.empty() && trimmed[0] == '<') {
        return Format::XML;
    }

    // JSON/JSONL detection
    if (!trimmed.empty() && (trimmed[0] == '{' || trimmed[0] == '[')) {
        // Check for JSONL: multiple lines all starting with {
        if (trimmed[0] == '{') {
            std::istringstream stream(trimmed);
            std::string line;
            int lineCount = 0;
            bool allStartWithBrace = true;

            while (std::getline(stream, line)) {
                std::string lt = trimLeft(line);
                if (lt.empty()) continue;
                lineCount++;
                if (lt[0] != '{') {
                    allStartWithBrace = false;
                    break;
                }
            }

            if (allStartWithBrace && lineCount > 1) {
                return Format::JSONL;
            }
        }
        return Format::JSON;
    }

    // Default to YAML
    return Format::YAML;
}

// ---------------------------------------------------------------------------
// Format detection from file extension
// ---------------------------------------------------------------------------
Format detectFormatFromExtension(const std::string& filename) {
    std::string ext = getExtension(filename);

    if (ext == ".json") return Format::JSON;
    if (ext == ".jsonl" || ext == ".ndjson") return Format::JSONL;
    if (ext == ".yaml" || ext == ".yml") return Format::YAML;
    if (ext == ".xml") return Format::XML;
    if (ext == ".html" || ext == ".htm") return Format::HTML;

    // Fallback: try content detection would be needed, but for extension-only, default JSON
    return Format::JSON;
}

// ---------------------------------------------------------------------------
// Format metadata
// ---------------------------------------------------------------------------
const char* formatName(Format f) {
    switch (f) {
        case Format::JSON:  return "JSON";
        case Format::JSONL: return "JSONL";
        case Format::YAML:  return "YAML";
        case Format::XML:   return "XML";
        case Format::HTML:  return "HTML";
    }
    return "Unknown";
}

const char* formatExtension(Format f) {
    switch (f) {
        case Format::JSON:  return "json";
        case Format::JSONL: return "jsonl";
        case Format::YAML:  return "yaml";
        case Format::XML:   return "xml";
        case Format::HTML:  return "html";
    }
    return "";
}

const char* formatMimeType(Format f) {
    switch (f) {
        case Format::JSON:  return "application/json";
        case Format::JSONL: return "application/x-ndjson";
        case Format::YAML:  return "application/x-yaml";
        case Format::XML:   return "application/xml";
        case Format::HTML:  return "text/html";
    }
    return "application/octet-stream";
}

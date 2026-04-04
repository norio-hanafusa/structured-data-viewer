#pragma once
#include <string>

enum class Format { JSON, JSONL, YAML, XML, HTML };

Format detectFormat(const std::string& input);
Format detectFormatFromExtension(const std::string& filename);
const char* formatName(Format f);
const char* formatExtension(Format f);
const char* formatMimeType(Format f);

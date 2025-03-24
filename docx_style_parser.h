#ifndef DOCX_STYLE_PARSER_H
#define DOCX_STYLE_PARSER_H

#include <string>
#include <vector>
#include <map>

struct StyleInfo {
    std::string name;
    std::string type;  // "paragraph", "character", "table", etc.
    std::map<std::string, std::string> properties;
};

std::vector<StyleInfo> extractDocxStyles(const std::string& filePath);

#endif // DOCX_STYLE_PARSER_H

#ifndef DOCX_STYLE_PARSER_H
#define DOCX_STYLE_PARSER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <zip.h>
#include <libxml/parser.h>

struct StyleInfo {
    std::string name;
    std::string type;
    std::map<std::string, std::string> properties;
    std::string fontName;
    std::string fontSize;
};

// Zip file handling
std::unique_ptr<zip_t, void(*)(zip_t*)> openDocxFile(const std::string& filePath);
std::vector<char> readStylesXml(zip_t* zip);

// XML parsing
std::unique_ptr<xmlDoc, void(*)(xmlDocPtr)> parseXml(const std::vector<char>& xmlData);
std::vector<xmlNodePtr> findStyleNodes(xmlDocPtr doc);

// Style processing
StyleInfo processStyleNode(xmlNodePtr node);
void extractFontProperties(xmlNodePtr rPrNode, StyleInfo& style);
void extractOtherProperties(xmlNodePtr node, StyleInfo& style);

// Main interface
std::vector<StyleInfo> extractDocxStyles(const std::string& filePath);

#endif // DOCX_STYLE_PARSER_H

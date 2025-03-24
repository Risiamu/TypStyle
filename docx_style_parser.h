#ifndef DOCX_STYLE_PARSER_H
#define DOCX_STYLE_PARSER_H

#include <string>
#include <vector>
#include <map>
#include <memory>

// Forward declarations
struct zip_t;
struct zip_file_t;
struct zip_stat_t;
struct zip_error_t;
typedef struct _xmlDoc xmlDoc;
typedef xmlDoc* xmlDocPtr;
typedef struct _xmlNode xmlNode;
typedef xmlNode* xmlNodePtr;
typedef struct _xmlAttr xmlAttr;

struct StyleInfo {
    std::string name;
    std::string type;
    std::map<std::string, std::string> properties;
    std::string fontName;
    std::string fontSize;
};

namespace DocxParser {

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

} // namespace DocxParser

#endif // DOCX_STYLE_PARSER_H

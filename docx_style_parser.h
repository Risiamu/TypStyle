#ifndef DOCX_STYLE_PARSER_H
#define DOCX_STYLE_PARSER_H

#include <string>
#include <vector>
#include <map>
#include <memory>

// Forward declarations for libzip
struct zip_t;
struct zip_file_t;
struct zip_stat_t;
struct zip_error_t;

// Forward declarations for libxml2
struct _xmlDoc;
typedef _xmlDoc xmlDoc;
typedef _xmlDoc* xmlDocPtr;
struct _xmlNode;
typedef _xmlNode xmlNode;
typedef _xmlNode* xmlNodePtr;
struct _xmlAttr;
typedef _xmlAttr xmlAttr;

// Function pointer types for deleters
typedef void (*zip_deleter)(zip_t*);
typedef void (*xmlDoc_deleter)(xmlDoc*);

/**
 * @brief Contains information about a DOCX style
 */
struct StyleInfo {
    std::string name;        ///< Name of the style
    std::string type;        ///< Type of style (paragraph/character/table/etc)
    std::map<std::string, std::string> properties; ///< Style properties
    std::string fontName;    ///< Primary font name used in this style
    std::string fontSize;    ///< Font size in half-points (1/144 of an inch)
};

namespace DocxParser {

/**
 * @brief Opens a DOCX file and returns a zip archive handle
 * @param filePath Path to the DOCX file
 * @return Unique pointer to zip archive with custom deleter
 * @throws std::runtime_error if file cannot be opened
 */
std::unique_ptr<zip_t, zip_deleter> openDocxFile(const std::string& filePath);

/**
 * @brief Reads styles.xml from an open DOCX zip archive
 * @param zip Open zip archive handle
 * @return Vector containing raw XML data
 * @throws std::runtime_error if styles.xml cannot be read
 */
std::vector<char> readStylesXml(zip_t* zip);

/**
 * @brief Parses XML data into a document object
 * @param xmlData Raw XML data to parse
 * @return Unique pointer to XML document with custom deleter
 * @throws std::runtime_error if XML parsing fails
 */
std::unique_ptr<xmlDoc, xmlDoc_deleter> parseXml(const std::vector<char>& xmlData);

/**
 * @brief Finds all style nodes in an XML document
 * @param doc Parsed XML document
 * @return Vector of pointers to style nodes
 */
std::vector<xmlNodePtr> findStyleNodes(xmlDocPtr doc);

/**
 * @brief Processes a single style node into StyleInfo
 * @param node XML node representing a style
 * @return StyleInfo containing extracted style data
 */
StyleInfo processStyleNode(xmlNodePtr node);

/**
 * @brief Extracts font properties from run properties node
 * @param rPrNode XML node containing run properties
 * @param[out] style StyleInfo to populate with font data
 */
void extractFontProperties(xmlNodePtr rPrNode, StyleInfo& style);

/**
 * @brief Extracts other style properties from a node
 * @param node XML node to process
 * @param[out] style StyleInfo to populate with properties
 */
void extractOtherProperties(xmlNodePtr node, StyleInfo& style);

/**
 * @brief Main interface - extracts all styles from a DOCX file
 * @param filePath Path to the DOCX file
 * @return Vector of StyleInfo objects for all styles found
 * @throws std::runtime_error for any file/parsing errors
 */
std::vector<StyleInfo> extractDocxStyles(const std::string& filePath);

} // namespace DocxParser

#endif // DOCX_STYLE_PARSER_H

#include "docx_style_parser.h"
#include <iostream>
#include <stdexcept>
#include <zip.h>
#include <zipconf.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

using namespace std;

namespace DocxParser {

/**
 * @brief Opens a DOCX file (which is a ZIP archive) and returns a handle
 * @param filePath Path to the DOCX file to open
 * @return unique_ptr managing the zip archive handle with custom deleter
 * @throws runtime_error if the file cannot be opened
 * 
 * @details
 * DOCX files are ZIP archives containing XML files. This function:
 * 1. Attempts to open the file using libzip
 * 2. Wraps the raw zip handle in a unique_ptr with custom deleter
 * 3. Provides basic error handling if opening fails
 */
unique_ptr<zip_t, zip_close_t> openDocxFile(const string& filePath) {
    int zipError = 0;
    zip_t* zip = zip_open(filePath.c_str(), 0, &zipError);
    if (!zip) {
        string errorMsg = "Failed to open DOCX file: ";
        if (zipError != 0) {
            errorMsg += "Error code: " + to_string(zipError);
        } else {
            errorMsg += "Unknown error";
        }
        throw runtime_error(errorMsg);
    }
    return unique_ptr<zip_t, zip_close_t>(zip, &zip_close);
}

/**
 * @brief Reads the styles.xml file from an open DOCX zip archive
 * @param zip Open zip archive handle
 * @return Vector containing the raw XML data
 * @throws runtime_error if styles.xml cannot be found/read
 * 
 * @details
 * DOCX stores styles in word/styles.xml. This function:
 * 1. Checks for existence of styles.xml in the archive
 * 2. Opens and reads the file contents into memory
 * 3. Returns the raw XML data for parsing
 */
vector<char> readStylesXml(zip_t* zip) {
    zip_stat_t stats = {};
    if (zip_stat(zip, "word/styles.xml", 0, &stats) != 0) {
        throw runtime_error("styles.xml not found in DOCX archive");
    }

    unique_ptr<zip_file_t, zip_fclose_t> stylesFile(
        zip_fopen(zip, "word/styles.xml", 0),
        &zip_fclose
    );

    if (!stylesFile) {
        throw runtime_error("Failed to open styles.xml in archive");
    }

    vector<char> buffer(stats.size);
    if (zip_fread(stylesFile.get(), buffer.data(), buffer.size()) != static_cast<zip_int64_t>(buffer.size())) {
        throw runtime_error("Failed to read styles.xml content");
    }

    return buffer;
}

// XML parsing functions
/**
 * @brief Parses raw XML data into a libxml2 document object
 * @param xmlData Raw XML data to parse
 * @return unique_ptr managing the xmlDoc with custom deleter
 * @throws runtime_error if parsing fails
 * 
 * @details
 * Uses libxml2 to parse the XML content from memory. The returned
 * document object can be traversed using libxml2's DOM API.
 */
unique_ptr<xmlDoc, void(*)(xmlDocPtr)> parseXml(const vector<char>& xmlData) {
    xmlDocPtr doc = xmlReadMemory(xmlData.data(), xmlData.size(), "styles.xml", NULL, 0);
    if (!doc) {
        throw runtime_error("Failed to parse styles.xml content");
    }
    return unique_ptr<xmlDoc, void(*)(xmlDocPtr)>(doc, xmlFreeDoc);
}

/**
 * @brief Finds all style nodes in the parsed XML document
 * @param doc Parsed XML document
 * @return Vector of pointers to style nodes
 * 
 * @details
 * Searches the XML document for all <w:style> elements which
 * represent individual style definitions in the DOCX file.
 */
vector<xmlNodePtr> findStyleNodes(xmlDocPtr doc) {
    vector<xmlNodePtr> styleNodes;
    xmlNodePtr root = xmlDocGetRootElement(doc);
    
    for (xmlNodePtr node = root->children; node; node = node->next) {
        if (node->type == XML_ELEMENT_NODE && 
            xmlStrcmp(node->name, (const xmlChar*)"style") == 0) {
            styleNodes.push_back(node);
        }
    }
    return styleNodes;
}

// Style processing functions
/**
 * @brief Extracts font-related properties from a run properties node
 * @param rPrNode XML node containing run properties (rPr)
 * @param[out] style StyleInfo struct to populate with font data
 * 
 * @details
 * Processes the rPr node to extract:
 * - Font names (ascii, hAnsi, eastAsia)
 * - Font size (sz)
 * Any found properties are stored in the StyleInfo struct.
 */
void extractFontProperties(xmlNodePtr rPrNode, StyleInfo& style) {
    for (xmlNodePtr child = rPrNode->children; child; child = child->next) {
        if (child->type != XML_ELEMENT_NODE) continue;
        
        string nodeName(reinterpret_cast<const char*>(child->name));
        
        if (nodeName == "rFonts") {
            for (xmlAttr* attr = child->properties; attr; attr = attr->next) {
                string attrName(reinterpret_cast<const char*>(attr->name));
                if (attrName == "ascii" || attrName == "hAnsi" || attrName == "eastAsia") {
                    xmlChar* value = xmlGetProp(child, attr->name);
                    if (value) {
                        style.fontName = reinterpret_cast<char*>(value);
                        xmlFree(value);
                        break; // Just get the first font name we find
                    }
                }
            }
        }
        else if (nodeName == "sz") {
            xmlChar* size = xmlGetProp(child, (const xmlChar*)"val");
            if (size) {
                style.fontSize = reinterpret_cast<char*>(size);
                xmlFree(size);
            }
        }
    }
}

/**
 * @brief Extracts non-font style properties from a style node
 * @param node XML style node to process
 * @param[out] style StyleInfo struct to populate with properties
 * 
 * @details
 * Processes the style node to extract all properties including:
 * - Style type
 * - Base style
 * - Any other style attributes
 * Properties are stored in the StyleInfo's properties map.
 */
void extractOtherProperties(xmlNodePtr node, StyleInfo& style) {
    for (xmlNodePtr prop = node->children; prop; prop = prop->next) {
        if (prop->type != XML_ELEMENT_NODE) continue;
        
        string propName(reinterpret_cast<const char*>(prop->name));
        if (propName == "rPr") {
            extractFontProperties(prop, style);
        } else {
            xmlChar* content = xmlNodeGetContent(prop);
            if (content) {
                style.properties[propName] = reinterpret_cast<char*>(content);
                xmlFree(content);
            }
        }
    }
}

/**
 * @brief Processes a single style node into a StyleInfo struct
 * @param node XML node representing a style definition
 * @return StyleInfo containing all extracted style data
 * 
 * @details
 * This is the main style processing function that:
 * 1. Extracts basic style attributes (name, type)
 * 2. Processes all child nodes for properties
 * 3. Returns a fully populated StyleInfo struct
 */
StyleInfo processStyleNode(xmlNodePtr node) {
    StyleInfo style;
    
    if (auto name = xmlGetProp(node, (const xmlChar*)"name")) {
        style.name = reinterpret_cast<char*>(name);
        xmlFree(name);
    }
    
    if (auto type = xmlGetProp(node, (const xmlChar*)"type")) {
        style.type = reinterpret_cast<char*>(type);
        xmlFree(type);
    }

    extractOtherProperties(node, style);
    return style;
}

// Main interface
} // namespace DocxParser

/**
 * @brief Main interface function - extracts all styles from a DOCX file
 * @param filePath Path to the DOCX file to process
 * @return Vector of StyleInfo objects for all styles found
 * @throws runtime_error for any file/parsing errors
 * 
 * @details
 * This is the primary public interface that coordinates:
 * 1. Opening the DOCX zip archive
 * 2. Locating and reading styles.xml
 * 3. Parsing the XML
 * 4. Extracting all style definitions
 * 5. Returning the collected style information
 */
vector<StyleInfo> DocxParser::extractDocxStyles(const string& filePath) {
    auto zip = DocxParser::openDocxFile(filePath);
    auto stylesXml = DocxParser::readStylesXml(zip.get());
    auto doc = DocxParser::parseXml(stylesXml);
    auto styleNodes = DocxParser::findStyleNodes(doc.get());
    
    vector<StyleInfo> styles;
    for (auto node : styleNodes) {
        styles.push_back(processStyleNode(node));
    }
    
    return styles;
}

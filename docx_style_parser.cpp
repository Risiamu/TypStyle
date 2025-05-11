// Standard C++ headers
#include <iostream>   // For console I/O (cout, cerr)
#include <stdexcept>  // For standard exceptions (runtime_error)

// Third-party library headers
#include <zip.h>      // For ZIP archive handling (libzip)
#include <zipconf.h>  // ZIP configuration constants
#include <libxml/parser.h>  // For XML parsing (libxml2)
#include <libxml/tree.h>    // For XML DOM tree handling

// Project header
#include "docx_style_parser.h"  // Our own header with declarations

// Using the standard namespace to avoid prefixing std::
// Note: In header files, it's better to explicitly use std:: to avoid namespace pollution
using namespace std;

/*
 * DOCX Style Parser - Beginner's Guide
 *
 * This program helps extract styling information from Microsoft Word (.docx) files.
 * DOCX files are actually ZIP archives containing XML files with styling data.
 *
 * Key Concepts:
 * 1. ZIP Handling: Uses libzip to read the compressed archive
 * 2. XML Parsing: Uses libxml2 to process the XML data
 * 3. Memory Management: Uses smart pointers (unique_ptr) for automatic cleanup
 * 4. Error Handling: Uses exceptions (try/catch) for error reporting
 *
 * Code Structure:
 * - Namespace DocxParser contains all main functionality
 * - Functions are organized in processing pipeline order
 * - StyleInfo struct stores extracted style properties
 *
 * Learning Resources:
 * - C++ Basics: https://www.learncpp.com/
 * - libxml2 Docs: https://gnome.pages.gitlab.gnome.org/libxml2/devhelp/
 * - libzip Docs: https://libzip.org/documentation/
 * - Smart Pointers: https://en.cppreference.com/w/cpp/memory/unique_ptr
 */

namespace DocxParser {

/**
 * @brief Opens a DOCX file (which is a ZIP archive) and returns a handle
 * @param filePath Path to the DOCX file to open (const reference)
 * @return unique_ptr managing the zip archive handle with custom deleter
 * @throws runtime_error if the file cannot be opened
 *
 * @details
 * DOCX files are ZIP archives containing XML files. This function:
 * 1. Attempts to open the file using libzip
 * 2. Wraps the raw zip handle in a unique_ptr with custom deleter
 * 3. Provides basic error handling if opening fails
 *
 * Beginner Notes:
 * - const string &filePath: Reference to avoid copy, const for safety
 * - unique_ptr: Smart pointer that auto-deletes resources
 * - zip_t: C library type representing ZIP archive
 * - zip_open: C function that returns a pointer to zip archive
 */
    /**
     * @brief Demonstrates RAII pattern with custom deleter
     * 
     * Common Patterns Used:
     * 1. RAII (Resource Acquisition Is Initialization):
     *    - Resources (file handles) are acquired in constructor
     *    - Released automatically when object goes out of scope
     * 2. Smart Pointers:
     *    - unique_ptr ensures single ownership
     *    - Custom deleter (zip_close) handles proper cleanup
     * 3. Error Handling:
     *    - Throws exceptions on failure (RAII ensures no leaks)
     */
    /**
     * @brief Opens a DOCX file and returns a managed zip archive handle
     * 
     * Syntax Breakdown:
     * 
     * 1. Return Type:
     *    std::unique_ptr<zip_t, zip_close_t>
     *    - unique_ptr: Smart pointer for automatic memory management
     *    - zip_t*: Raw pointer type being managed
     *    - zip_close_t: Type of custom deleter function
     * 
     * 2. Parameters:
     *    const string &filePath
     *    - const: Promises not to modify the string
     *    - string &: Reference to avoid copying
     * 
     * 3. Function Body:
     *    - Uses libzip's zip_open() C function
     *    - Converts C++ string to C-style with c_str()
     *    - Checks for errors and throws exceptions
     *    - Returns smart pointer with custom deleter
     */
    unique_ptr<zip_t, zip_close_t> openDocxFile(const string &filePath) {
        // Error code storage (0 means success)
        int zipError = 0;  

        // Convert C++ string to C-style and open archive
        // zip_open() parameters:
        // 1. path: C-style string path
        // 2. flags: 0 for default options
        // 3. error: Pointer to store error code
        zip_t *zip = zip_open(filePath.c_str(), 0, &zipError);

        if (!zip) {
            // Build detailed error message
            string errorMsg = "Failed to open DOCX file: ";
            errorMsg += (zipError != 0) 
                ? "Error code: " + to_string(zipError)
                : "Unknown error";
            
            throw runtime_error(errorMsg);  
        }

        // Create smart pointer with custom deleter
        // unique_ptr constructor parameters:
        // 1. Raw pointer to manage
        // 2. Deleter function (zip_close in this case)
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
    // Reads styles.xml from an open ZIP archive into a vector<char>
    /**
     * @brief Shows buffer management pattern
     * 
     * Common Patterns Used:
     * 1. Buffer Management:
     *    - Pre-allocates vector to exact needed size
     *    - Uses data() for raw buffer access
     * 2. RAII Wrappers:
     *    - unique_ptr manages C file handle
     * 3. Error Propagation:
     *    - Throws exceptions up the call stack
     */
    vector<char> readStylesXml(zip_t *zip) {
        // Initialize zip_stat_t struct to zero (C-style initialization)
        // This will hold file metadata like size
        zip_stat_t stats = {};

        // Check if styles.xml exists in the archive
        // zip_stat() returns 0 on success, non-zero on failure
        if (zip_stat(zip, "word/styles.xml", 0, &stats) != 0) {
            throw runtime_error("styles.xml not found in DOCX archive");
        }

        // Open the file inside the ZIP archive
        // We use unique_ptr with custom deleter to ensure proper cleanup
        unique_ptr<zip_file_t, zip_fclose_t> stylesFile(
            zip_fopen(zip, "word/styles.xml", 0),  // Open file
            &zip_fclose  // Function to call when unique_ptr is destroyed
        );

        if (!stylesFile) {  // Check if file opened successfully
            throw runtime_error("Failed to open styles.xml in archive");
        }

        // Create a vector with exact size needed for file contents
        // vector<char> is like a dynamic array that manages its own memory
        vector<char> buffer(stats.size);

        // Read file contents into vector
        // stylesFile.get() - gets raw pointer from unique_ptr
        // buffer.data() - gets pointer to vector's internal array
        // static_cast converts size_t to zip_int64_t explicitly
        if (zip_fread(stylesFile.get(), buffer.data(), buffer.size()) !=
            static_cast<zip_int64_t>(buffer.size())) {
            throw runtime_error("Failed to read styles.xml content");
        }

        // Return vector by value - C++ will use move semantics (no copy)
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
    /**
     * @brief Demonstrates C library integration pattern
     * 
     * Common Patterns Used:
     * 1. C Library Integration:
     *    - Wraps C-style pointers in smart pointers
     *    - Uses function pointer as custom deleter
     * 2. Memory Safety:
     *    - Ensures xmlFreeDoc is always called
     * 3. Error Checking:
     *    - Validates parser output
     */
    unique_ptr<xmlDoc, void (*)(xmlDocPtr)> parseXml(const vector<char> &xmlData) {
        // xmlReadMemory parses XML from a memory buffer (not from file)
        // Parameters:
        // 1. Pointer to XML data (vector's data() method)
        // 2. Size of data
        // 3. "Filename" for error messages
        // 4. Encoding (NULL for auto-detect)
        // 5. Parser options (0 for defaults)
        xmlDocPtr doc = xmlReadMemory(xmlData.data(), xmlData.size(), "styles.xml", NULL, 0);

        if (!doc) {  // Check if parsing succeeded
            throw runtime_error("Failed to parse styles.xml content");
        }

        // Create unique_ptr with custom deleter function (xmlFreeDoc)
        // The function pointer syntax looks complex but ensures proper cleanup
        return unique_ptr<xmlDoc, void (*)(xmlDocPtr)>(doc, xmlFreeDoc);
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
    /**
     * @brief Shows XML DOM traversal pattern
     * 
     * Common Patterns Used:
     * 1. Tree Traversal:
     *    - Iterates through child/sibling pointers
     *    - Uses depth-first search
     * 2. Filter Pattern:
     *    - Collects nodes matching criteria
     * 3. C String Handling:
     *    - Uses xmlStrcmp for XML string comparison
     */
    vector <xmlNodePtr> findStyleNodes(xmlDocPtr doc) {
        // Create empty vector to store node pointers
        vector<xmlNodePtr> styleNodes;

        // Get root element of XML document
        xmlNodePtr root = xmlDocGetRootElement(doc);

        // Iterate through all child nodes of root
        // Unlike Python's for loops, this is a manual linked list traversal:
        // - Start with first child (root->children)
        // - Continue while node is not null (node)
        // - Move to next sibling (node = node->next)
        for (xmlNodePtr node = root->children; node; node = node->next) {
            // Check if node is an element node (not text/comment/etc)
            // and if its name is "style"
            if (node->type == XML_ELEMENT_NODE &&
                xmlStrcmp(node->name, (const xmlChar *) "style") == 0) {
                // Check for required qFormat and exclude semiHidden
                bool hasQFormat = false;
                bool isHidden = false;
                for (xmlNodePtr child = node->children; child; child = child->next) {
                    if (child->type == XML_ELEMENT_NODE) {
                        if (xmlStrcmp(child->name, (const xmlChar *) "qFormat") == 0) {
                            hasQFormat = true;
                        } else if (xmlStrcmp(child->name, (const xmlChar *) "semiHidden") == 0) {
                            isHidden = true;
                        }
                    }
                }
                if (hasQFormat && !isHidden) {
                    // Add node pointer to vector
                    styleNodes.push_back(node);
                }
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
    /**
     * @brief Extracts font properties from XML node
     * @param rPrNode XML node pointer (xmlNodePtr)
     * @param style Reference to StyleInfo to modify
     *
     * Beginner Notes:
     * - xmlNodePtr is a pointer to libxml2's node structure
     * - &style means we modify the original object (not a copy)
     * - The function walks through XML child nodes using:
     *   - node->children: Start of child nodes list
     *   - node->next: Move to next sibling node
     *   - node->type: Check if node is an element (XML_ELEMENT_NODE)
     */
    /**
     * @brief Demonstrates property extraction pattern
     * 
     * Common Patterns Used:
     * 1. Visitor Pattern:
     *    - Visits each node to extract properties
     * 2. Out Parameter:
     *    - Modifies style object passed by reference
     * 3. String Conversion:
     *    - Converts XML strings to C++ strings
     */
    void extractFontProperties(xmlNodePtr rPrNode, StyleInfo &style) {
        // Iterate through all child nodes of rPrNode
        for (xmlNodePtr child = rPrNode->children; child; child = child->next) {
            // Skip non-element nodes (text nodes, comments etc)
            if (child->type != XML_ELEMENT_NODE) continue;

            // Convert XML node name (xmlChar*) to C++ string
            // reinterpret_cast is used to convert between pointer types
            string nodeName(reinterpret_cast<const char *>(child->name));

            if (nodeName == "rFonts") {
                // Handle font name properties
                // Iterate through all attributes of the node
                for (xmlAttr *attr = child->properties; attr; attr = attr->next) {
                    string attrName(reinterpret_cast<const char *>(attr->name));

                    // We're interested in these font name attributes
                    if (attrName == "ascii" || attrName == "hAnsi" || attrName == "eastAsia") {
                        // Get attribute value - returns allocated string we must free
                        xmlChar *value = xmlGetProp(child, attr->name);
                        if (value) {
                            // Store font name in style struct
                            style.fontName = reinterpret_cast<char *>(value);
                            // Free the allocated string - C++ doesn't have garbage collection
                            xmlFree(value);
                            break; // Just get the first font name we find
                        }
                    }
                }
            } else if (nodeName == "sz") {
                // Handle font size property
                xmlChar *size = xmlGetProp(child, (const xmlChar *) "val");
                if (size) {
                    style.fontSize = reinterpret_cast<char *>(size);
                    xmlFree(size);  // Don't forget to free allocated memory!
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
    void extractStyleName(xmlNodePtr node, StyleInfo &style) {
        // Look for w:name child element
        for (xmlNodePtr child = node->children; child; child = child->next) {
            if (child->type == XML_ELEMENT_NODE &&
                xmlStrcmp(child->name, (const xmlChar *) "name") == 0) {
                if (auto val = xmlGetProp(child, (const xmlChar *) "val")) {
                    style.name = reinterpret_cast<char *>(val);
                    xmlFree(val);
                }
                break; // Found name element, stop searching
            }
        }
    }

    /**
     * @brief Processes XML node properties into StyleInfo
     * @param node XML node to process
     * @param style StyleInfo to populate with properties
     *
     * @details Handles both attribute-based values (val) and node content
     */
    void processXmlProperties(xmlNodePtr node, StyleInfo& style) {
        if (node->type != XML_ELEMENT_NODE) return;

        string propName(reinterpret_cast<const char*>(node->name));
        xmlChar* val = xmlGetProp(node, (const xmlChar*)"val");
        if (val) {
            style.properties[propName] = reinterpret_cast<char*>(val);
            xmlFree(val);
        } else {
            xmlChar* content = xmlNodeGetContent(node);
            if (content) {
                style.properties[propName] = reinterpret_cast<char*>(content);
                xmlFree(content);
            }
        }
    }

    void extractOtherProperties(xmlNodePtr node, StyleInfo &style) {
        for (xmlNodePtr prop = node->children; prop; prop = prop->next) {
            if (prop->type != XML_ELEMENT_NODE) continue;

            string propName(reinterpret_cast<const char *>(prop->name));
            if (propName == "rPr") {
                extractFontProperties(prop, style);
                // Process all rPr children as properties
                for (xmlNodePtr child = prop->children; child; child = child->next) {
                    processXmlProperties(child, style);
                }
            } else if (propName == "pPr") {
                // Process all pPr children as properties
                for (xmlNodePtr child = prop->children; child; child = child->next) {
                    processXmlProperties(child, style);
                }
            } else {
                // Handle properties directly
                processXmlProperties(prop, style);
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

        extractStyleName(node, style);

        if (auto type = xmlGetProp(node, (const xmlChar *) "type")) {
            // There are two types we need to pay attention to in styles.xml
            // one is paragraph, this is the main extraction target
            // as typesetters use paragraph based style more than anything else.
            // So we first get the type of the style.
            style.type = reinterpret_cast<char *>(type);
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
/**
 * @brief Shows pipeline processing pattern
 * 
 * Common Patterns Used:
 * 1. Pipeline Pattern:
 *    - Each step transforms output for next step
 *    - Steps: Open -> Read -> Parse -> Process
 * 2. Resource Management:
 *    - All resources automatically cleaned up
 * 3. Collection Processing:
 *    - Transforms XML nodes into StyleInfo objects
 */
vector<StyleInfo> DocxParser::extractDocxStyles(const string &filePath) {
    auto zip = DocxParser::openDocxFile(filePath);
    auto stylesXml = DocxParser::readStylesXml(zip.get());
    auto doc = DocxParser::parseXml(stylesXml);
    auto styleNodes = DocxParser::findStyleNodes(doc.get());

    vector<StyleInfo> styles;
    for (auto node: styleNodes) {
        styles.push_back(processStyleNode(node));
    }

    return styles;
}

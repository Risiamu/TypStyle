#include <zip.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <memory>

struct StyleInfo {
    std::string name;
    std::string type;  // "paragraph", "character", "table", etc.
    std::map<std::string, std::string> properties;
};

std::vector<StyleInfo> extractDocxStyles(const std::string& filePath) {
    std::vector<StyleInfo> styles;
    
    // Error handling for ZIP operations
    int zipError = 0;
    std::unique_ptr<zip_t, decltype(&zip_close)> zip(
        zip_open(filePath.c_str(), 0, &zipError),
        &zip_close
    );
    
    if (!zip) {
        std::string errorMsg = "Failed to open DOCX file: ";
        if (zipError != 0) {
            zip_error_t error;
            zip_error_init_with_code(&error, zipError);
            errorMsg += zip_error_strerror(&error);
            zip_error_fini(&error);
        } else {
            errorMsg += zip_strerror(nullptr);
        }
        throw std::runtime_error(errorMsg);
    }

    // Locate styles.xml in the archive
    zip_stat_t stats;
    if (zip_stat(zip.get(), "word/styles.xml", 0, &stats) != 0) {
        throw std::runtime_error("styles.xml not found in DOCX archive - this may not be a valid DOCX file");
    }

    // Read styles.xml content
    std::unique_ptr<zip_file_t, decltype(&zip_fclose)> stylesFile(
        zip_fopen(zip.get(), "word/styles.xml", 0),
        &zip_fclose
    );
    
    if (!stylesFile) {
        throw std::runtime_error("Failed to open styles.xml in archive - file may be corrupted");
    }

    std::vector<char> buffer(stats.size);
    if (zip_fread(stylesFile.get(), buffer.data(), buffer.size()) != static_cast<zip_int64_t>(buffer.size())) {
        throw std::runtime_error("Failed to read styles.xml content - file may be corrupted");
    }

    // Parse XML content
    std::unique_ptr<xmlDoc, decltype(&xmlFreeDoc)> doc(
        xmlReadMemory(buffer.data(), buffer.size(), "styles.xml", NULL, 0),
        &xmlFreeDoc
    );
    
    if (!doc) {
        throw std::runtime_error("Failed to parse styles.xml content - invalid XML format");
    }

    // Process XML nodes
    xmlNodePtr root = xmlDocGetRootElement(doc.get());
    for (xmlNodePtr node = root->children; node; node = node->next) {
        if (node->type == XML_ELEMENT_NODE && 
            xmlStrcmp(node->name, (const xmlChar*)"style") == 0) {
            
            StyleInfo style;
            
            // Get style attributes
            if (auto name = xmlGetProp(node, (const xmlChar*)"name")) {
                style.name = reinterpret_cast<char*>(name);
                xmlFree(name);
            }
            
            if (auto type = xmlGetProp(node, (const xmlChar*)"type")) {
                style.type = reinterpret_cast<char*>(type);
                xmlFree(type);
            }

            // Process style properties
            for (xmlNodePtr prop = node->children; prop; prop = prop->next) {
                if (prop->type == XML_ELEMENT_NODE) {
                    std::string propName(reinterpret_cast<const char*>(prop->name));
                    
                    // Special handling for rPr (run properties) which contains font info
                    if (propName == "rPr") {
                        for (xmlNodePtr rPrChild = prop->children; rPrChild; rPrChild = rPrChild->next) {
                            if (rPrChild->type == XML_ELEMENT_NODE) {
                                std::string childName(reinterpret_cast<const char*>(rPrChild->name));
                                
                                // Extract font names
                                if (childName == "rFonts") {
                                    for (xmlAttr* attr = rPrChild->properties; attr; attr = attr->next) {
                                        std::string attrName(reinterpret_cast<const char*>(attr->name));
                                        if (attrName == "ascii" || attrName == "hAnsi" || attrName == "eastAsia") {
                                            xmlChar* attrValue = xmlGetProp(rPrChild, attr->name);
                                            if (attrValue) {
                                                style.properties["font:" + attrName] = reinterpret_cast<char*>(attrValue);
                                                xmlFree(attrValue);
                                            }
                                        }
                                    }
                                }
                                // Extract font size
                                else if (childName == "sz") {
                                    for (xmlAttr* attr = rPrChild->properties; attr; attr = attr->next) {
                                        std::string attrName(reinterpret_cast<const char*>(attr->name));
                                        if (attrName == "val") {
                                            xmlChar* attrValue = xmlGetProp(rPrChild, attr->name);
                                            if (attrValue) {
                                                style.properties["font:size"] = reinterpret_cast<char*>(attrValue);
                                                xmlFree(attrValue);
                                            }
                                        }
                                    }
                                }
                                // Extract other properties normally
                                else {
                                    xmlChar* content = xmlNodeGetContent(rPrChild);
                                    if (content) {
                                        style.properties[childName] = reinterpret_cast<char*>(content);
                                        xmlFree(content);
                                    }
                                }
                            }
                        }
                    }
                    // Process other properties normally
                    else {
                        xmlChar* content = xmlNodeGetContent(prop);
                        if (content) {
                            style.properties[propName] = reinterpret_cast<char*>(content);
                            xmlFree(content);
                        }
                    }
                }
            }
            
            styles.push_back(std::move(style));
        }
    }

    return styles;
}

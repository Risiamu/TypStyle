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
        std::cerr << "Failed to open DOCX file: " << zip_error_strerror(zipError) << std::endl;
        return styles;
    }

    // Locate styles.xml in the archive
    zip_stat_t stats;
    if (zip_stat(zip.get(), "word/styles.xml", 0, &stats) != 0) {
        std::cerr << "styles.xml not found in DOCX archive" << std::endl;
        return styles;
    }

    // Read styles.xml content
    std::unique_ptr<zip_file_t, decltype(&zip_fclose)> stylesFile(
        zip_fopen(zip.get(), "word/styles.xml", 0),
        &zip_fclose
    );
    
    if (!stylesFile) {
        std::cerr << "Failed to open styles.xml in archive" << std::endl;
        return styles;
    }

    std::vector<char> buffer(stats.size);
    if (zip_fread(stylesFile.get(), buffer.data(), buffer.size()) != static_cast<zip_int64_t>(buffer.size())) {
        std::cerr << "Failed to read styles.xml content" << std::endl;
        return styles;
    }

    // Parse XML content
    std::unique_ptr<xmlDoc, decltype(&xmlFreeDoc)> doc(
        xmlReadMemory(buffer.data(), buffer.size(), "styles.xml", NULL, 0),
        &xmlFreeDoc
    );
    
    if (!doc) {
        std::cerr << "Failed to parse styles.xml content" << std::endl;
        return styles;
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

            // Process style properties (simplified)
            for (xmlNodePtr prop = node->children; prop; prop = prop->next) {
                if (prop->type == XML_ELEMENT_NODE) {
                    std::string propName(reinterpret_cast<char*>(prop->name));
                    style.properties[propName] = ""; // Actual implementation would extract values
                }
            }
            
            styles.push_back(std::move(style));
        }
    }

    return styles;
}

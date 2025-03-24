#include "docx_style_parser.h"
#include <iostream>
#include <stdexcept>
#include <zip.h>
#include <zipconf.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

using namespace std;

// Zip file handling functions
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
unique_ptr<xmlDoc, void(*)(xmlDocPtr)> parseXml(const vector<char>& xmlData) {
    xmlDocPtr doc = xmlReadMemory(xmlData.data(), xmlData.size(), "styles.xml", NULL, 0);
    if (!doc) {
        throw runtime_error("Failed to parse styles.xml content");
    }
    return unique_ptr<xmlDoc, void(*)(xmlDocPtr)>(doc, xmlFreeDoc);
}

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
vector<StyleInfo> extractDocxStyles(const string& filePath) {
    auto zip = openDocxFile(filePath);
    auto stylesXml = readStylesXml(zip.get());
    auto doc = parseXml(stylesXml);
    auto styleNodes = findStyleNodes(doc.get());
    
    vector<StyleInfo> styles;
    for (auto node : styleNodes) {
        styles.push_back(processStyleNode(node));
    }
    
    return styles;
}

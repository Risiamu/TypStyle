#include <iostream>
#include "docx_style_parser.h"
#include "spdlog/spdlog.h"

int main() {
    try {
        // TIP
        // Extract and display DOCX styles
        const std::string docxPath = "sample.docx";
        std::cout << "\nExtracting styles from " << docxPath << "...\n";

        // TIP
        // Check if file exists first
        if (FILE* file = fopen(docxPath.c_str(), "r")) {
            // TIP
            // The c_str() function in C++ is a member function of the std::string class
            // that converts a C++ std::string into a C-style string
            // (a null-terminated character array, const char*).
            fclose(file);
            spdlog::info("docx file exists, closing file now.");
            auto styles = DocxParser::extractDocxStyles(docxPath);

            if (styles.empty()) {
                std::cout << "No styles found in the document.\n";
            } else {
                std::cout << "Found " << styles.size() << " styles:\n";
                for (const auto& style : styles) {
                    std::cout << "\nStyle: " << style.name
                              << " (Type: " << style.type << ")\n";
                    std::cout << "Properties:\n";
                    if (!style.fontName.empty()) {
                        std::cout << "  Font: " << style.fontName << "\n";
                    }
                    if (!style.fontSize.empty()) {
                        std::cout << "  Font Size: " << style.fontSize << "\n";
                    }
                    for (const auto& prop : style.properties) {
                        std::cout << "  " << prop.first << ": "
                             << (prop.second.empty() ? "[no value]" : prop.second) << "\n";
                    }
                }
            }
        } else {
            std::cerr << "Error: File not found - " << docxPath << "\n";
            std::cerr << "Please ensure the file exists in the same directory as the executable.\n";
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return 1;
    }

    return 0;
}

// TIP See CLion help at <a
// href="https://www.jetbrains.com/help/clion/">jetbrains.com/help/clion/</a>.
//  Also, you can try interactive lessons for CLion by selecting
//  'Help | Learn IDE Features' from the main menu.

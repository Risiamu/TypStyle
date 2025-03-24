#include <iostream>
#include "docx_style_parser.h"
// click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
int main() {
    // TIP Press <shortcut actionId="RenameElement"/> when your caret is at the
    // <b>lang</b> variable name to see how CLion can help you rename it.
    auto lang = "C++";
    std::cout << "Hello and welcome to " << lang << "!\n";

    for (int i = 1; i <= 5; i++) {
        // TIP Press <shortcut actionId="Debug"/> to start debugging your code.
        // We have set one <icon src="AllIcons.Debugger.Db_set_breakpoint"/>
        // breakpoint for you, but you can always add more by pressing
        // <shortcut actionId="ToggleLineBreakpoint"/>.
        std::cout << "i = " << i << std::endl;
    }

    try {
        // Extract and display DOCX styles
        const std::string docxPath = "sample.docx";
        std::cout << "\nExtracting styles from " << docxPath << "...\n";
        
        // Check if file exists first
        if (FILE* file = fopen(docxPath.c_str(), "r")) {
            fclose(file);
            auto styles = extractDocxStyles(docxPath);
            
            if (styles.empty()) {
                std::cout << "No styles found in the document.\n";
            } else {
                std::cout << "Found " << styles.size() << " styles:\n";
                for (const auto& style : styles) {
                    std::cout << "\nStyle: " << style.name 
                              << " (Type: " << style.type << ")\n";
                    cout << "Properties:\n";
                    if (!style.fontName.empty()) {
                        cout << "  Font: " << style.fontName << "\n";
                    }
                    if (!style.fontSize.empty()) {
                        cout << "  Font Size: " << style.fontSize << "\n";
                    }
                    for (const auto& prop : style.properties) {
                        cout << "  " << prop.first << ": " 
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

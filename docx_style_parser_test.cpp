#include "docx_style_parser.h"
#include <gtest/gtest.h>
#include <fstream>

using namespace DocxParser;

TEST(DocxParserTest, HandlesMissingFile) {
    EXPECT_THROW(extractDocxStyles("nonexistent.docx"), std::runtime_error);
}

TEST(DocxParserTest, ExtractsStylesFromValidDocx) {
    auto styles = extractDocxStyles("sample.docx");
    
    // Verify we got some styles
    ASSERT_FALSE(styles.empty());
    
    // Verify common style properties
    bool foundNormal = false;
    for (const auto& style : styles) {
        if (style.name == "Normal") {
            foundNormal = true;
            EXPECT_EQ(style.type, "paragraph");
            EXPECT_FALSE(style.fontName.empty());
            break;
        }
    }
    EXPECT_TRUE(foundNormal);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

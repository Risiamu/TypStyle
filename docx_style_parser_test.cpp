// Google Test framework header - provides testing macros and infrastructure
#include <gtest/gtest.h>
// Standard C++ file operations
#include <fstream>
// Our header with the functions to test
#include "docx_style_parser.h"

// Use the DocxParser namespace where our functions are defined
using namespace DocxParser;

/**
 * @brief Test case for handling missing DOCX files
 *
 * @details
 * This test verifies that attempting to extract styles from a non-existent file
 * throws the expected runtime_error exception.
 *
 * Key Concepts:
 * - EXPECT_THROW: Google Test macro that verifies a specific exception is thrown
 * - std::runtime_error: Standard exception type for runtime errors
 */
TEST(DocxParserTest, HandlesMissingFile) {
    // Attempt to extract styles from non-existent file
    // EXPECT_THROW verifies the code throws the specified exception type
    EXPECT_THROW(extractDocxStyles("nonexistent.docx"), std::runtime_error);
}

/**
 * @brief Test case for successful style extraction
 *
 * @details
 * This test verifies that:
 * 1. Styles can be extracted from a valid DOCX file
 * 2. The returned collection isn't empty
 * 3. Common style properties (like "Normal" style) exist and have expected values
 *
 * Key Concepts:
 * - ASSERT_FALSE: Google Test assertion that fails if condition is true
 * - EXPECT_EQ: Google Test assertion that verifies equality
 * - Range-based for loop: Modern C++ loop syntax
 * - auto: Automatic type deduction
 */
TEST(DocxParserTest, ExtractsStylesFromValidDocx) {
    // Extract styles from sample document
    // 'auto' deduces the return type (vector<StyleInfo>)
    auto styles = extractDocxStyles("sample.docx");

    // Verify we got some styles - ASSERT_FALSE will stop test if empty
    ASSERT_FALSE(styles.empty());

    // Verify common style properties exist
    bool foundNormal = false;
    // Range-based for loop iterates through all styles
    for (const auto& style : styles) {
        // Check for the standard "Normal" style
        if (style.name == "Normal") {
            foundNormal = true;
            // Verify it's a paragraph style
            EXPECT_EQ(style.type, "paragraph");
            // Verify it has a font name specified
            EXPECT_FALSE(style.fontName.empty());
            break; // No need to check other styles once we find Normal
        }
    }
    // Verify we found the Normal style
    EXPECT_TRUE(foundNormal);
}

/**
 * @brief Main function for running tests
 *
 * @details
 * Initializes Google Test framework and runs all test cases.
 *
 * @param argc Argument count
 * @param argv Argument values
 * @return int Exit code (0 for success)
 */
int main(int argc, char **argv) {
    // Initialize Google Test framework
    testing::InitGoogleTest(&argc, argv);
    // Run all tests and return exit code
    return RUN_ALL_TESTS();
}

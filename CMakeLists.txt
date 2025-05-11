cmake_minimum_required(VERSION 3.30)

message(CMAKE_TOOLCHAIN_FILE = "${CMAKE_TOOLCHAIN_FILE}")


project(TypStyle
        VERSION 1.0
        DESCRIPTION "DOCX Style Extractor Tool"
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/vcpkg/installed/x64-windows-static")
set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "Vcpkg triplet")

add_compile_options("/utf-8")

find_package(libxml2 CONFIG REQUIRED)
find_package(libzip CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

# Main application
add_executable(TypStyle
        main.cpp
        docx_style_parser.cpp
        docx_style_parser.h
)

target_link_libraries(TypStyle PRIVATE
        LibXml2::LibXml2
        libzip::zip
        spdlog::spdlog
)

# Test executable
enable_testing()
add_executable(TypStyleTests
        docx_style_parser_test.cpp
        docx_style_parser.cpp
)

target_link_libraries(TypStyleTests PRIVATE
        LibXml2::LibXml2
        libzip::zip
        GTest::gtest
        GTest::gmock_main
)

add_test(NAME TypStyleTests COMMAND TypStyleTests)

if (MSVC)
    # Set consistent runtime library for all configurations
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "" FORCE)
    add_compile_options(
        $<$<CONFIG:Debug>:/MTd>
        $<$<CONFIG:Release>:/MT>
    )
endif()

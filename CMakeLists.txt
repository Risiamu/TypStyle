# Minimum required CMake version
cmake_minimum_required(VERSION 3.30)

# Project name and basic configuration
project(TypStyle
    VERSION 1.0
    DESCRIPTION "DOCX Style Extractor Tool"
    LANGUAGES CXX
)

# Configure vcpkg integration
# Note: This assumes vcpkg is installed at the specified path
# To install vcpkg:
#   git clone https://github.com/Microsoft/vcpkg.git
#   .\vcpkg\bootstrap-vcpkg.bat
set(CMAKE_TOOLCHAIN_FILE "C:/Users/kousa/.vcpkg-clion/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")

# C++ standard configuration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Static linking configuration
# This tells vcpkg to use static libraries instead of dynamic ones
# To install required static libraries:
#   vcpkg install libzip:x64-windows-static libxml2:x64-windows-static
set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "Vcpkg triplet")

# Add vcpkg package paths to search locations
list(APPEND CMAKE_PREFIX_PATH 
    "C:/Users/kousa/Documents/TypStyle/vcpkg/packages/libxml2_x64-windows-static/share/libxml2"
    "C:/Users/kousa/Documents/TypStyle/vcpkg/packages/libzip_x64-windows-static/share/libzip"
    "C:/Users/kousa/Documents/TypStyle/vcpkg/packages/zlib_x64-windows-static/share/zlib"
)

# Find and configure required dependencies
find_package(ZLIB CONFIG REQUIRED)
find_package(libzip CONFIG REQUIRED)
find_package(LibXml2 CONFIG REQUIRED)

# Define the executable and its source files
add_executable(TypStyle
    main.cpp
    docx_style_parser.cpp
    docx_style_parser.h
)

# Link dependencies to the executable
target_link_libraries(TypStyle PRIVATE
    libzip::zip       # ZIP archive handling
    LibXml2::LibXml2   # XML parsing
    ZLIB::ZLIB        # Compression library
)

# Configure static runtime for MSVC builds
# This ensures no MSVC runtime DLLs are required
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Optional: Install target configuration
# install(TARGETS TypStyle DESTINATION bin)

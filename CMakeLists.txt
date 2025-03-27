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
set(CMAKE_TOOLCHAIN_FILE "C:/Users/kousa/Documents/TypStyle/vcpkg/scripts/buildsystems/vcpkg.cmake"
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
#find_package(BZip2 REQUIRED)
find_package(libxml2 CONFIG REQUIRED)
find_package(libzip CONFIG REQUIRED)

# Define the executable and its source files
add_executable(TypStyle
    main.cpp
    docx_style_parser.cpp
    docx_style_parser.h
)

# Link dependencies to the executable
target_link_libraries(TypStyle PRIVATE
    libzip::zip       # ZIP archive handling
    libxml2::libxml2   # XML parsing
)

# Configure static runtime for MSVC builds
# This ensures no MSVC runtime DLLs are required
#if(MSVC)
#    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
#endif()

# Optional: Install target configuration
# install(TARGETS TypStyle DESTINATION bin)

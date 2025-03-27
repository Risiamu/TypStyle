# Minimum required CMake version
cmake_minimum_required(VERSION 3.30)
set(CMAKE_TOOLCHAIN_FILE "C:/Users/kousa/Documents/TypStyle/vcpkg/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
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
set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
set(CMAKE_EXE_LINKER_FLAGS "-static")
set(BUILD_SHARED_LIBS OFF)
# C++ standard configuration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Static linking configuration
set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "Vcpkg triplet")

find_package(libzip CONFIG REQUIRED)
find_package(LibXml2 CONFIG REQUIRED)
# Define the executable and its source files
add_executable(TypStyle
    main.cpp
    docx_style_parser.cpp
    docx_style_parser.h
)

target_compile_options(TypStyle PRIVATE /MT)
target_link_options(TypStyle PRIVATE /INCREMENTAL:NO /NODEFAULTLIB:MSVCRT)
# Link dependencies to the executable
target_link_libraries(TypStyle PRIVATE
    libzip::zip
    LibXml2::LibXml2
    bcrypt  # Required for libzip static linking
    advapi32 # Required for XML security functions
)

# Configure static runtime for MSVC builds
# This ensures no MSVC runtime DLLs are required
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Optional: Install target configuration
install(TARGETS TypStyle DESTINATION bin)

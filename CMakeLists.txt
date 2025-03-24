cmake_minimum_required(VERSION 3.30)
project(TypStyle)

# Set vcpkg toolchain
set(CMAKE_TOOLCHAIN_FILE "C:/Users/kousa/.vcpkg-clion/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")

set(CMAKE_CXX_STANDARD 11)

# Find required libraries
list(APPEND CMAKE_PREFIX_PATH 
    "C:/Users/kousa/.vcpkg-clion/vcpkg/packages/libzip_x64-mingw-dynamic/share/libzip"
    "C:/Users/kousa/.vcpkg-clion/vcpkg/packages/libxml2_x64-windows/share/libxml2"
)

find_package(libzip CONFIG REQUIRED)
find_package(LibXml2 CONFIG REQUIRED)

add_executable(TypStyle main.cpp docx_style_parser.cpp docx_style_parser.h)

# Link libraries
target_link_libraries(TypStyle PRIVATE
    libzip::zip
    LibXml2::LibXml2
)

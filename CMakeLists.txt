cmake_minimum_required(VERSION 3.30)
project(TypStyle)

# Set vcpkg toolchain
set(CMAKE_TOOLCHAIN_FILE "C:/Users/kousa/.vcpkg-clion/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")

set(CMAKE_CXX_STANDARD 17)

# Set vcpkg to use static libraries
set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "Vcpkg triplet")

# Find required libraries
find_package(libzip CONFIG REQUIRED)
find_package(LibXml2 CONFIG REQUIRED)

add_executable(TypStyle main.cpp docx_style_parser.cpp docx_style_parser.h)

# Link libraries
target_link_libraries(TypStyle PRIVATE
    libzip::zip
    LibXml2::LibXml2
)

# Set static runtime library
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

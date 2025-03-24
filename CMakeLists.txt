cmake_minimum_required(VERSION 3.30)
project(TypStyle)

# Set vcpkg toolchain (path might need adjustment)
set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" 
    CACHE STRING "Vcpkg toolchain file")

set(CMAKE_CXX_STANDARD 11)

# Find required libraries
find_package(libzip CONFIG REQUIRED)
find_package(LibXml2 CONFIG REQUIRED)

add_executable(TypStyle main.cpp docx_style_parser.cpp)

# Link libraries
target_link_libraries(TypStyle PRIVATE
    libzip::zip
    LibXml2::LibXml2
)

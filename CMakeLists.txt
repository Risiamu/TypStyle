cmake_minimum_required(VERSION 3.30)
project(TypStyle)

set(CMAKE_CXX_STANDARD 11)

# Find required libraries
find_package(LibZip REQUIRED)
find_package(LibXml2 REQUIRED)

# Include directories
include_directories(
    ${LIBZIP_INCLUDE_DIRS}
    ${LIBXML2_INCLUDE_DIR}
)

add_executable(TypStyle main.cpp
        docx_style_parser.cpp)

# Link libraries
target_link_libraries(TypStyle
    ${LIBZIP_LIBRARIES}
    ${LIBXML2_LIBRARIES}
)

cmake_minimum_required(VERSION 3.30)
project(TypStyle)

# Set vcpkg toolchain
set(CMAKE_TOOLCHAIN_FILE "C:/Users/kousa/.vcpkg-clion/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")

set(CMAKE_CXX_STANDARD 11)

# Find required libraries
find_package(ZIP REQUIRED)
find_package(LibXml2 REQUIRED)

add_executable(TypStyle main.cpp docx_style_parser.cpp)

# Link libraries
target_link_libraries(TypStyle PRIVATE
    ${ZIP_LIBRARIES}
    ${LIBXML2_LIBRARIES}
)

# Include directories
target_include_directories(TypStyle PRIVATE
    ${ZIP_INCLUDE_DIRS}
    ${LIBXML2_INCLUDE_DIR}
)

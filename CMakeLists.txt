cmake_minimum_required(VERSION 3.30)
project(TypStyle)

set(CMAKE_CXX_STANDARD 11)

add_executable(TypStyle main.cpp
        docx_style_parser.cpp)

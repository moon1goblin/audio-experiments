cmake_minimum_required(VERSION 3.26)

set (CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# for clangd lsp to work
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(audioshit)
add_executable(run readmidi.cpp)

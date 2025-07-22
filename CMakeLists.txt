cmake_minimum_required(VERSION 3.20)

project(sig LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Collect all source files
set(SOURCES
    src/main.cpp
    src/lexer/private/lexer.cpp
    src/parser/private/parser.cpp
    src/codegen/private/codegen.cpp
)

# Add the executable
add_executable(sig ${SOURCES})

# Optional: organize headers in `include/`
target_include_directories(sig PRIVATE ${CMAKE_SOURCE_DIR}/src)

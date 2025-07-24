cmake_minimum_required(VERSION 3.20)

project(sig LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find LLVM
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Set up LLVM
include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST UNIX_COMMAND "${LLVM_DEFINITIONS}")
add_definitions(${LLVM_DEFINITIONS_LIST})

# Find the libraries that correspond to the LLVM components
set(llvm_libs LLVM-20)

# Collect all source files
set(SOURCES
    src/main.cpp
    src/lexer/private/lexer.cpp
    src/parser/private/parser.cpp
    src/codegen/private/codegen.cpp
    src/codegen/private/llvm_codegen.cpp
    src/codegen/private/legacy_stubs.cpp
)

# Add the executable
add_executable(sig ${SOURCES})

# Link against LLVM libraries
target_link_libraries(sig ${llvm_libs})

# Optional: organize headers in `include/`
target_include_directories(sig PRIVATE ${CMAKE_SOURCE_DIR}/src)

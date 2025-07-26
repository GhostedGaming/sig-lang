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
    src/args.cpp
    src/lexer/private/lexer.cpp
    src/lexer/private/token.cpp
    src/lexer/private/lexer_helpers.cpp
    src/lexer/private/lexer_core.cpp
    src/parser/private/parser.cpp
src/parser/private/parser_functions.cpp
src/parser/private/parser_control_flow.cpp
src/parser/private/parser_statements.cpp
    src/modules/private/module_resolver.cpp
    src/codegen/private/runtime_setup.cpp
    src/codegen/private/code_generator.cpp
    src/codegen/private/jit_executor.cpp
    src/codegen/private/object_generator.cpp
)

# Add the executable
add_executable(sig ${SOURCES})

# Link against LLVM libraries
target_link_libraries(sig ${llvm_libs})

# Optional: organize headers in `include/`
target_include_directories(sig PRIVATE ${CMAKE_SOURCE_DIR}/src)

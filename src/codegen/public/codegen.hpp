#pragma once
#include <ast/public/ast.hpp>
#include <string>

// Legacy assembly generation (deprecated)
std::string generate_asm(const AST& ast);

// New LLVM-based compilation
#include "llvm_codegen.hpp"

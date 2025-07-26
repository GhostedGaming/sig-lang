#pragma once

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Constants.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <memory>
#include <unordered_map>
#include <string>
#include <ast/public/ast.hpp>

class CodeGen {
private:
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    std::unique_ptr<llvm::orc::LLJIT> jit;
    
    // Symbol tables
    std::unordered_map<std::string, llvm::Value*> variables;
    std::unordered_map<std::string, llvm::Function*> functions;
    
    // Current function being compiled
    llvm::Function* current_function = nullptr;
    
    // Target architecture
    bool target_32bit = false;
    
    // Helper methods
    llvm::Value* codegen_stmt(const ASTNode& stmt);
    void setup_runtime_functions();
    void configure_target_architecture();
    
public:
    CodeGen();
    explicit CodeGen(bool target_32bit);
    ~CodeGen() = default;
    
    // Configuration
    void set_target_32bit(bool enable) { target_32bit = enable; }
    
    // Main compilation interface
    void compile(const AST& program);
    void execute();
    void dump_ir();
    
    // Create executable
    void create_executable(const std::string& output_name);
    
    // Alternative: compile to object file
    void compile_to_object(const std::string& filename);
};

#include "../public/codegen.hpp"
#include <llvm/Support/TargetSelect.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Triple.h>
#include <llvm/TargetParser/Host.h>
#include <iostream>

using namespace llvm;

CodeGen::CodeGen() : target_32bit(false) {
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    
    context = std::make_unique<LLVMContext>();
    module = std::make_unique<Module>("sig_module", *context);
    builder = std::make_unique<IRBuilder<>>(*context);
    
    configure_target_architecture();
    
    auto JIT = orc::LLJITBuilder().create();
    if (!JIT) {
        std::cerr << "Failed to create JIT: " << toString(JIT.takeError()) << std::endl;
        exit(1);
    }
    jit = std::move(*JIT);
    
    setup_runtime_functions();
}

CodeGen::CodeGen(bool target_32bit) : target_32bit(target_32bit) {
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    
    context = std::make_unique<LLVMContext>();
    module = std::make_unique<Module>("sig_module", *context);
    builder = std::make_unique<IRBuilder<>>(*context);
    
    configure_target_architecture();
    
    auto JIT = orc::LLJITBuilder().create();
    if (!JIT) {
        std::cerr << "Failed to create JIT: " << toString(JIT.takeError()) << std::endl;
        exit(1);
    }
    jit = std::move(*JIT);
    
    setup_runtime_functions();
}

void CodeGen::configure_target_architecture() {
    if (target_32bit) {
        // Configure for 32-bit target
        std::string target_triple;
        
        #ifdef _WIN32
            target_triple = "i686-pc-windows-msvc";
        #elif defined(__APPLE__)
            target_triple = "i686-apple-darwin";
        #else
            target_triple = "i686-pc-linux-gnu";
        #endif
        
        module->setTargetTriple(target_triple);
        module->setDataLayout("e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128");
    } else {
        // Use native 64-bit target (default)
        module->setTargetTriple(Triple::normalize(sys::getDefaultTargetTriple()));
        
        // Set appropriate data layout for 64-bit
        #ifdef _WIN32
            module->setDataLayout("e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128");
        #else
            module->setDataLayout("e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128");
        #endif
    }
}

void CodeGen::setup_runtime_functions() {
    // printf for formatted output
    std::vector<Type*> printf_args = {PointerType::getUnqual(*context)};
    FunctionType* printf_type = FunctionType::get(Type::getInt32Ty(*context), printf_args, true);
    Function* printf_func = Function::Create(printf_type, Function::ExternalLinkage, "printf", *module);
    functions["printf"] = printf_func;
    
    // puts for simple strings
    std::vector<Type*> puts_args = {PointerType::getUnqual(*context)};
    FunctionType* puts_type = FunctionType::get(Type::getInt32Ty(*context), puts_args, false);
    Function* puts_func = Function::Create(puts_type, Function::ExternalLinkage, "puts", *module);
    functions["puts"] = puts_func;
}

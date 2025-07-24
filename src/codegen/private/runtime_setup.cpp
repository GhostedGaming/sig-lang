#include "../public/llvm_codegen.hpp"
#include <llvm/Support/TargetSelect.h>
#include <iostream>

using namespace llvm;

LLVMCodeGen::LLVMCodeGen() {
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    
    context = std::make_unique<LLVMContext>();
    module = std::make_unique<Module>("sig_module", *context);
    builder = std::make_unique<IRBuilder<>>(*context);
    
    auto JIT = orc::LLJITBuilder().create();
    if (!JIT) {
        std::cerr << "Failed to create JIT: " << toString(JIT.takeError()) << std::endl;
        exit(1);
    }
    jit = std::move(*JIT);
    
    setup_runtime_functions();
}

void LLVMCodeGen::setup_runtime_functions() {
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

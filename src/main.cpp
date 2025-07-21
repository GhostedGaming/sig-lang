#include <iostream>
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/Support/TargetSelect.h"

using namespace llvm;

int main() {
    std::cout << "Creating LLVM IR for a simple calculator...\n" << std::endl;
    
    // Initialize LLVM
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    
    LLVMContext context;
    std::unique_ptr<Module> module = std::make_unique<Module>("calculator", context);
    IRBuilder<> builder(context);
    
    // Create function: int add(int a, int b) { return a + b; }
    Type* intType = Type::getInt32Ty(context);
    FunctionType* addFuncType = FunctionType::get(intType, {intType, intType}, false);
    Function* addFunc = Function::Create(addFuncType, Function::ExternalLinkage, "add", *module);
    
    // Set parameter names
    auto args = addFunc->arg_begin();
    Value* a = &*args++;
    a->setName("a");
    Value* b = &*args;
    b->setName("b");
    
    // Create basic block and generate code
    BasicBlock* entry = BasicBlock::Create(context, "entry", addFunc);
    builder.SetInsertPoint(entry);
    Value* sum = builder.CreateAdd(a, b, "sum");
    builder.CreateRet(sum);
    
    // Create function: int multiply(int x, int y) { return x * y; }
    FunctionType* mulFuncType = FunctionType::get(intType, {intType, intType}, false);
    Function* mulFunc = Function::Create(mulFuncType, Function::ExternalLinkage, "multiply", *module);
    
    auto mulArgs = mulFunc->arg_begin();
    Value* x = &*mulArgs++;
    x->setName("x");
    Value* y = &*mulArgs;
    y->setName("y");
    
    BasicBlock* mulEntry = BasicBlock::Create(context, "entry", mulFunc);
    builder.SetInsertPoint(mulEntry);
    Value* product = builder.CreateMul(x, y, "product");
    builder.CreateRet(product);
    
    // Create main function: int main() { return add(5, 3) * multiply(2, 4); }
    FunctionType* mainFuncType = FunctionType::get(intType, {}, false);
    Function* mainFunc = Function::Create(mainFuncType, Function::ExternalLinkage, "main", *module);
    
    BasicBlock* mainEntry = BasicBlock::Create(context, "entry", mainFunc);
    builder.SetInsertPoint(mainEntry);
    
    // Call add(5, 3)
    std::vector<Value*> addArgs = {
        ConstantInt::get(intType, 5),
        ConstantInt::get(intType, 3)
    };
    Value* addResult = builder.CreateCall(addFunc, addArgs, "add_result");
    
    // Call multiply(2, 4)
    std::vector<Value*> mulArgs_call = {
        ConstantInt::get(intType, 2),
        ConstantInt::get(intType, 4)
    };
    Value* mulResult = builder.CreateCall(mulFunc, mulArgs_call, "mul_result");
    
    // Multiply the results: add_result * mul_result
    Value* finalResult = builder.CreateMul(addResult, mulResult, "final_result");
    builder.CreateRet(finalResult);
    
    // Verify the module
    std::string error;
    if (verifyModule(*module, &errs())) {
        std::cerr << "Error: Module verification failed!" << std::endl;
        return 1;
    }
    
    // Print the generated IR
    std::cout << "Generated LLVM IR:\n" << std::endl;
    module->print(outs(), nullptr);
    std::cout << "\n" << std::endl;
    
    // Execute the code using JIT
    std::cout << "Executing code with JIT...\n" << std::endl;
    
    ExecutionEngine* engine = EngineBuilder(std::move(module)).create();
    if (!engine) {
        std::cerr << "Error: Could not create execution engine!" << std::endl;
        return 1;
    }
    
    // Run the main function
    std::vector<GenericValue> noArgs;
    GenericValue result = engine->runFunction(mainFunc, noArgs);
    
    std::cout << "Result: add(5, 3) * multiply(2, 4) = " << result.IntVal.getSExtValue() << std::endl;
    std::cout << "Expected: 8 * 8 = 64" << std::endl;
    
    // Clean up
    delete engine;
    
    return 0;
}
#include "../public/llvm_codegen.hpp"
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/Casting.h>
#include <iostream>

using namespace llvm;

LLVMCodeGen::LLVMCodeGen() {
    // Initialize LLVM targets
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    
    // Create LLVM context and module
    context = std::make_unique<LLVMContext>();
    module = std::make_unique<Module>("sig_module", *context);
    builder = std::make_unique<IRBuilder<>>(*context);
    
    // Create JIT
    auto JIT = orc::LLJITBuilder().create();
    if (!JIT) {
        std::cerr << "Failed to create JIT: " << toString(JIT.takeError()) << std::endl;
        exit(1);
    }
    jit = std::move(*JIT);
    
    setup_runtime_functions();
}

void LLVMCodeGen::setup_runtime_functions() {
    // Create printf function for print statements
    std::vector<Type*> printf_args = {PointerType::getUnqual(*context)};
    FunctionType* printf_type = FunctionType::get(Type::getInt32Ty(*context), printf_args, true);
    Function* printf_func = Function::Create(printf_type, Function::ExternalLinkage, "printf", *module);
    functions["printf"] = printf_func;
    
    // Create puts function for simple string printing
    std::vector<Type*> puts_args = {PointerType::getUnqual(*context)};
    FunctionType* puts_type = FunctionType::get(Type::getInt32Ty(*context), puts_args, false);
    Function* puts_func = Function::Create(puts_type, Function::ExternalLinkage, "puts", *module);
    functions["puts"] = puts_func;
}

void LLVMCodeGen::compile(const AST& program) {
    // Create main function
    FunctionType* main_type = FunctionType::get(Type::getInt32Ty(*context), false);
    Function* main_func = Function::Create(main_type, Function::ExternalLinkage, "main", *module);
    current_function = main_func;
    
    BasicBlock* entry = BasicBlock::Create(*context, "entry", main_func);
    builder->SetInsertPoint(entry);
    
    // Compile each statement
    bool has_return = false;
    for (const auto& node : program) {
        codegen_stmt(node);
        // Check if this was a return statement
        if (std::holds_alternative<ReturnStatement>(node)) {
            has_return = true;
        }
    }
    
    // Add return 0 if no explicit return
    if (!has_return) {
        builder->CreateRet(ConstantInt::get(Type::getInt32Ty(*context), 0));
    }
    
    // Verify the function
    if (verifyFunction(*main_func, &errs())) {
        std::cerr << "Error: Function verification failed!" << std::endl;
        main_func->print(errs());
        exit(1);
    }
}

Value* LLVMCodeGen::codegen_stmt(const ASTNode& stmt) {
    return std::visit([this](const auto& s) -> Value* {
        using T = std::decay_t<decltype(s)>;
        
        if constexpr (std::is_same_v<T, ReturnStatement>) {
            Value* ret_val = ConstantInt::get(Type::getInt32Ty(*context), s.value);
            return builder->CreateRet(ret_val);
        }
        else if constexpr (std::is_same_v<T, PrintStatement>) {
            if (std::holds_alternative<int>(s.value)) {
                // Print integer
                Value* format_str = builder->CreateGlobalString("%d\n");
                Value* val = ConstantInt::get(Type::getInt32Ty(*context), std::get<int>(s.value));
                return builder->CreateCall(functions["printf"], {format_str, val});
            } else {
                // Print string
                Value* str = builder->CreateGlobalString(std::get<std::string>(s.value));
                return builder->CreateCall(functions["puts"], {str});
            }
        }
        else if constexpr (std::is_same_v<T, VariableDeclaration>) {
            // Allocate space for variable
            Type* var_type = Type::getInt32Ty(*context);
            AllocaInst* alloca = builder->CreateAlloca(var_type, nullptr, s.var_name);
            variables[s.var_name] = alloca;
            return alloca;
        }
        else if constexpr (std::is_same_v<T, VariableAssignment>) {
            Value* var = variables[s.var_name];
            
            // Determine the type and value first
            Value* val = nullptr;
            Type* var_type = nullptr;
            
            if (std::holds_alternative<int>(s.value)) {
                val = ConstantInt::get(Type::getInt32Ty(*context), std::get<int>(s.value));
                var_type = Type::getInt32Ty(*context);
            } else if (std::holds_alternative<double>(s.value)) {
                val = ConstantFP::get(Type::getDoubleTy(*context), std::get<double>(s.value));
                var_type = Type::getDoubleTy(*context);
            } else if (std::holds_alternative<float>(s.value)) {
                val = ConstantFP::get(Type::getFloatTy(*context), std::get<float>(s.value));
                var_type = Type::getFloatTy(*context);
            } else if (std::holds_alternative<std::string>(s.value)) {
                val = builder->CreateGlobalString(std::get<std::string>(s.value));
                var_type = PointerType::getUnqual(*context);
            }
            
            if (!var && val && var_type) {
                // Create variable if it doesn't exist (for let x = value; syntax)
                AllocaInst* alloca = builder->CreateAlloca(var_type, nullptr, s.var_name);
                variables[s.var_name] = alloca;
                var = alloca;
            }
            
            if (val && var) {
                return builder->CreateStore(val, var);
            }
        }
        else if constexpr (std::is_same_v<T, PrintVariable>) {
            Value* var = variables[s.variableName];
            if (!var) {
                std::cerr << "Error: Undefined variable " << s.variableName << std::endl;
                return nullptr;
            }
            
            // Try to determine if it's a string or integer based on the alloca type
            AllocaInst* alloca = dyn_cast<AllocaInst>(var);
            if (alloca) {
                Type* allocated_type = alloca->getAllocatedType();
                if (allocated_type->isPointerTy()) {
                    // String variable
                    Value* loaded_val = builder->CreateLoad(allocated_type, var);
                    return builder->CreateCall(functions["puts"], {loaded_val});
                } else if (allocated_type->isIntegerTy()) {
                    // Integer variable
                    Value* loaded_val = builder->CreateLoad(allocated_type, var);
                    Value* format_str = builder->CreateGlobalString("%d\n");
                    return builder->CreateCall(functions["printf"], {format_str, loaded_val});
                }
            }
            
            return nullptr;
        }
        else if constexpr (std::is_same_v<T, FunctionDefinition>) {
            // Create function
            FunctionType* func_type = FunctionType::get(Type::getVoidTy(*context), false);
            Function* func = Function::Create(func_type, Function::ExternalLinkage, s.name, *module);
            functions[s.name] = func;
            
            // Save current state
            Function* prev_func = current_function;
            current_function = func;
            
            BasicBlock* func_entry = BasicBlock::Create(*context, "entry", func);
            builder->SetInsertPoint(func_entry);
            
            // Compile function body
            for (const auto& stmt : s.body) {
                codegen_stmt(stmt);
            }
            
            // Add return void if no explicit return
            if (func_entry->getTerminator() == nullptr) {
                builder->CreateRetVoid();
            }
            
            // Restore state
            current_function = prev_func;
            if (prev_func) {
                builder->SetInsertPoint(&prev_func->back());
            }
            
            return func;
        }
        else if constexpr (std::is_same_v<T, FunctionCall>) {
            Function* func = functions[s.function_name];
            if (!func) {
                std::cerr << "Error: Undefined function " << s.function_name << std::endl;
                return nullptr;
            }
            return builder->CreateCall(func);
        }
        else if constexpr (std::is_same_v<T, IfStatement>) {
            // For now, simple implementation - can be expanded later
            return nullptr;
        }
        else if constexpr (std::is_same_v<T, WhileStatement>) {
            // For now, simple implementation - can be expanded later
            return nullptr;
        }
        else if constexpr (std::is_same_v<T, AsmStatement>) {
            // Inline assembly - skip for now as LLVM handles this differently
            return nullptr;
        }
        
        return nullptr;
    }, stmt);
}

void LLVMCodeGen::execute() {
    // Link with C standard library for printf/puts
    auto ProcessSymsGenerator = orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(
        jit->getDataLayout().getGlobalPrefix());
    if (!ProcessSymsGenerator) {
        std::cerr << "Failed to create process symbols generator: " 
                  << toString(ProcessSymsGenerator.takeError()) << std::endl;
        return;
    }
    jit->getMainJITDylib().addGenerator(std::move(*ProcessSymsGenerator));
    
    // Create a copy of the module and context for JIT
    auto TSM = orc::ThreadSafeModule(std::move(module), std::move(context));
    
    if (auto Err = jit->addIRModule(std::move(TSM))) {
        std::cerr << "Failed to add module: " << toString(std::move(Err)) << std::endl;
        return;
    }
    
    // Look up main function
    auto MainSym = jit->lookup("main");
    if (!MainSym) {
        std::cerr << "Failed to find main function: " << toString(MainSym.takeError()) << std::endl;
        return;
    }
    
    // Execute main
    auto MainFunc = (int(*)())MainSym->getValue();
    int result = MainFunc();
    std::cout << "Program exited with code: " << result << std::endl;
}

void LLVMCodeGen::dump_ir() {
    if (module) {
        module->print(outs(), nullptr);
    } else {
        std::cerr << "Error: Module has been moved or is null" << std::endl;
    }
}

void LLVMCodeGen::compile_to_object(const std::string& filename) {
    // This would implement object file generation
    // For now, just dump IR
    std::error_code EC;
    raw_fd_ostream dest(filename + ".ll", EC, sys::fs::OF_None);
    
    if (EC) {
        std::cerr << "Could not open file: " << EC.message() << std::endl;
        return;
    }
    
    module->print(dest, nullptr);
}

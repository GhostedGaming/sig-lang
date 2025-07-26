#include "../public/codegen.hpp"
#include <llvm/IR/Verifier.h>
#include <iostream>

using namespace llvm;

void CodeGen::compile(const AST& program) {
    FunctionType* main_type = FunctionType::get(Type::getInt32Ty(*context), false);
    Function* main_func = Function::Create(main_type, Function::ExternalLinkage, "main", *module);
    current_function = main_func;
    
    BasicBlock* entry = BasicBlock::Create(*context, "entry", main_func);
    builder->SetInsertPoint(entry);
    
    bool has_return = false;
    for (const auto& node : program) {
        codegen_stmt(node);
        if (std::holds_alternative<ReturnStatement>(node)) {
            has_return = true;
        }
    }
    
    if (!has_return) {
        builder->CreateRet(ConstantInt::get(Type::getInt32Ty(*context), 0));
    }
    
    if (verifyFunction(*main_func, &errs())) {
        std::cerr << "Error: Function verification failed!" << std::endl;
        main_func->print(errs());
        exit(1);
    }
}

Value* CodeGen::codegen_stmt(const ASTNode& stmt) {
    return std::visit([this](const auto& s) -> Value* {
        using T = std::decay_t<decltype(s)>;
        
        if constexpr (std::is_same_v<T, ReturnStatement>) {
            Value* ret_val = ConstantInt::get(Type::getInt32Ty(*context), s.value);
            return builder->CreateRet(ret_val);
        }
        else if constexpr (std::is_same_v<T, PrintStatement>) {
            if (std::holds_alternative<int>(s.value)) {
                Value* format_str = builder->CreateGlobalString("%d\n");
                Value* val = ConstantInt::get(Type::getInt32Ty(*context), std::get<int>(s.value));
                return builder->CreateCall(functions["printf"], {format_str, val});
            } else {
                Value* str = builder->CreateGlobalString(std::get<std::string>(s.value));
                return builder->CreateCall(functions["puts"], {str});
            }
        }
        else if constexpr (std::is_same_v<T, VariableDeclaration>) {
            Type* var_type = Type::getInt32Ty(*context);
            AllocaInst* alloca = builder->CreateAlloca(var_type, nullptr, s.var_name);
            variables[s.var_name] = alloca;
            return alloca;
        }
        else if constexpr (std::is_same_v<T, VariableAssignment>) {
            Value* var = variables[s.var_name];
            
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
            
            AllocaInst* alloca = dyn_cast<AllocaInst>(var);
            if (alloca) {
                Type* allocated_type = alloca->getAllocatedType();
                if (allocated_type->isPointerTy()) {
                    Value* loaded_val = builder->CreateLoad(allocated_type, var);
                    return builder->CreateCall(functions["puts"], {loaded_val});
                } else if (allocated_type->isIntegerTy()) {
                    Value* loaded_val = builder->CreateLoad(allocated_type, var);
                    Value* format_str = builder->CreateGlobalString("%d\n");
                    return builder->CreateCall(functions["printf"], {format_str, loaded_val});
                }
            }
            
            return nullptr;
        }
        else if constexpr (std::is_same_v<T, FunctionDefinition>) {
            FunctionType* func_type = FunctionType::get(Type::getVoidTy(*context), false);
            Function* func = Function::Create(func_type, Function::ExternalLinkage, s.name, *module);
            functions[s.name] = func;
            
            Function* prev_func = current_function;
            current_function = func;
            
            BasicBlock* func_entry = BasicBlock::Create(*context, "entry", func);
            builder->SetInsertPoint(func_entry);
            
            for (const auto& stmt : s.body) {
                codegen_stmt(stmt);
            }
            
            if (func_entry->getTerminator() == nullptr) {
                builder->CreateRetVoid();
            }
            
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
            // TODO: implement
            return nullptr;
        }
        else if constexpr (std::is_same_v<T, WhileStatement>) {
            // TODO: implement
            return nullptr;
        }
        else if constexpr (std::is_same_v<T, ForStatement>) {
            // Create basic blocks for the for loop
            BasicBlock* for_init = BasicBlock::Create(*context, "for.init", current_function);
            BasicBlock* for_cond = BasicBlock::Create(*context, "for.cond", current_function);
            BasicBlock* for_body = BasicBlock::Create(*context, "for.body", current_function);
            BasicBlock* for_inc = BasicBlock::Create(*context, "for.inc", current_function);
            BasicBlock* for_end = BasicBlock::Create(*context, "for.end", current_function);
            
            // Jump to initialization
            builder->CreateBr(for_init);
            
            // Initialization block
            builder->SetInsertPoint(for_init);
            // Create and initialize loop variable
            Type* var_type = Type::getInt32Ty(*context);
            AllocaInst* loop_var = builder->CreateAlloca(var_type, nullptr, s.initialization);
            variables[s.initialization] = loop_var;
            Value* init_val = ConstantInt::get(Type::getInt32Ty(*context), 1);
            builder->CreateStore(init_val, loop_var);
            builder->CreateBr(for_cond);
            
            // Condition block
            builder->SetInsertPoint(for_cond);
            Value* current_val = builder->CreateLoad(var_type, loop_var);
            Value* count_val = ConstantInt::get(Type::getInt32Ty(*context), std::stoi(s.count));
            Value* cond = builder->CreateICmpSLE(current_val, count_val);
            builder->CreateCondBr(cond, for_body, for_end);
            
            // Body block
            builder->SetInsertPoint(for_body);
            for (const auto& body_stmt : s.body) {
                codegen_stmt(body_stmt);
            }
            builder->CreateBr(for_inc);
            
            // Increment block
            builder->SetInsertPoint(for_inc);
            Value* next_val = builder->CreateAdd(current_val, ConstantInt::get(Type::getInt32Ty(*context), 1));
            builder->CreateStore(next_val, loop_var);
            builder->CreateBr(for_cond);
            
            // End block
            builder->SetInsertPoint(for_end);
            
            return nullptr;
        }
        else if constexpr (std::is_same_v<T, AsmStatement>) {
            // TODO: inline assembly support
            return nullptr;
        }
        
        return nullptr;
    }, stmt);
}

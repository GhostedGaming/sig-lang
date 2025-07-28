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
            if (no_std) {
                std::cerr << "Error: print() is not available with --no-std. Use direct system calls or implement your own I/O.\n";
                return nullptr;
            }
            
            Value* val = codegen_expression(s.value);
            if (!val) return nullptr;
            
            // Determine format based on type
            if (val->getType()->isIntegerTy(32)) {
                Value* format_str = builder->CreateGlobalString("%d");
                return builder->CreateCall(functions["printf"], {format_str, val});
            } else if (val->getType()->isDoubleTy()) {
                Value* format_str = builder->CreateGlobalString("%f");
                return builder->CreateCall(functions["printf"], {format_str, val});
            } else if (val->getType()->isIntegerTy(1)) {
                Value* format_str = builder->CreateGlobalString("%s");
                Value* cond = builder->CreateICmpEQ(val, ConstantInt::get(Type::getInt1Ty(*context), 1));
                Value* true_str = builder->CreateGlobalString("true");
                Value* false_str = builder->CreateGlobalString("false");
                Value* result = builder->CreateSelect(cond, true_str, false_str);
                return builder->CreateCall(functions["printf"], {format_str, result});
            } else {
                Value* format_str = builder->CreateGlobalString("%s");
                return builder->CreateCall(functions["printf"], {format_str, val});
            }
        }
        else if constexpr (std::is_same_v<T, PrintlnStatement>) {
            if (no_std) {
                std::cerr << "Error: println() is not available with --no-std. Use direct system calls or implement your own I/O.\n";
                return nullptr;
            }
            
            Value* val = codegen_expression(s.value);
            if (!val) return nullptr;
            
            // Determine format based on type
            if (val->getType()->isIntegerTy(32)) {
                Value* format_str = builder->CreateGlobalString("%d\n");
                return builder->CreateCall(functions["printf"], {format_str, val});
            } else if (val->getType()->isDoubleTy()) {
                Value* format_str = builder->CreateGlobalString("%f\n");
                return builder->CreateCall(functions["printf"], {format_str, val});
            } else if (val->getType()->isIntegerTy(1)) {
                Value* cond = builder->CreateICmpEQ(val, ConstantInt::get(Type::getInt1Ty(*context), 1));
                Value* true_str = builder->CreateGlobalString("true");
                Value* false_str = builder->CreateGlobalString("false");
                Value* result = builder->CreateSelect(cond, true_str, false_str);
                return builder->CreateCall(functions["puts"], {result});
            } else {
                return builder->CreateCall(functions["puts"], {val});
            }
        }
        else if constexpr (std::is_same_v<T, VariableDeclaration>) {
            Type* var_type = Type::getInt32Ty(*context); // Default
            
            if (s.type.has_value()) {
                switch (s.type.value()) {
                    case SigType::U8:
                    case SigType::I8:
                        var_type = Type::getInt8Ty(*context);
                        break;
                    case SigType::U16:
                    case SigType::I16:
                        var_type = Type::getInt16Ty(*context);
                        break;
                    case SigType::U32:
                    case SigType::I32:
                        var_type = Type::getInt32Ty(*context);
                        break;
                    case SigType::U64:
                    case SigType::I64:
                        var_type = Type::getInt64Ty(*context);
                        break;
                    default:
                        var_type = Type::getInt32Ty(*context);
                        break;
                }
            }
            
            AllocaInst* alloca = builder->CreateAlloca(var_type, nullptr, s.var_name);
            variables[s.var_name] = alloca;
            return alloca;
        }
        else if constexpr (std::is_same_v<T, VariableAssignment>) {
            Value* var = variables[s.var_name];
            
            Value* val = nullptr;
            Type* var_type = nullptr;
            
            if (std::holds_alternative<TypedValue>(s.value)) {
                const TypedValue& typed_val = std::get<TypedValue>(s.value);
                
                switch (typed_val.type) {
                    case SigType::U8:
                        val = ConstantInt::get(Type::getInt8Ty(*context), std::get<uint8_t>(typed_val.value));
                        var_type = Type::getInt8Ty(*context);
                        break;
                    case SigType::U16:
                        val = ConstantInt::get(Type::getInt16Ty(*context), std::get<uint16_t>(typed_val.value));
                        var_type = Type::getInt16Ty(*context);
                        break;
                    case SigType::U32:
                        val = ConstantInt::get(Type::getInt32Ty(*context), std::get<uint32_t>(typed_val.value));
                        var_type = Type::getInt32Ty(*context);
                        break;
                    case SigType::U64:
                        val = ConstantInt::get(Type::getInt64Ty(*context), std::get<uint64_t>(typed_val.value));
                        var_type = Type::getInt64Ty(*context);
                        break;
                    case SigType::I8:
                        val = ConstantInt::get(Type::getInt8Ty(*context), std::get<int8_t>(typed_val.value));
                        var_type = Type::getInt8Ty(*context);
                        break;
                    case SigType::I16:
                        val = ConstantInt::get(Type::getInt16Ty(*context), std::get<int16_t>(typed_val.value));
                        var_type = Type::getInt16Ty(*context);
                        break;
                    case SigType::I32:
                        val = ConstantInt::get(Type::getInt32Ty(*context), std::get<int32_t>(typed_val.value));
                        var_type = Type::getInt32Ty(*context);
                        break;
                    case SigType::I64:
                        val = ConstantInt::get(Type::getInt64Ty(*context), std::get<int64_t>(typed_val.value));
                        var_type = Type::getInt64Ty(*context);
                        break;
                    default:
                        // Handle other types if needed
                        break;
                }
            } else if (std::holds_alternative<int>(s.value)) {
                val = ConstantInt::get(Type::getInt32Ty(*context), std::get<int>(s.value));
                var_type = Type::getInt32Ty(*context);
            } else if (std::holds_alternative<double>(s.value)) {
                val = ConstantFP::get(Type::getDoubleTy(*context), std::get<double>(s.value));
                var_type = Type::getDoubleTy(*context);
            } else if (std::holds_alternative<bool>(s.value)) {
                val = ConstantInt::get(Type::getInt1Ty(*context), std::get<bool>(s.value));
                var_type = Type::getInt1Ty(*context);
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
            
            return nullptr;
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
            std::vector<Type*> param_types;
            for (size_t i = 0; i < s.params.size(); ++i) {
                param_types.push_back(Type::getInt32Ty(*context));
            }
            
            FunctionType* func_type = FunctionType::get(Type::getVoidTy(*context), param_types, false);
            Function* func = Function::Create(func_type, Function::ExternalLinkage, s.name, *module);
            functions[s.name] = func;
            
            Function* prev_func = current_function;
            current_function = func;
            
            BasicBlock* func_entry = BasicBlock::Create(*context, "entry", func);
            builder->SetInsertPoint(func_entry);
            
            // Set up parameter variables
            auto param_iter = func->arg_begin();
            for (size_t i = 0; i < s.params.size(); ++i, ++param_iter) {
                Argument* arg = &*param_iter;
                arg->setName(s.params[i]);
                
                AllocaInst* alloca = builder->CreateAlloca(Type::getInt32Ty(*context), nullptr, s.params[i]);
                builder->CreateStore(arg, alloca);
                variables[s.params[i]] = alloca;
            }
            
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
            
            // Convert arguments to LLVM values
            std::vector<Value*> args;
            for (size_t i = 0; i < s.arguments.size(); i++) {
                const auto& arg = s.arguments[i];
                Value* arg_val = nullptr;
                
                if (std::holds_alternative<int>(arg)) {
                    int int_val = std::get<int>(arg);
                    arg_val = ConstantInt::get(Type::getInt32Ty(*context), int_val);
                    
                    // Convert int to double for math functions
                    if (s.function_name == "abs" || s.function_name == "sqrt" || 
                        s.function_name == "max" || s.function_name == "min") {
                        arg_val = builder->CreateSIToFP(arg_val, Type::getDoubleTy(*context));
                    }
                } else if (std::holds_alternative<double>(arg)) {
                    arg_val = ConstantFP::get(Type::getDoubleTy(*context), std::get<double>(arg));
                } else if (std::holds_alternative<bool>(arg)) {
                    arg_val = ConstantInt::get(Type::getInt1Ty(*context), std::get<bool>(arg));
                } else if (std::holds_alternative<std::string>(arg)) {
                    arg_val = builder->CreateGlobalString(std::get<std::string>(arg));
                }
                
                if (arg_val) {
                    args.push_back(arg_val);
                }
            }
            
            return builder->CreateCall(func, args);
        }
        else if constexpr (std::is_same_v<T, IfStatement>) {
            // If statements not yet implemented
            return nullptr;
        }
        else if constexpr (std::is_same_v<T, WhileStatement>) {
            // While loops not yet implemented
            return nullptr;
        }
        else if constexpr (std::is_same_v<T, ModStatement>) {
            // Module statements are processed during module resolution phase
            // They don't generate runtime code directly
            return nullptr;
        }
        else if constexpr (std::is_same_v<T, ForStatement>) {
            // Set up loop structure
            BasicBlock* for_init = BasicBlock::Create(*context, "for.init", current_function);
            BasicBlock* for_cond = BasicBlock::Create(*context, "for.cond", current_function);
            BasicBlock* for_body = BasicBlock::Create(*context, "for.body", current_function);
            BasicBlock* for_inc = BasicBlock::Create(*context, "for.inc", current_function);
            BasicBlock* for_end = BasicBlock::Create(*context, "for.end", current_function);
            
            // Start the loop
            builder->CreateBr(for_init);
            
            // Initialization block
            builder->SetInsertPoint(for_init);
            // Setup loop counter
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
            // Inline assembly not yet implemented
            return nullptr;
        }
        else if constexpr (std::is_same_v<T, BinaryExpression>) {
            return codegen_binary_expr(s);
        }
        else if constexpr (std::is_same_v<T, UnaryExpression>) {
            return codegen_unary_expr(s);
        }
        // Note: Advanced AST types not yet implemented in ast_simple.hpp
        // TODO: Add support for StructDefinition, DereferenceExpression, etc.
        
        return nullptr;
    }, stmt);
}

Value* CodeGen::codegen_value(const std::variant<int, double, bool, std::string, TypedValue>& value) {
    if (std::holds_alternative<int>(value)) {
        return ConstantInt::get(Type::getInt32Ty(*context), std::get<int>(value));
    } else if (std::holds_alternative<double>(value)) {
        return ConstantFP::get(Type::getDoubleTy(*context), std::get<double>(value));
    } else if (std::holds_alternative<bool>(value)) {
        return ConstantInt::get(Type::getInt1Ty(*context), std::get<bool>(value));
    } else if (std::holds_alternative<std::string>(value)) {
        const std::string& str = std::get<std::string>(value);
        // Check if it's a variable name
        auto var_it = variables.find(str);
        if (var_it != variables.end()) {
            return builder->CreateLoad(Type::getInt32Ty(*context), var_it->second);
        }
        // Otherwise, treat as string literal
        return builder->CreateGlobalString(str);
    } else if (std::holds_alternative<TypedValue>(value)) {
        const TypedValue& typed_val = std::get<TypedValue>(value);
        
        switch (typed_val.type) {
            case SigType::U8:
                return ConstantInt::get(Type::getInt8Ty(*context), std::get<uint8_t>(typed_val.value));
            case SigType::U16:
                return ConstantInt::get(Type::getInt16Ty(*context), std::get<uint16_t>(typed_val.value));
            case SigType::U32:
                return ConstantInt::get(Type::getInt32Ty(*context), std::get<uint32_t>(typed_val.value));
            case SigType::U64:
                return ConstantInt::get(Type::getInt64Ty(*context), std::get<uint64_t>(typed_val.value));
            case SigType::I8:
                return ConstantInt::get(Type::getInt8Ty(*context), std::get<int8_t>(typed_val.value));
            case SigType::I16:
                return ConstantInt::get(Type::getInt16Ty(*context), std::get<int16_t>(typed_val.value));
            case SigType::I32:
                return ConstantInt::get(Type::getInt32Ty(*context), std::get<int32_t>(typed_val.value));
            case SigType::I64:
                return ConstantInt::get(Type::getInt64Ty(*context), std::get<int64_t>(typed_val.value));
            case SigType::Bool:
                return ConstantInt::get(Type::getInt1Ty(*context), std::get<bool>(typed_val.value));
            case SigType::Float:
                return ConstantFP::get(Type::getDoubleTy(*context), std::get<double>(typed_val.value));
            case SigType::String:
                return builder->CreateGlobalString(std::get<std::string>(typed_val.value));
            default:
                return nullptr;
        }
    }
    return nullptr;
}

Value* CodeGen::codegen_binary_expr(const BinaryExpression& expr) {
    Value* left = codegen_value(expr.left);
    Value* right = codegen_value(expr.right);
    
    if (!left || !right) {
        std::cerr << "Error: Unable to generate code for binary expression operands" << std::endl;
        return nullptr;
    }
    
    switch (expr.operator_type) {
        case SigBinaryOperator::Add:
            return builder->CreateAdd(left, right, "add");
        case SigBinaryOperator::Subtract:
            return builder->CreateSub(left, right, "sub");
        case SigBinaryOperator::Multiply:
            return builder->CreateMul(left, right, "mul");
        case SigBinaryOperator::Divide:
            return builder->CreateSDiv(left, right, "div");
        case SigBinaryOperator::Modulo:
            return builder->CreateSRem(left, right, "mod");
        case SigBinaryOperator::Equal:
            return builder->CreateICmpEQ(left, right, "eq");
        case SigBinaryOperator::NotEqual:
            return builder->CreateICmpNE(left, right, "ne");
        case SigBinaryOperator::LessThan:
            return builder->CreateICmpSLT(left, right, "lt");
        case SigBinaryOperator::LessThanEqual:
            return builder->CreateICmpSLE(left, right, "le");
        case SigBinaryOperator::GreaterThan:
            return builder->CreateICmpSGT(left, right, "gt");
        case SigBinaryOperator::GreaterThanEqual:
            return builder->CreateICmpSGE(left, right, "ge");
        case SigBinaryOperator::And:
            return builder->CreateAnd(left, right, "and");
        case SigBinaryOperator::Or:
            return builder->CreateOr(left, right, "or");
        case SigBinaryOperator::BitwiseAnd:
            return builder->CreateAnd(left, right, "bitwise_and");
        case SigBinaryOperator::BitwiseOr:
            return builder->CreateOr(left, right, "bitwise_or");
        case SigBinaryOperator::BitwiseXor:
            return builder->CreateXor(left, right, "bitwise_xor");
        case SigBinaryOperator::LeftShift:
            return builder->CreateShl(left, right, "left_shift");
        case SigBinaryOperator::RightShift:
            return builder->CreateAShr(left, right, "right_shift");
        default:
            std::cerr << "Error: Unsupported binary operator" << std::endl;
            return nullptr;
    }
}

Value* CodeGen::codegen_unary_expr(const UnaryExpression& expr) {
    Value* operand = codegen_value(expr.operand);
    
    if (!operand) {
        std::cerr << "Error: Unable to generate code for unary expression operand" << std::endl;
        return nullptr;
    }
    
    switch (expr.operator_type) {
        case SigBinaryOperator::Not:
            return builder->CreateNot(operand, "not");
        default:
            std::cerr << "Error: Unsupported unary operator" << std::endl;
            return nullptr;
    }
}

Value* CodeGen::codegen_expression(const Expression& expr) {
    return std::visit([this](const auto& e) -> Value* {
        using T = std::decay_t<decltype(e)>;
        
        if constexpr (std::is_same_v<T, int>) {
            return ConstantInt::get(Type::getInt32Ty(*context), e);
        }
        else if constexpr (std::is_same_v<T, double>) {
            return ConstantFP::get(Type::getDoubleTy(*context), e);
        }
        else if constexpr (std::is_same_v<T, bool>) {
            return ConstantInt::get(Type::getInt1Ty(*context), e);
        }
        else if constexpr (std::is_same_v<T, std::string>) {
            return builder->CreateGlobalString(e);
        }
        else if constexpr (std::is_same_v<T, BinaryExpression>) {
            return codegen_binary_expr(e);
        }
        else if constexpr (std::is_same_v<T, UnaryExpression>) {
            return codegen_unary_expr(e);
        }
        else if constexpr (std::is_same_v<T, TypedValue>) {
            // Handle typed values (like hex literals)
            switch (e.type) {
                case SigType::U8:
                    return ConstantInt::get(Type::getInt8Ty(*context), std::get<uint8_t>(e.value));
                case SigType::U16:
                    return ConstantInt::get(Type::getInt16Ty(*context), std::get<uint16_t>(e.value));
                case SigType::U32:
                    return ConstantInt::get(Type::getInt32Ty(*context), std::get<uint32_t>(e.value));
                case SigType::U64:
                    return ConstantInt::get(Type::getInt64Ty(*context), std::get<uint64_t>(e.value));
                case SigType::I8:
                    return ConstantInt::get(Type::getInt8Ty(*context), std::get<int8_t>(e.value));
                case SigType::I16:
                    return ConstantInt::get(Type::getInt16Ty(*context), std::get<int16_t>(e.value));
                case SigType::I32:
                    return ConstantInt::get(Type::getInt32Ty(*context), std::get<int32_t>(e.value));
                case SigType::I64:
                    return ConstantInt::get(Type::getInt64Ty(*context), std::get<int64_t>(e.value));
                default:
                    return nullptr;
            }
        }
        
        return nullptr;
    }, expr);
}

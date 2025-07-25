#include "templates.hpp"
#include <climits>
#include <iostream>

TemplateManager::TemplateManager() {
    init_instruction_templates();
}

void TemplateManager::init_instruction_templates() {
    init_syscall_templates();
    init_print_templates();
    init_variable_templates();
    init_function_templates();
    init_misc_templates();
}

void TemplateManager::init_syscall_templates() {
    // Generic syscall template
    templates.emplace_back(
        "syscall %rax,%rdi,%rsi,%rdx",
        [](const RTLInsn& insn) { return insn.op == RTLInsn::SYSCALL; },
        [](const RTLInsn& insn, CodeGenContext& ctx) {
            std::string code;
            if (insn.operands.size() >= 1) code += ctx.get_optimal_mov("rax", insn.operands[0]);
            if (insn.operands.size() >= 2) code += ctx.get_optimal_mov("rdi", insn.operands[1]);
            if (insn.operands.size() >= 3) code += ctx.get_optimal_mov("rsi", insn.operands[2]);
            if (insn.operands.size() >= 4) code += ctx.get_optimal_mov("rdx", insn.operands[3]);
            code += "    syscall\n";
            return code;
        }, 1, "Generic syscall"
    );
}

void TemplateManager::init_print_templates() {
    // Print string literal
    templates.emplace_back(
        "print_string_literal",
        [](const RTLInsn& insn) {
            return insn.op == RTLInsn::CALL && 
                   insn.attributes.count("builtin") && 
                   insn.attributes.at("builtin") == "print" &&
                   insn.attributes.count("type") && 
                   insn.attributes.at("type") == "string";
        },
        [](const RTLInsn& insn, CodeGenContext& ctx) {
            auto msg_label = ctx.new_string_label();
            auto content = insn.operands[0];
            ctx.emit_data(msg_label, "db \"" + content + "\", 0xA");
            
            return ctx.get_optimal_mov("rax", "1") +
                   ctx.get_optimal_mov("rdi", "1") +
                   "    mov rsi, " + msg_label + "\n" +
                   ctx.get_optimal_mov("rdx", std::to_string(content.length() + 1)) +
                   "    syscall\n";
        }, 1, "Print string literal"
    );
    
    // Print integer literal
    templates.emplace_back(
        "print_int_literal",
        [](const RTLInsn& insn) {
            return insn.op == RTLInsn::CALL && 
                   insn.attributes.count("builtin") && 
                   insn.attributes.at("builtin") == "print" &&
                   insn.attributes.count("type") && 
                   insn.attributes.at("type") == "int";
        },
        [](const RTLInsn& insn, CodeGenContext& ctx) {
            auto msg_label = ctx.new_string_label();
            auto content = insn.operands[0];
            ctx.emit_data(msg_label, "db \"" + content + "\", 0xA");
            
            return ctx.get_optimal_mov("rax", "1") +
                   ctx.get_optimal_mov("rdi", "1") +
                   "    mov rsi, " + msg_label + "\n" +
                   ctx.get_optimal_mov("rdx", std::to_string(content.length() + 1)) +
                   "    syscall\n";
        }, 1, "Print integer literal"
    );
    
    // Print integer variable
    templates.emplace_back(
        "print_int_variable",
        [](const RTLInsn& insn) {
            return insn.op == RTLInsn::CALL && 
                   insn.attributes.count("builtin") && 
                   insn.attributes.at("builtin") == "print" &&
                   insn.attributes.count("type") && 
                   insn.attributes.at("type") == "variable" &&
                   insn.attributes.count("var_type") &&
                   insn.attributes.at("var_type") == "int";
        },
        [](const RTLInsn& insn, CodeGenContext& ctx) {
            auto var_name = insn.operands[0];
            
            if (ctx.variable_registers.count(var_name)) {
                // Variable is in register - convert to string and print
                auto reg_name = ctx.reg_alloc.get_register_name(ctx.variable_registers[var_name]);
                return "    ; Print integer variable " + var_name + " from " + reg_name + "\n" +
                       "    ; TODO: Implement integer-to-string conversion for register values\n" +
                       "    ; For now, this is a placeholder\n";
            } else {
                // Variable is in memory
                return "    ; Print integer variable " + var_name + "\n" +
                       "    mov eax, [" + var_name + "]\n" +
                       "    ; TODO: Convert integer to string and print\n" +
                       "    ; For now, this is a placeholder\n";
            }
        }, 1, "Print integer variable"
    );
    
    // Print string variable
    templates.emplace_back(
        "print_string_variable",
        [](const RTLInsn& insn) {
            return insn.op == RTLInsn::CALL && 
                   insn.attributes.count("builtin") && 
                   insn.attributes.at("builtin") == "print" &&
                   insn.attributes.count("type") && 
                   insn.attributes.at("type") == "variable" &&
                   insn.attributes.count("var_type") &&
                   insn.attributes.at("var_type") == "string";
        },
        [](const RTLInsn& insn, CodeGenContext& ctx) {
            auto var_name = insn.operands[0];
            auto len_label = var_name + "_len";
            
            return ctx.get_optimal_mov("rax", "1") +
                   ctx.get_optimal_mov("rdi", "1") +
                   "    mov rsi, " + var_name + "\n" +
                   "    mov edx, [" + len_label + "]\n" +
                   "    syscall\n" +
                   "    ; Print newline\n" +
                   ctx.get_optimal_mov("rax", "1") +
                   ctx.get_optimal_mov("rdi", "1") +
                   "    mov rsi, newline\n" +
                   ctx.get_optimal_mov("rdx", "1") +
                   "    syscall\n";
        }, 1, "Print string variable"
    );
    
    // Generic variable print (fallback)
    templates.emplace_back(
        "print_variable_generic",
        [](const RTLInsn& insn) {
            return insn.op == RTLInsn::CALL && 
                   insn.attributes.count("builtin") && 
                   insn.attributes.at("builtin") == "print" &&
                   insn.attributes.count("type") && 
                   insn.attributes.at("type") == "variable";
        },
        [](const RTLInsn& insn, CodeGenContext& ctx) {
            auto var_name = insn.operands[0];
            
            // Check if we know the variable type from context
            if (ctx.variable_types.count(var_name)) {
                auto var_type = ctx.variable_types[var_name];
                if (var_type == "string") {
                    auto len_label = var_name + "_len";
                    return ctx.get_optimal_mov("rax", "1") +
                           ctx.get_optimal_mov("rdi", "1") +
                           "    mov rsi, " + var_name + "\n" +
                           "    mov edx, [" + len_label + "]\n" +
                           "    syscall\n" +
                           "    ; Print newline\n" +
                           ctx.get_optimal_mov("rax", "1") +
                           ctx.get_optimal_mov("rdi", "1") +
                           "    mov rsi, newline\n" +
                           ctx.get_optimal_mov("rdx", "1") +
                           "    syscall\n";
                } else if (var_type == "int") {
                    if (ctx.variable_registers.count(var_name)) {
                        auto reg_name = ctx.reg_alloc.get_register_name(ctx.variable_registers[var_name]);
                        return "    ; Print integer variable " + var_name + " from " + reg_name + " (simplified)\n" +
                               "    ; Integer printing not fully implemented\n";
                    } else {
                        return "    ; Print integer variable " + var_name + " (simplified)\n" +
                               "    mov eax, [" + var_name + "]\n" +
                               "    ; Integer printing not fully implemented\n";
                    }
                }
            }
            
            // Fallback: treat as string with fixed length
            return "    ; Print variable " + var_name + " (unknown type, treating as string)\n" +
                   ctx.get_optimal_mov("rax", "1") +
                   ctx.get_optimal_mov("rdi", "1") +
                   "    mov rsi, " + var_name + "\n" +
                   ctx.get_optimal_mov("rdx", "64") +
                   "    syscall\n"; // Assume max 64 chars
        }, 2, "Print variable (generic fallback)"
    );
}

void TemplateManager::init_variable_templates() {
    // Variable assignment with register allocation
    templates.emplace_back(
        "var_assign_reg",
        [](const RTLInsn& insn) {
            return insn.op == RTLInsn::DATA && 
                   insn.attributes.count("type") && 
                   insn.attributes.at("type") == "assignment";
        },
        [](const RTLInsn& insn, CodeGenContext& ctx) {
            auto var_name = insn.operands[0];
            auto value = insn.operands[1];
            auto value_type = insn.attributes.count("value_type") ? 
                            insn.attributes.at("value_type") : "unknown";
            
            // Try to allocate register for frequently used variables
            if (ctx.optimization_level > 0) {
                Register reg = ctx.reg_alloc.allocate_register(var_name);
                ctx.variable_registers[var_name] = reg;
                
                if (value_type == "int") {
                    ctx.variable_types[var_name] = "int";
                    return "; Register allocation: " + var_name + " -> " + 
                           ctx.reg_alloc.get_register_name(reg) + "\n" +
                           ctx.get_optimal_mov(ctx.reg_alloc.get_register_name(reg), value);
                }
            }
            
            // Fallback to memory
            if (value_type == "int") {
                ctx.variable_types[var_name] = "int";
                ctx.emit_data(var_name, "dd " + value);
            } else if (value_type == "string") {
                ctx.variable_types[var_name] = "string";
                auto len_label = var_name + "_len";
                ctx.emit_data(var_name, "db \"" + value + "\"");
                ctx.emit_data(len_label, "dd " + std::to_string(value.length()));
            }
            
            return "; Variable " + var_name + " = " + value + " (type: " + value_type + ")\n";
        }, 1, "Variable assignment with register allocation"
    );
    
    // Variable declaration (no assignment)
    templates.emplace_back(
        "var_declaration",
        [](const RTLInsn& insn) {
            return insn.op == RTLInsn::DATA && 
                   insn.attributes.count("type") && 
                   insn.attributes.at("type") == "declaration";
        },
        [](const RTLInsn& insn, CodeGenContext& ctx) {
            auto var_name = insn.operands[0];
            
            // Reserve space in BSS section for uninitialized variables
            ctx.emit_bss(var_name, "resd 1  ; Uninitialized variable");
            ctx.variable_types[var_name] = "uninitialized";
            
            return "; Variable declaration: " + var_name + "\n";
        }, 1, "Variable declaration"
    );
}

void TemplateManager::init_function_templates() {
    // Function definitions with proper register saving
    templates.emplace_back(
        "function_def_optimized",
        [](const RTLInsn& insn) {
            return insn.op == RTLInsn::LABEL && 
                   insn.attributes.count("type") && 
                   insn.attributes.at("type") == "function";
        },
        [](const RTLInsn& insn, CodeGenContext& ctx) {
            ctx.in_function = true;
            ctx.current_function = insn.operands[0];
            
            std::string prologue = "\n" + insn.operands[0] + ":\n" +
                                 "    push rbp\n" +
                                 "    mov rbp, rsp\n";
            
            // Save callee-saved registers that we'll use
            auto callee_saved = ctx.reg_alloc.get_callee_saved_used();
            for (Register reg : callee_saved) {
                prologue += "    push " + ctx.reg_alloc.get_register_name(reg) + "\n";
            }
            
            return prologue;
        }, 1, "Optimized function definition"
    );
    
    // Function call
    templates.emplace_back(
        "function_call",
        [](const RTLInsn& insn) {
            return insn.op == RTLInsn::CALL && 
                   insn.attributes.count("type") && 
                   insn.attributes.at("type") == "function";
        },
        [](const RTLInsn& insn, CodeGenContext& ctx) {
            return "    call " + insn.operands[0] + "\n";
        }, 1, "Function call"
    );
    
    // Function return
    templates.emplace_back(
        "function_return",
        [](const RTLInsn& insn) {
            return insn.op == RTLInsn::RETURN && 
                   insn.attributes.count("type") && 
                   insn.attributes.at("type") == "function";
        },
        [](const RTLInsn& insn, CodeGenContext& ctx) {
            ctx.in_function = false;
            ctx.current_function.clear();
            return std::string("    mov rsp, rbp\n") +
                   "    pop rbp\n" +
                   "    ret\n";
        }, 1, "Function return"
    );
}

void TemplateManager::init_misc_templates() {
    // If statement start (condition check and conditional jump)
    templates.emplace_back(
        "if_statement_start",
        [](const RTLInsn& insn) { return insn.op == RTLInsn::IF_START; },
        [](const RTLInsn& insn, CodeGenContext& ctx) -> std::string {
            if (insn.operands.size() < 3) {
                return std::string("; Error: If statement needs left, op, right operands\n");
            }
            
            auto left = insn.operands[0];
            auto op = insn.operands[1];
            auto right = insn.operands[2];
            auto if_end_label = insn.attributes.count("if_end_label") ? 
                              insn.attributes.at("if_end_label") : "if_end";
            auto else_label = insn.attributes.count("else_label") ? 
                            insn.attributes.at("else_label") : "";
            
            // Check if this is an elif condition
            bool is_elif_condition = insn.attributes.count("is_elif_condition") && 
                                   insn.attributes.at("is_elif_condition") == "true";
            
            std::string code = is_elif_condition ? 
                              "; Elif condition: " + left + " " + op + " " + right + "\n" :
                              "; If statement: " + left + " " + op + " " + right + "\n";
            
            // Load left operand into register
            if (std::isdigit(left[0]) || (left[0] == '-' && std::isdigit(left[1]))) {
                // Left is a number literal
                code += ctx.get_optimal_mov("eax", left);
            } else {
                // Left is a variable - check if it's in register or memory
                if (ctx.variable_registers.count(left)) {
                    auto reg = ctx.reg_alloc.get_register_name(ctx.variable_registers[left]);
                    // Convert 64-bit register to 32-bit for comparison
                    std::string reg32 = reg;
                    if (reg.substr(0, 1) == "r" && reg.length() > 2) {
                        reg32 = reg + "d"; // r15 -> r15d, etc.
                    }
                    code += "    mov eax, " + reg32 + "\n";
                } else {
                    code += "    mov eax, [" + left + "]\n";
                }
            }
            
            // Compare with right operand
            if (std::isdigit(right[0]) || (right[0] == '-' && std::isdigit(right[1]))) {
                // Right is a number literal
                code += "    cmp eax, " + right + "\n";
            } else {
                // Right is a variable
                if (ctx.variable_registers.count(right)) {
                    auto reg = ctx.reg_alloc.get_register_name(ctx.variable_registers[right]);
                    // Convert 64-bit register to 32-bit for comparison
                    std::string reg32 = reg;
                    if (reg.substr(0, 1) == "r" && reg.length() > 2) {
                        reg32 = reg + "d"; // r15 -> r15d, etc.
                    }
                    code += "    cmp eax, " + reg32 + "\n";
                } else {
                    code += "    cmp eax, [" + right + "]\n";
                }
            }
            
            // Generate conditional jump based on operator
            std::string jump_target = else_label.empty() ? if_end_label : else_label;
            
            if (op == "==") {
                code += "    jne " + jump_target + "\n";
            } else if (op == "!=") {
                code += "    je " + jump_target + "\n";
            } else if (op == "<") {
                code += "    jge " + jump_target + "\n";
            } else if (op == "<=") {
                code += "    jg " + jump_target + "\n";
            } else if (op == ">") {
                code += "    jle " + jump_target + "\n";
            } else if (op == ">=") {
                code += "    jl " + jump_target + "\n";
            } else {
                code += "; Unknown operator: " + op + "\n";
            }
            
            return code;
        }, 1, "If statement condition check"
    );
    
    // Else block start (jump over then block)
    templates.emplace_back(
        "else_statement_start",
        [](const RTLInsn& insn) { return insn.op == RTLInsn::ELSE_START; },
        [](const RTLInsn& insn, CodeGenContext& ctx) {
            auto if_end_label = insn.attributes.count("if_end_label") ? 
                              insn.attributes.at("if_end_label") : "if_end";
            auto else_label = insn.attributes.count("else_label") ? 
                            insn.attributes.at("else_label") : "else";
            
            // Check if this is an elif clause
            bool is_elif = insn.attributes.count("is_elif") && 
                          insn.attributes.at("is_elif") == "true";
            
            if (is_elif) {
                // For elif, just place the label (no jump needed as previous condition handles it)
                return "    jmp " + if_end_label + "\n" + else_label + ":\n";
            } else {
                // For regular else, jump over then block and place else label
                return "    jmp " + if_end_label + "\n" + else_label + ":\n";
            }
        }, 1, "Else/Elif block start"
    );
    
    // If statement end (place end label)
    templates.emplace_back(
        "if_statement_end",
        [](const RTLInsn& insn) { return insn.op == RTLInsn::IF_END; },
        [](const RTLInsn& insn, CodeGenContext& ctx) {
            auto if_end_label = insn.attributes.count("if_end_label") ? 
                              insn.attributes.at("if_end_label") : "if_end";
            return if_end_label + ":\n";
        }, 1, "If statement end label"
    );
    
    // While loop start (label and condition check)
    templates.emplace_back(
        "while_statement_start",
        [](const RTLInsn& insn) { return insn.op == RTLInsn::WHILE_START; },
        [](const RTLInsn& insn, CodeGenContext& ctx) -> std::string {
            auto while_start_label = insn.attributes.count("while_start_label") ? 
                                   insn.attributes.at("while_start_label") : "while_start";
            auto while_end_label = insn.attributes.count("while_end_label") ? 
                                 insn.attributes.at("while_end_label") : "while_end";
            
            std::string code = while_start_label + ":\n";
            
            // Handle different condition types
            if (insn.operands.size() >= 3 && !insn.operands[1].empty()) {
                // Binary comparison: while (left op right)
                auto left = insn.operands[0];
                auto op = insn.operands[1];
                auto right = insn.operands[2];
                
                code += "; While condition: " + left + " " + op + " " + right + "\n";
                
                // Load left operand into register
                if (std::isdigit(left[0]) || (left[0] == '-' && std::isdigit(left[1]))) {
                    code += ctx.get_optimal_mov("eax", left);
                } else {
                    if (ctx.variable_registers.count(left)) {
                        auto reg = ctx.reg_alloc.get_register_name(ctx.variable_registers[left]);
                        std::string reg32 = reg;
                        if (reg.substr(0, 1) == "r" && reg.length() > 2) {
                            reg32 = reg + "d";
                        }
                        code += "    mov eax, " + reg32 + "\n";
                    } else {
                        code += "    mov eax, [" + left + "]\n";
                    }
                }
                
                // Compare with right operand
                if (std::isdigit(right[0]) || (right[0] == '-' && std::isdigit(right[1]))) {
                    code += "    cmp eax, " + right + "\n";
                } else {
                    if (ctx.variable_registers.count(right)) {
                        auto reg = ctx.reg_alloc.get_register_name(ctx.variable_registers[right]);
                        std::string reg32 = reg;
                        if (reg.substr(0, 1) == "r" && reg.length() > 2) {
                            reg32 = reg + "d";
                        }
                        code += "    cmp eax, " + reg32 + "\n";
                    } else {
                        code += "    cmp eax, [" + right + "]\n";
                    }
                }
                
                // Generate conditional jump to end loop
                if (op == "==") {
                    code += "    jne " + while_end_label + "\n";
                } else if (op == "!=") {
                    code += "    je " + while_end_label + "\n";
                } else if (op == "<") {
                    code += "    jge " + while_end_label + "\n";
                } else if (op == "<=") {
                    code += "    jg " + while_end_label + "\n";
                } else if (op == ">") {
                    code += "    jle " + while_end_label + "\n";
                } else if (op == ">=") {
                    code += "    jl " + while_end_label + "\n";
                }
            } else if (insn.operands.size() >= 1) {
                // Single operand condition: while (x) or while (1)
                auto condition = insn.operands[0];
                code += "; While condition: " + condition + "\n";
                
                if (std::isdigit(condition[0]) || (condition[0] == '-' && std::isdigit(condition[1]))) {
                    // Numeric literal
                    if (condition == "0") {
                        // while (0) - never execute
                        code += "    jmp " + while_end_label + "\n";
                    }
                    // while (non-zero) - always execute (infinite loop), no condition check needed
                } else {
                    // Variable condition
                    if (ctx.variable_registers.count(condition)) {
                        auto reg = ctx.reg_alloc.get_register_name(ctx.variable_registers[condition]);
                        std::string reg32 = reg;
                        if (reg.substr(0, 1) == "r" && reg.length() > 2) {
                            reg32 = reg + "d";
                        }
                        code += "    cmp " + reg32 + ", 0\n";
                    } else {
                        code += "    cmp dword [" + condition + "], 0\n";
                    }
                    code += "    je " + while_end_label + "\n";
                }
            }
            
            return code;
        }, 1, "While loop start"
    );
    
    // While loop end (jump back to start and place end label)
    templates.emplace_back(
        "while_statement_end",
        [](const RTLInsn& insn) { return insn.op == RTLInsn::WHILE_END; },
        [](const RTLInsn& insn, CodeGenContext& ctx) {
            auto while_start_label = insn.attributes.count("while_start_label") ? 
                                   insn.attributes.at("while_start_label") : "while_start";
            auto while_end_label = insn.attributes.count("while_end_label") ? 
                                 insn.attributes.at("while_end_label") : "while_end";
            
            return "    jmp " + while_start_label + "\n" + while_end_label + ":\n";
        }, 1, "While loop end"
    );
    
    // For loop start (initialization, label and condition check)
    templates.emplace_back(
        "for_statement_start",
        [](const RTLInsn& insn) { return insn.op == RTLInsn::FOR_START; },
        [](const RTLInsn& insn, CodeGenContext& ctx) -> std::string {
            auto for_start_label = insn.attributes.count("for_start_label") ? 
                                 insn.attributes.at("for_start_label") : "for_start";
            auto for_end_label = insn.attributes.count("for_end_label") ? 
                               insn.attributes.at("for_end_label") : "for_end";
            
            if (insn.operands.size() < 3) {
                return "; Error: for loop missing operands\n";
            }
            
            auto loop_var = insn.operands[0];
            auto condition = insn.operands[1];
            auto count = insn.operands[2];
            
            std::string code = "";
            
            // Store loop variable for the FOR_END template
            ctx.current_for_variable.push_back(loop_var);
            
            // Initialize loop variable
            code += "; For loop initialization: " + loop_var + " = 1\n";
            ctx.emit_variable(loop_var, "dd 1");
            code += ctx.get_optimal_mov("eax", "1");
            code += "    mov [" + loop_var + "], eax\n";
            
            // Loop start label
            code += for_start_label + ":\n";
            
            // Condition check: loop_var <= count
            code += "; For condition: " + loop_var + " <= " + count + "\n";
            code += "    mov eax, [" + loop_var + "]\n";
            code += "    cmp eax, " + count + "\n";
            code += "    jg " + for_end_label + "\n";
            
            return code;
        }, 1, "For loop start"
    );
    
    // For loop end (increment and jump back to start)
    templates.emplace_back(
        "for_statement_end",
        [](const RTLInsn& insn) { return insn.op == RTLInsn::FOR_END; },
        [](const RTLInsn& insn, CodeGenContext& ctx) {
            auto for_start_label = insn.attributes.count("for_start_label") ? 
                                 insn.attributes.at("for_start_label") : "for_start";
            auto for_end_label = insn.attributes.count("for_end_label") ? 
                               insn.attributes.at("for_end_label") : "for_end";
            
            std::string code = "";
            
            // We need to track the loop variable better
            // For the example for(i,1,100), the variable name is 'i'
            // We'll store this in the context during FOR_START processing
            std::string loop_var = "i"; // Default fallback
            if (ctx.current_for_variable.size() > 0) {
                loop_var = ctx.current_for_variable.back();
                ctx.current_for_variable.pop_back();
            }
            
            code += "; For loop increment\n";
            code += "    mov eax, [" + loop_var + "]\n";
            code += "    inc eax\n";
            code += "    mov [" + loop_var + "], eax\n";
            code += "    jmp " + for_start_label + "\n";
            code += for_end_label + ":\n";
            
            return code;
        }, 1, "For loop end"
    );
    
    // Optimized program exit
    templates.emplace_back(
        "exit_optimized",
        [](const RTLInsn& insn) { 
            return insn.op == RTLInsn::RETURN && 
                   insn.attributes.find("type") == insn.attributes.end(); 
        },
        [](const RTLInsn& insn, CodeGenContext& ctx) {
            auto exit_code = insn.operands.empty() ? "0" : insn.operands[0];
            return ctx.get_optimal_mov("rax", "60") +
                   ctx.get_optimal_mov("rdi", exit_code) +
                   "    syscall\n";
        }, 1, "Optimized program exit"
    );
    
    // Inline assembly
    templates.emplace_back(
        "inline_asm",
        [](const RTLInsn& insn) { return insn.op == RTLInsn::INLINE_ASM; },
        [](const RTLInsn& insn, CodeGenContext& ctx) {
            return "    " + insn.operands[0] + "\n";
        }, 1, "Inline assembly"
    );
}

const InstructionTemplate* TemplateManager::select_template(const RTLInsn& insn) {
    const InstructionTemplate* best_template = nullptr;
    int best_cost = INT_MAX;
    
    for (const auto& tmpl : templates) {
        if (tmpl.matcher(insn) && tmpl.cost < best_cost) {
            best_template = &tmpl;
            best_cost = tmpl.cost;
        }
    }
    
    return best_template;
}

void TemplateManager::add_template(const InstructionTemplate& tmpl) {
    templates.push_back(tmpl);
}

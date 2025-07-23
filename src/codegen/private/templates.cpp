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

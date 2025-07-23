#include <codegen/public/codegen.hpp>
#include "templates.hpp"
#include "optimization.hpp"
#include "context.hpp"
#include "rtl.hpp"
#include "../utils/utils.hpp"
#include <sstream>
#include <vector>
#include <functional>
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <climits>
#include <memory>
#include <queue>

// Template-based code generator
class TemplateCodeGen {
private:
    OptimizationManager opt_manager;
    TemplateManager template_manager;
    
    template<typename T>
    void add_print_insn(std::vector<RTLInsn>& rtl_insns, const T& value, const std::string& type) {
        RTLInsn insn(RTLInsn::CALL);
        if constexpr (std::is_same_v<T, int>) {
            insn.operands.push_back(std::to_string(value));
        } else {
            insn.operands.push_back(value);
        }
        insn.attributes["builtin"] = "print";
        insn.attributes["type"] = type;
        rtl_insns.push_back(std::move(insn));
    }
    
    void process_ast_node(const ASTNode& node, std::vector<RTLInsn>& rtl_insns, CodeGenContext& ctx) {
        std::visit([&](auto&& stmt) {
            using T = std::decay_t<decltype(stmt)>;
            
            if constexpr (std::is_same_v<T, ReturnStatement>) {
                rtl_insns.emplace_back(RTLInsn::RETURN, std::vector<std::string>{std::to_string(stmt.value)});
            }
            else if constexpr (std::is_same_v<T, PrintStatement>) {
                std::visit([&](auto&& value) {
                    using ValueType = std::decay_t<decltype(value)>;
                    RTLInsn insn(RTLInsn::CALL);
                    if constexpr (std::is_same_v<ValueType, int>) {
                        insn.operands.push_back(std::to_string(value));
                        insn.attributes["type"] = "int";
                    } else {
                        insn.operands.push_back(value);
                        insn.attributes["type"] = "string";
                    }
                    insn.attributes["builtin"] = "print";
                    rtl_insns.push_back(std::move(insn));
                }, stmt.value);
            }
            else if constexpr (std::is_same_v<T, PrintVariable>) {
                RTLInsn insn(RTLInsn::CALL, {stmt.variableName});
                insn.attributes["builtin"] = "print";
                insn.attributes["type"] = "variable";
                
                if (ctx.variable_types.count(stmt.variableName)) {
                    insn.attributes["var_type"] = ctx.variable_types[stmt.variableName];
                }
                
                rtl_insns.push_back(std::move(insn));
            }
            else if constexpr (std::is_same_v<T, AsmStatement>) {
                rtl_insns.emplace_back(RTLInsn::INLINE_ASM, std::vector<std::string>{stmt.value});
            }
            else if constexpr (std::is_same_v<T, FunctionDefinition>) {
                RTLInsn label_insn(RTLInsn::LABEL, {stmt.name});
                label_insn.attributes["type"] = "function";
                rtl_insns.push_back(std::move(label_insn));
                
                for (const auto& body_stmt : stmt.body) {
                    process_ast_node(body_stmt, rtl_insns, ctx);
                }
                
                RTLInsn ret_insn(RTLInsn::RETURN);
                ret_insn.attributes["type"] = "function";
                rtl_insns.push_back(std::move(ret_insn));
            }
            else if constexpr (std::is_same_v<T, FunctionCall>) {
                RTLInsn insn(RTLInsn::CALL, {stmt.function_name});
                insn.attributes["type"] = "function";
                rtl_insns.push_back(std::move(insn));
            }
            else if constexpr (std::is_same_v<T, VariableDeclaration>) {
                RTLInsn insn(RTLInsn::DATA, {stmt.var_name});
                insn.attributes["type"] = "declaration";
                rtl_insns.push_back(std::move(insn));
            }
            else if constexpr (std::is_same_v<T, VariableAssignment>) {
                RTLInsn insn(RTLInsn::DATA, {stmt.var_name});
                
                std::visit([&](auto&& value) {
                    using ValueType = std::decay_t<decltype(value)>;
                    if constexpr (std::is_same_v<ValueType, int>) {
                        insn.operands.push_back(std::to_string(value));
                        insn.attributes["value_type"] = "int";
                    } else if constexpr (std::is_same_v<ValueType, std::string>) {
                        insn.operands.push_back(value);
                        insn.attributes["value_type"] = "string";
                    } else if constexpr (std::is_same_v<ValueType, double>) {
                        insn.operands.push_back(std::to_string(value));
                        insn.attributes["value_type"] = "double";
                    } else if constexpr (std::is_same_v<ValueType, float>) {
                        insn.operands.push_back(std::to_string(value));
                        insn.attributes["value_type"] = "float";
                    }
                }, stmt.value);
                
                insn.attributes["type"] = "assignment";
                rtl_insns.push_back(std::move(insn));
            }
            else if constexpr (std::is_same_v<T, IfStatement>) {
                // Generate unique labels for this if statement
                auto if_end_label = ctx.new_label("if_end");
                auto else_label = stmt.elseBlock ? ctx.new_label("else") : "";
                
                // Create if start instruction with condition info
                RTLInsn if_start(RTLInsn::IF_START, {stmt.left, stmt.op, stmt.right});
                if_start.attributes["if_end_label"] = if_end_label;
                if (stmt.elseBlock) {
                    if_start.attributes["else_label"] = else_label;
                }
                rtl_insns.push_back(std::move(if_start));
                
                // Process then block
                for (const auto& then_stmt : stmt.thenBlock) {
                    process_ast_node(then_stmt, rtl_insns, ctx);
                }
                
                // If there's an else block, add else start
                if (stmt.elseBlock) {
                    RTLInsn else_start(RTLInsn::ELSE_START);
                    else_start.attributes["else_label"] = else_label;
                    else_start.attributes["if_end_label"] = if_end_label;
                    rtl_insns.push_back(std::move(else_start));
                    
                    // Process else block
                    for (const auto& else_stmt : stmt.elseBlock.value()) {
                        process_ast_node(else_stmt, rtl_insns, ctx);
                    }
                }
                
                // Add if end marker
                RTLInsn if_end(RTLInsn::IF_END);
                if_end.attributes["if_end_label"] = if_end_label;
                rtl_insns.push_back(std::move(if_end));
            }
        }, node);
    }
    
public:
    std::vector<RTLInsn> lower_to_rtl(const AST& ast) {
        std::vector<RTLInsn> rtl_insns;
        rtl_insns.reserve(ast.size() * 2);
        
        CodeGenContext temp_ctx;
        
        for (const auto& node : ast) {
            process_ast_node(node, rtl_insns, temp_ctx);
        }
        
        return rtl_insns;
    }
    
    std::string generate_assembly(const std::vector<RTLInsn>& rtl_insns, int opt_level = 2) {
        CodeGenContext ctx;
        ctx.optimization_level = opt_level;
        
        // Make a copy for optimization
        std::vector<RTLInsn> optimized_insns = rtl_insns;
        
        // Run optimization passes
        if (opt_level > 0) {
            opt_manager.run_optimizations(optimized_insns);
        }
        
        std::ostringstream main_code, function_code;
        bool in_function = false;
        
        // Add common data
        ctx.emit_data("newline", "db 0xA");
        
        for (const auto& insn : optimized_insns) {
            const InstructionTemplate* tmpl = template_manager.select_template(insn);
            if (!tmpl) {
                std::cerr << "Warning: No template found for instruction type " << insn.op << std::endl;
                continue;
            }
            
            std::string code = tmpl->generator(insn, ctx);
            
            // Function organization
            if (insn.op == RTLInsn::LABEL && insn.attributes.count("type") && 
                insn.attributes.at("type") == "function") {
                in_function = true;
                function_code << code;
            }
            else if (insn.op == RTLInsn::RETURN && insn.attributes.count("type") && 
                     insn.attributes.at("type") == "function") {
                // Add register restoration for functions
                auto callee_saved = ctx.reg_alloc.get_callee_saved_used();
                for (auto it = callee_saved.rbegin(); it != callee_saved.rend(); ++it) {
                    function_code << "    pop " << ctx.reg_alloc.get_register_name(*it) << "\n";
                }
                function_code << "    mov rsp, rbp\n    pop rbp\n    ret\n";
                in_function = false;
            }
            else if (in_function) {
                function_code << code;
            } else {
                main_code << code;
            }
        }
        
        // Assemble final output
        std::ostringstream output;
        
        // Data section
        output << "section .data\n";
        if (!ctx.data_section.str().empty()) {
            output << ctx.data_section.str();
        }
        
        // BSS section
        if (!ctx.bss_section.str().empty()) {
            output << "\nsection .bss\n";
            output << ctx.bss_section.str();
        }
        
        // Text section
        output << "\nsection .text\n";
        output << "global _start\n\n";
        
        // Main program
        output << "_start:\n";
        output << main_code.str();
        
        // Functions
        if (!function_code.str().empty()) {
            output << "\n; Function definitions\n";
            output << function_code.str();
        }
        
        return output.str();
    }
};

// Public interface
std::string generate_asm(const AST& ast) {
    if (ast.empty()) {
        return "section .text\nglobal _start\n_start:\n    xor rax, rax\n    mov rdi, 60\n    syscall\n";
    }
    
    TemplateCodeGen codegen;
    auto rtl_insns = codegen.lower_to_rtl(ast);
    
    return codegen.generate_assembly(rtl_insns, 2); // Optimization level 2
}
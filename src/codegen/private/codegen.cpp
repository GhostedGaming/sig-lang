#include <codegen/public/codegen.hpp>
#include <sstream>
#include <vector>
#include <functional>
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <climits>

// RTL Instruction representation
struct RTLInsn {
    enum OpType { SET, CALL, RETURN, LABEL, DATA, INLINE_ASM, LOAD, STORE };
    
    OpType op;
    std::vector<std::string> operands;
    std::unordered_map<std::string, std::string> attributes;
    
    RTLInsn(OpType operation, std::vector<std::string> ops = {}) 
        : op(operation), operands(std::move(ops)) {}
};

// Code generation context
class CodeGenContext {
public:
    std::ostringstream data_section, text_section, bss_section;
    std::unordered_map<std::string, std::string> variable_types; // Track variable types
    std::unordered_map<std::string, std::string> string_labels; // Map strings to labels
    int next_label_id = 0;
    int next_string_id = 0;
    bool in_function = false;
    std::string current_function;
    
    std::string new_label(const std::string& prefix = "L") {
        return prefix + std::to_string(next_label_id++);
    }
    
    std::string new_string_label() {
        return "str" + std::to_string(next_string_id++);
    }
    
    void emit_data(const std::string& label, const std::string& data) {
        data_section << label << ": " << data << "\n";
    }
    
    void emit_bss(const std::string& label, const std::string& data) {
        bss_section << label << ": " << data << "\n";
    }
    
    // Helper to get string length at compile time
    std::string get_string_length(const std::string& str) {
        return std::to_string(str.length());
    }
};

// Pattern-based code generator - FIXED VERSION
class PatternCodeGen {
private:
    using PatternFunc = std::function<std::string(const RTLInsn&, CodeGenContext&)>;
    using PredicateFunc = std::function<bool(const RTLInsn&)>;
    
    struct Pattern {
        PredicateFunc predicate;
        PatternFunc generator;
        int cost;
        std::string description; // For debugging
    };
    
    std::vector<Pattern> patterns;
    
    void define_patterns() {
        // Linux exit syscall
        patterns.push_back({
            [](const RTLInsn& insn) { 
                return insn.op == RTLInsn::RETURN && 
                       insn.attributes.find("type") == insn.attributes.end(); 
            },
            [](const RTLInsn& insn, CodeGenContext& ctx) {
                return "    mov rax, 60\n    mov rdi, " + 
                       (insn.operands.empty() ? "0" : insn.operands[0]) + 
                       "\n    syscall\n";
            }, 1, "Program exit"
        });
        
        // Print string literal
        patterns.push_back({
            [](const RTLInsn& insn) {
                return insn.op == RTLInsn::CALL && 
                       insn.attributes.count("builtin") && 
                       insn.attributes.at("builtin") == "print" &&
                       insn.attributes.count("type") && 
                       insn.attributes.at("type") == "string";
            },
            [](const RTLInsn& insn, CodeGenContext& ctx) {
                auto msg_label = ctx.new_string_label();
                auto len_value = ctx.get_string_length(insn.operands[0]);
                
                ctx.emit_data(msg_label, "db \"" + insn.operands[0] + "\", 0xA");

                return "    mov rax, 1\n    mov rdi, 1\n    mov rsi, " + msg_label + 
                       "\n    mov rdx, " + std::to_string(std::stoi(len_value) + 1) + "\n    syscall\n";
            }, 1, "Print string literal"
        });
        
        // Print integer literal
        patterns.push_back({
            [](const RTLInsn& insn) {
                return insn.op == RTLInsn::CALL && 
                       insn.attributes.count("builtin") && 
                       insn.attributes.at("builtin") == "print" &&
                       insn.attributes.count("type") && 
                       insn.attributes.at("type") == "int";
            },
            [](const RTLInsn& insn, CodeGenContext& ctx) {
                // Convert integer to string for printing
                auto msg_label = ctx.new_string_label();
                auto len_value = ctx.get_string_length(insn.operands[0]);
                
                ctx.emit_data(msg_label, "db \"" + insn.operands[0] + "\", 0xA");
                
                return "    mov rax, 1\n    mov rdi, 1\n    mov rsi, " + msg_label + 
                       "\n    mov rdx, " + std::to_string(std::stoi(len_value) + 1) + "\n    syscall\n";
            }, 1, "Print integer literal"
        });
        
        // Print integer variable
        patterns.push_back({
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
                // For integer variables, we need to convert to string first
                // This is a simplified approach - in a real compiler you'd have
                // a proper integer-to-string conversion routine
                auto var_name = insn.operands[0];
                
                return "    ; Print integer variable " + var_name + "\n" +
                       "    mov eax, [" + var_name + "]\n" +
                       "    ; TODO: Convert integer to string and print\n" +
                       "    ; For now, this is a placeholder\n";
            }, 1, "Print integer variable"
        });
        
        // Print string variable - FIXED VERSION
        patterns.push_back({
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
                
                return "    mov rax, 1\n    mov rdi, 1\n    mov rsi, " + var_name + 
                       "\n    mov edx, [" + len_label + "]\n    syscall\n" +  // Use edx for 32-bit load
                       "    ; Print newline\n" +
                       "    mov rax, 1\n    mov rdi, 1\n    mov rsi, newline\n    mov rdx, 1\n    syscall\n";
            }, 1, "Print string variable"
        });
        
        // Generic variable print (fallback) - IMPROVED VERSION
        patterns.push_back({
            [](const RTLInsn& insn) {
                return insn.op == RTLInsn::CALL && 
                       insn.attributes.count("builtin") && 
                       insn.attributes.at("builtin") == "print" &&
                       insn.attributes.count("type") && 
                       insn.attributes.at("type") == "variable";
            },
            [](const RTLInsn& insn, CodeGenContext& ctx) {
                auto var_name = insn.operands[0];
                
                // Check if we know the variable type
                if (ctx.variable_types.count(var_name)) {
                    auto var_type = ctx.variable_types[var_name];
                    if (var_type == "string") {
                        auto len_label = var_name + "_len";
                        return "    mov rax, 1\n    mov rdi, 1\n    mov rsi, " + var_name + 
                               "\n    mov edx, [" + len_label + "]\n    syscall\n" +  // Use edx
                               "    ; Print newline\n" +
                               "    mov rax, 1\n    mov rdi, 1\n    mov rsi, newline\n    mov rdx, 1\n    syscall\n";
                    } else if (var_type == "int") {
                        return "    ; Print integer variable " + var_name + " (simplified)\n" +
                               "    mov eax, [" + var_name + "]\n" +
                               "    ; Integer printing not fully implemented\n";
                    }
                }
                
                // Fallback: treat as string with fixed length
                return "    ; Print variable " + var_name + " (unknown type, treating as string)\n" +
                       "    mov rax, 1\n    mov rdi, 1\n    mov rsi, " + var_name + 
                       "\n    mov rdx, 64\n    syscall\n"; // Assume max 64 chars
            }, 2, "Print variable (generic fallback)"
        });
        
        // Inline assembly
        patterns.push_back({
            [](const RTLInsn& insn) { return insn.op == RTLInsn::INLINE_ASM; },
            [](const RTLInsn& insn, CodeGenContext& ctx) {
                return "    " + insn.operands[0] + "\n";
            }, 1, "Inline assembly"
        });
        
        // Function definition
        patterns.push_back({
            [](const RTLInsn& insn) {
                return insn.op == RTLInsn::LABEL && 
                       insn.attributes.count("type") && 
                       insn.attributes.at("type") == "function";
            },
            [](const RTLInsn& insn, CodeGenContext& ctx) {
                ctx.in_function = true;
                ctx.current_function = insn.operands[0];
                return "\n" + insn.operands[0] + ":\n" +
                       "    push rbp\n" +
                       "    mov rbp, rsp\n";
            }, 1, "Function definition"
        });
        
        // Function call
        patterns.push_back({
            [](const RTLInsn& insn) {
                return insn.op == RTLInsn::CALL && 
                       insn.attributes.count("type") && 
                       insn.attributes.at("type") == "function";
            },
            [](const RTLInsn& insn, CodeGenContext& ctx) {
                return "    call " + insn.operands[0] + "\n";
            }, 1, "Function call"
        });
        
        // Function return
        patterns.push_back({
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
        });
        
        // Integer variable assignment
        patterns.push_back({
            [](const RTLInsn& insn) {
                return insn.op == RTLInsn::DATA && 
                       insn.attributes.count("type") && 
                       insn.attributes.at("type") == "assignment" &&
                       insn.attributes.count("value_type") &&
                       insn.attributes.at("value_type") == "int";
            },
            [](const RTLInsn& insn, CodeGenContext& ctx) {
                auto var_name = insn.operands[0];
                auto value = insn.operands[1];
                
                ctx.variable_types[var_name] = "int";
                ctx.emit_data(var_name, "dd " + value);
                
                return "; Integer variable " + var_name + " = " + value + "\n";
            }, 1, "Integer variable assignment"
        });
        
        // String variable assignment - FIXED VERSION
        patterns.push_back({
            [](const RTLInsn& insn) {
                return insn.op == RTLInsn::DATA && 
                       insn.attributes.count("type") && 
                       insn.attributes.at("type") == "assignment" &&
                       insn.attributes.count("value_type") &&
                       insn.attributes.at("value_type") == "string";
            },
            [](const RTLInsn& insn, CodeGenContext& ctx) {
                auto var_name = insn.operands[0];
                auto value = insn.operands[1];
                auto len_label = var_name + "_len";
                
                ctx.variable_types[var_name] = "string";
                // Store string without null terminator since we know the length
                ctx.emit_data(var_name, "db \"" + value + "\"");
                ctx.emit_data(len_label, "dd " + std::to_string(value.length()));
                
                return "; String variable " + var_name + " = \"" + value + "\"\n";
            }, 1, "String variable assignment"
        });
        
        // Generic variable assignment (fallback)
        patterns.push_back({
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
                
                if (value_type == "float" || value_type == "double") {
                    ctx.variable_types[var_name] = "float";
                    ctx.emit_data(var_name, "dq " + value);
                } else {
                    // Default to quad word
                    ctx.variable_types[var_name] = "unknown";
                    ctx.emit_data(var_name, "dq " + value);
                }
                
                return "; Variable " + var_name + " = " + value + " (type: " + value_type + ")\n";
            }, 2, "Generic variable assignment"
        });
        
        // Variable declaration (no assignment)
        patterns.push_back({
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
        });
    }
    
    const Pattern* select_pattern(const RTLInsn& insn) {
        const Pattern* best_pattern = nullptr;
        int best_cost = INT_MAX;
        
        for (const auto& pattern : patterns) {
            if (pattern.predicate(insn) && pattern.cost < best_cost) {
                best_pattern = &pattern;
                best_cost = pattern.cost;
            }
        }
        
        return best_pattern;
    }
    
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
                    if constexpr (std::is_same_v<ValueType, int>) {
                        add_print_insn(rtl_insns, value, "int");
                    } else {
                        add_print_insn(rtl_insns, value, "string");
                    }
                }, stmt.value);
            }
            else if constexpr (std::is_same_v<T, PrintVariable>) {
                RTLInsn insn(RTLInsn::CALL, {stmt.variableName});
                insn.attributes["builtin"] = "print";
                insn.attributes["type"] = "variable";
                
                // Try to determine variable type from context
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
        }, node);
    }
    
public:
    PatternCodeGen() { define_patterns(); }
    
    std::vector<RTLInsn> lower_to_rtl(const AST& ast) {
        std::vector<RTLInsn> rtl_insns;
        rtl_insns.reserve(ast.size() * 2); // Rough estimate
        
        CodeGenContext temp_ctx; // Temporary context for type tracking during lowering
        
        for (const auto& node : ast) {
            process_ast_node(node, rtl_insns, temp_ctx);
        }
        
        return rtl_insns;
    }
    
    std::string generate_assembly(const std::vector<RTLInsn>& rtl_insns) {
        CodeGenContext ctx;
        std::ostringstream main_code, function_code;
        bool in_function = false;
        
        // Add common data
        ctx.emit_data("newline", "db 0xA");
        
        for (const auto& insn : rtl_insns) {
            const Pattern* pattern = select_pattern(insn);
            if (!pattern) {
                std::cerr << "Warning: No pattern found for instruction type " << insn.op << std::endl;
                continue;
            }
            
            std::string code = pattern->generator(insn, ctx);
            
            // Function organization logic
            if (insn.op == RTLInsn::LABEL && insn.attributes.count("type") && 
                insn.attributes.at("type") == "function") {
                in_function = true;
                function_code << code;
            }
            else if (insn.op == RTLInsn::RETURN && insn.attributes.count("type") && 
                     insn.attributes.at("type") == "function") {
                function_code << code;
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
        
        // BSS section (uninitialized data)
        if (!ctx.bss_section.str().empty()) {
            output << "\nsection .bss\n";
            output << ctx.bss_section.str();
        }
        
        // Text section
        output << "\nsection .text\n";
        output << "global _start\n\n";
        
        // Main program entry point
        output << "_start:\n";
        output << main_code.str();
        
        // Function definitions
        if (!function_code.str().empty()) {
            output << "\n; Function definitions\n";
            output << function_code.str();
        }
        
        return output.str();
    }
    
    // Debug helper
    void print_rtl_debug(const std::vector<RTLInsn>& rtl_insns) {
        std::cout << "=== RTL Instructions Debug ===" << std::endl;
        for (size_t i = 0; i < rtl_insns.size(); ++i) {
            const auto& insn = rtl_insns[i];
            std::cout << i << ": ";
            
            switch (insn.op) {
                case RTLInsn::SET: std::cout << "SET"; break;
                case RTLInsn::CALL: std::cout << "CALL"; break;
                case RTLInsn::RETURN: std::cout << "RETURN"; break;
                case RTLInsn::LABEL: std::cout << "LABEL"; break;
                case RTLInsn::DATA: std::cout << "DATA"; break;
                case RTLInsn::INLINE_ASM: std::cout << "INLINE_ASM"; break;
                case RTLInsn::LOAD: std::cout << "LOAD"; break;
                case RTLInsn::STORE: std::cout << "STORE"; break;
            }
            
            for (const auto& op : insn.operands) {
                std::cout << " " << op;
            }
            
            if (!insn.attributes.empty()) {
                std::cout << " [";
                bool first = true;
                for (const auto& attr : insn.attributes) {
                    if (!first) std::cout << ", ";
                    std::cout << attr.first << "=" << attr.second;
                    first = false;
                }
                std::cout << "]";
            }
            
            std::cout << std::endl;
        }
        std::cout << "===============================" << std::endl;
    }
};

// Public interface
std::string generate_asm(const AST& ast) {
    if (ast.empty()) {
        return "section .text\nglobal _start\n_start:\n    mov rax, 60\n    mov rdi, 0\n    syscall\n";
    }
    
    PatternCodeGen codegen;
    auto rtl_insns = codegen.lower_to_rtl(ast);
    
    // Uncomment for debugging:
    // codegen.print_rtl_debug(rtl_insns);
    
    return codegen.generate_assembly(rtl_insns);
}

// Additional utility functions for advanced code generation
namespace CodeGenUtils {
    
    // Helper to escape strings for assembly
    std::string escape_string_for_asm(const std::string& str) {
        std::string result;
        result.reserve(str.length() * 2); // Rough estimate
        
        for (char c : str) {
            switch (c) {
                case '\n': result += "\\n"; break;
                case '\t': result += "\\t"; break;
                case '\r': result += "\\r"; break;
                case '\\': result += "\\\\"; break;
                case '"': result += "\\\""; break;
                case '\0': result += "\\0"; break;
                default:
                    if (c >= 32 && c <= 126) {
                        result += c;
                    } else {
                        // Convert to hex escape
                        result += "\\x";
                        result += "0123456789ABCDEF"[(c >> 4) & 0xF];
                        result += "0123456789ABCDEF"[c & 0xF];
                    }
                    break;
            }
        }
        
        return result;
    }
    
    // Helper to generate unique labels
    std::string generate_unique_label(const std::string& prefix) {
        static int counter = 0;
        return prefix + "_" + std::to_string(counter++);
    }
    
    // Helper to calculate string length including escape sequences
    size_t calculate_string_length(const std::string& str) {
        size_t length = 0;
        for (size_t i = 0; i < str.length(); ++i) {
            if (str[i] == '\\' && i + 1 < str.length()) {
                switch (str[i + 1]) {
                    case 'n':
                    case 't':
                    case 'r':
                    case '\\':
                    case '"':
                    case '0':
                        ++i; // Skip the next character
                        break;
                    case 'x':
                        if (i + 3 < str.length()) {
                            i += 3; // Skip \x and two hex digits
                        }
                        break;
                }
            }
            ++length;
        }
        return length;
    }
}

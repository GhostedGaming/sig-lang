#include <codegen/public/codegen.hpp>
#include <sstream>
#include <vector>
#include <functional>
#include <memory>
#include <algorithm>

// RTL-like intermediate representation (like GCC's RTL)
struct RTLInsn {
    enum OpType { 
        SET,        // Assignment/move
        CALL,       // Function call  
        RETURN,     // Return from function
        LABEL,      // Code label
        DATA        // Data declaration
    };
    
    OpType op;
    std::vector<std::string> operands;
    std::unordered_map<std::string, std::string> attributes;
    
    RTLInsn(OpType operation) : op(operation) {}
    RTLInsn(OpType operation, std::vector<std::string> ops) 
        : op(operation), operands(std::move(ops)) {}
};

// Machine description pattern (like GCC's .md files)
struct InsnPattern {
    std::string name;
    std::function<bool(const RTLInsn&)> predicate;
    std::function<std::string(const RTLInsn&, class CodeGenContext&)> template_fn;
    int cost;
    
    InsnPattern(std::string n, 
                std::function<bool(const RTLInsn&)> pred,
                std::function<std::string(const RTLInsn&, class CodeGenContext&)> tmpl,
                int c = 1) 
        : name(std::move(n)), predicate(pred), template_fn(tmpl), cost(c) {}
};

// Code generation context (tracks state during generation)
class CodeGenContext {
public:
    std::ostringstream data_section;
    std::ostringstream text_section;
    int next_label_id = 0;
    
    std::string new_label(const std::string& prefix = "L") {
        return prefix + std::to_string(next_label_id++);
    }
    
    void emit_data(const std::string& label, const std::string& data) {
        data_section << label << ": " << data << "\n";
    }
    
    void emit_text(const std::string& code) {
        text_section << code;
    }
};

// The main code generator (like GCC's backend)
class PatternCodeGen {
private:
    std::vector<InsnPattern> patterns;
    
    void define_patterns() {
        // Pattern for return statements -> Linux exit syscall
        patterns.emplace_back(
            "linux_exit",
            [](const RTLInsn& insn) { 
                return insn.op == RTLInsn::RETURN; 
            },
            [](const RTLInsn& insn, CodeGenContext& ctx) {
                std::ostringstream code;
                code << " mov rax, 60\n";
                code << " mov rdi, " << insn.operands[0] << "\n";
                code << " syscall\n";
                return code.str();
            }
        );
        
        // Pattern for print statements -> Linux write syscall  
        patterns.emplace_back(
            "linux_write",
            [](const RTLInsn& insn) {
                return insn.op == RTLInsn::CALL && 
                       insn.attributes.count("builtin") &&
                       insn.attributes.at("builtin") == "print";
            },
            [](const RTLInsn& insn, CodeGenContext& ctx) {
                std::string msg_label = "msg" + std::to_string(ctx.next_label_id);
                std::string len_label = "len" + msg_label;
                ctx.next_label_id++;
                
                // Emit data
                ctx.emit_data(msg_label, "db \"" + insn.operands[0] + "\", 0xA");
                ctx.emit_data(len_label, "equ $ - " + msg_label);
                
                // Generate code
                std::ostringstream code;
                code << " mov rax, 1\n";
                code << " mov rdi, 1\n"; 
                code << " mov rsi, " << msg_label << "\n";
                code << " mov rdx, " << len_label << "\n";
                code << " syscall\n";
                return code.str();
            }
        );
        
        // Could add more patterns for different targets:
        // - Windows syscalls
        // - Function calls via ABI
        // - Different instruction sets (ARM, etc.)
    }
    
    // Pattern selection (like GCC's instruction selection pass)
    const InsnPattern* select_pattern(const RTLInsn& insn) {
        std::vector<const InsnPattern*> matches;
        
        // Find all matching patterns
        for (const auto& pattern : patterns) {
            if (pattern.predicate(insn)) {
                matches.push_back(&pattern);
            }
        }
        
        if (matches.empty()) {
            return nullptr;
        }
        
        // Select best pattern (lowest cost, like GCC's cost model)
        return *std::min_element(matches.begin(), matches.end(),
            [](const InsnPattern* a, const InsnPattern* b) {
                return a->cost < b->cost;
            });
    }
    
public:
    PatternCodeGen() {
        define_patterns();
    }
    
    // Convert AST to RTL (like GCC's tree->RTL lowering)
    std::vector<RTLInsn> lower_to_rtl(const AST& ast) {
        std::vector<RTLInsn> rtl_insns;
        
        for (const auto& node : ast) {
            std::visit([&](auto&& stmt) {
                using T = std::decay_t<decltype(stmt)>;
                
                if constexpr (std::is_same_v<T, ReturnStatement>) {
                    RTLInsn insn(RTLInsn::RETURN);
                    insn.operands.push_back(std::to_string(stmt.value));
                    rtl_insns.push_back(std::move(insn));
                    
                } else if constexpr (std::is_same_v<T, PrintStatement>) {
                    RTLInsn insn(RTLInsn::CALL);
                    insn.operands.push_back(std::to_string(stmt.value));
                    insn.attributes["builtin"] = "print";
                    rtl_insns.push_back(std::move(insn));
                }
            }, node);
        }
        
        return rtl_insns;
    }
    
    // Generate assembly from RTL (like GCC's final pass)
    std::string generate_assembly(const std::vector<RTLInsn>& rtl_insns) {
        CodeGenContext ctx;
        
        // Process each RTL instruction
        for (const auto& insn : rtl_insns) {
            const InsnPattern* pattern = select_pattern(insn);
            
            if (!pattern) {
                throw std::runtime_error("No pattern found for instruction");
            }
            
            // Apply the pattern template
            std::string code = pattern->template_fn(insn, ctx);
            ctx.emit_text(code);
        }
        
        // Assemble final output
        std::ostringstream output;
        output << "section .data\n" << ctx.data_section.str();
        output << "\nsection .text\nglobal _start\n_start:\n" << ctx.text_section.str();
        
        return output.str();
    }
};

// Your original interface, now using GCC's approach internally
std::string generate_asm(const AST& ast) {
    PatternCodeGen codegen;
    
    // Step 1: Lower AST to RTL (like GCC's gimple->RTL)
    auto rtl_insns = codegen.lower_to_rtl(ast);
    
    // Step 2: Pattern matching and code generation (like GCC's final pass)  
    return codegen.generate_assembly(rtl_insns);
}
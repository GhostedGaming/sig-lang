#include <codegen/public/codegen.hpp>
#include <sstream>
#include <vector>
#include <functional>
#include <memory>
#include <algorithm>

/**
 * RTL (Register Transfer Language) Intermediate Representation
 * 
 * This structure represents low-level instructions similar to GCC's RTL.
 * It serves as an intermediate form between high-level AST nodes and 
 * target machine code, making it easier to apply optimizations and
 * generate efficient assembly code.
 */
struct RTLInsn {
    // Enumeration of supported RTL instruction types
    enum OpType { 
        SET,        // Assignment/move operations (e.g., x = y)
        CALL,       // Function calls and builtin operations
        RETURN,     // Return statements with optional values
        LABEL,      // Code labels for jumps and references
        DATA,       // Data declarations and constants
        INLINE_ASM  // Inline assembly statements
    };
    
    OpType op;                                              // The instruction operation type
    std::vector<std::string> operands;                      // Instruction operands (registers, constants, etc.)
    std::unordered_map<std::string, std::string> attributes; // Additional metadata (type info, flags, etc.)
    
    // Constructors for creating RTL instructions
    RTLInsn(OpType operation) : op(operation) {}
    RTLInsn(OpType operation, std::vector<std::string> ops) 
        : op(operation), operands(std::move(ops)) {}
};

/**
 * Machine Description Pattern
 * 
 * Similar to GCC's machine description (.md) files, this structure
 * defines how RTL instructions map to target assembly code.
 * Each pattern includes:
 * - A predicate to match specific RTL instructions
 * - A template function to generate assembly code
 * - A cost for instruction selection optimization
 */
struct InsnPattern {
    std::string name;                                                           // Pattern identifier for debugging
    std::function<bool(const RTLInsn&)> predicate;                            // Function to match RTL instructions
    std::function<std::string(const RTLInsn&, class CodeGenContext&)> template_fn; // Code generation template
    int cost;                                                                   // Relative cost for pattern selection
    
    InsnPattern(std::string n, 
                std::function<bool(const RTLInsn&)> pred,
                std::function<std::string(const RTLInsn&, class CodeGenContext&)> tmpl,
                int c = 1) 
        : name(std::move(n)), predicate(pred), template_fn(tmpl), cost(c) {}
};

/**
 * Code Generation Context
 * 
 * Maintains state during code generation including:
 * - Separate streams for data and text sections
 * - Label generation for unique identifiers
 * - Helper functions for emitting assembly code
 */
class CodeGenContext {
public:
    std::ostringstream data_section;    // Accumulates .data section content
    std::ostringstream text_section;    // Accumulates .text section content
    int next_label_id = 0;             // Counter for generating unique labels
    
    /**
     * Generate a unique label with optional prefix
     * Used for string literals, jump targets, etc.
     */
    std::string new_label(const std::string& prefix = "L") {
        return prefix + std::to_string(next_label_id++);
    }
    
    /**
     * Emit data declaration to the .data section
     * @param label - Label name for the data
     * @param data - Assembly data directive (e.g., "db 'hello'")
     */
    void emit_data(const std::string& label, const std::string& data) {
        data_section << label << ": " << data << "\n";
    }
    
    /**
     * Emit code to the .text section
     * @param code - Assembly instructions to add
     */
    void emit_text(const std::string& code) {
        text_section << code;
    }
};

/**
 * Pattern-Based Code Generator
 * 
 * The main backend engine that:
 * 1. Converts high-level AST to RTL intermediate representation
 * 2. Applies pattern matching to select optimal instruction sequences
 * 3. Generates target assembly code using Linux system calls
 * 
 * This approach is similar to GCC's backend architecture.
 */
class PatternCodeGen {
private:
    std::vector<InsnPattern> patterns;  // Collection of instruction patterns
    
    /**
     * Define instruction patterns for the target architecture.
     * Each pattern specifies how to convert RTL instructions to x86-64 assembly
     * using Linux system calls for I/O operations.
     */
    void define_patterns() {
        // Pattern for return statements -> Linux exit syscall
        // Converts: return <value> -> exit(<value>) syscall
        patterns.emplace_back(
            "linux_exit",
            [](const RTLInsn& insn) { 
                return insn.op == RTLInsn::RETURN; 
            },
            [](const RTLInsn& insn, CodeGenContext& ctx) {
                std::ostringstream code;
                code << " mov rax, 60\n";                     // sys_exit syscall number
                code << " mov rdi, " << insn.operands[0] << "\n"; // exit status
                code << " syscall\n";                         // invoke syscall
                return code.str();
            }
        );

        // Pattern for printing string literals
        // Converts: print("string") -> write(1, string, length) syscall
        patterns.emplace_back(
            "linux_write_string",
            [](const RTLInsn& insn) {
                // Match CALL instructions with print builtin and string type
                return insn.op == RTLInsn::CALL && 
                       insn.attributes.count("builtin") &&
                       insn.attributes.at("builtin") == "print" &&
                       insn.attributes.count("type") &&
                       insn.attributes.at("type") == "string";
            },
            [](const RTLInsn& insn, CodeGenContext& ctx) {
                // Generate unique labels for the string data
                std::string msg_label = "msg" + std::to_string(ctx.next_label_id);
                std::string len_label = "len" + msg_label;
                ctx.next_label_id++;

                // Emit string data with newline (0xA = '\n')
                ctx.emit_data(msg_label, "db \"" + insn.operands[0] + "\", 0xA");
                ctx.emit_data(len_label, "equ $ - " + msg_label); // Calculate length

                // Generate sys_write syscall code
                std::ostringstream code;
                code << " mov rax, 1\n";                    // sys_write syscall number
                code << " mov rdi, 1\n";                    // file descriptor (stdout)
                code << " mov rsi, " << msg_label << "\n";  // buffer pointer
                code << " mov rdx, " << len_label << "\n";  // buffer length
                code << " syscall\n";                       // invoke syscall
                return code.str();
            }
        );

        // Pattern for printing integer values
        // Converts: print(42) -> write(1, "42\n", 3) syscall
        // Note: This is a simplified version that assumes the integer is already a string
        patterns.emplace_back(
            "linux_write_int",
            [](const RTLInsn& insn) {
                // Match CALL instructions with print builtin and int type
                return insn.op == RTLInsn::CALL && 
                       insn.attributes.count("builtin") &&
                       insn.attributes.at("builtin") == "print" &&
                       insn.attributes.count("type") &&
                       insn.attributes.at("type") == "int";
            },
            [](const RTLInsn& insn, CodeGenContext& ctx) {
                // Generate unique labels for the integer string
                std::string msg_label = "msg" + std::to_string(ctx.next_label_id);
                std::string len_label = "len" + msg_label;
                ctx.next_label_id++;

                // Convert integer to string representation and emit data
                std::string int_as_string = insn.operands[0];
                ctx.emit_data(msg_label, "db \"" + int_as_string + "\", 0xA");
                ctx.emit_data(len_label, "equ $ - " + msg_label);

                // Generate sys_write syscall code (same as string version)
                std::ostringstream code;
                code << " mov rax, 1\n";                    // sys_write syscall number
                code << " mov rdi, 1\n";                    // file descriptor (stdout)
                code << " mov rsi, " << msg_label << "\n";  // buffer pointer
                code << " mov rdx, " << len_label << "\n";  // buffer length
                code << " syscall\n";                       // invoke syscall
                return code.str();
            }
        );

        // Pattern for inline assembly statements
        // Converts: asm("mov rax, 1") -> direct assembly code injection
        patterns.emplace_back(
            "inline_assembly",
            [](const RTLInsn& insn) {
                // Match INLINE_ASM instructions
                return insn.op == RTLInsn::INLINE_ASM;
            },
            [](const RTLInsn& insn, CodeGenContext& ctx) {
                // Directly emit the assembly code with proper formatting
                std::ostringstream code;
                code << " " << insn.operands[0] << "\n";    // Add leading space for formatting
                return code.str();
            }
        );
    }
    
    /**
     * Pattern Selection Algorithm
     * 
     * Similar to GCC's instruction selection pass, this function:
     * 1. Finds all patterns that match the given RTL instruction
     * 2. Selects the pattern with the lowest cost
     * 3. Returns null if no pattern matches
     * 
     * This enables optimization by choosing the most efficient instruction sequence.
     */
    const InsnPattern* select_pattern(const RTLInsn& insn) {
        std::vector<const InsnPattern*> matches;
        
        // Find all matching patterns by testing predicates
        for (const auto& pattern : patterns) {
            if (pattern.predicate(insn)) {
                matches.push_back(&pattern);
            }
        }
        
        // Return null if no patterns match
        if (matches.empty()) {
            return nullptr;
        }
        
        // Select the pattern with lowest cost (best optimization)
        return *std::min_element(matches.begin(), matches.end(),
            [](const InsnPattern* a, const InsnPattern* b) {
                return a->cost < b->cost;
            });
    }
    
public:
    /**
     * Constructor - Initialize the code generator by defining instruction patterns
     */
    PatternCodeGen() {
        define_patterns();
    }
    
    /**
     * AST to RTL Lowering Pass
     * 
     * Converts high-level AST nodes to low-level RTL instructions.
     * This is similar to GCC's tree-to-RTL lowering phase.
     * 
     * @param ast - Abstract syntax tree to convert
     * @return Vector of RTL instructions
     */
    std::vector<RTLInsn> lower_to_rtl(const AST& ast) {
        std::vector<RTLInsn> rtl_insns;

        // Process each AST node
        for (const auto& node : ast) {
            // Use std::visit to handle the variant types in the AST
            std::visit([&](auto&& stmt) {
                using T = std::decay_t<decltype(stmt)>;

                // Handle return statements
                if constexpr (std::is_same_v<T, ReturnStatement>) {
                    RTLInsn insn(RTLInsn::RETURN);
                    insn.operands.push_back(std::to_string(stmt.value));
                    rtl_insns.push_back(std::move(insn));

                } 
                // Handle print statements
                else if constexpr (std::is_same_v<T, PrintStatement>) {
                    RTLInsn insn(RTLInsn::CALL);

                    // Handle the variant value (int or string) and preserve type information
                    std::visit([&](auto&& value) {
                        using ValueType = std::decay_t<decltype(value)>;
                        if constexpr (std::is_same_v<ValueType, int>) {
                            insn.operands.push_back(std::to_string(value));
                            insn.attributes["type"] = "int";           // Tag as integer
                        } else if constexpr (std::is_same_v<ValueType, std::string>) {
                            insn.operands.push_back(value);
                            insn.attributes["type"] = "string";        // Tag as string
                        }
                    }, stmt.value);

                    insn.attributes["builtin"] = "print";              // Mark as print builtin
                    rtl_insns.push_back(std::move(insn));
                }
                // Handle inline assembly statements
                else if constexpr (std::is_same_v<T, AsmStatement>) {
                    RTLInsn insn(RTLInsn::INLINE_ASM);
                    insn.operands.push_back(stmt.value);        // Store the assembly string
                    rtl_insns.push_back(std::move(insn));
                }
            }, node);
        }

        return rtl_insns;
    }

    /**
     * Assembly Code Generation
     * 
     * Takes RTL instructions and generates final x86-64 assembly code:
     * 1. Selects appropriate patterns for each RTL instruction
     * 2. Applies pattern templates to generate assembly code
     * 3. Assembles final output with proper sections
     * 
     * @param rtl_insns - Vector of RTL instructions to process
     * @return Complete assembly code as string
     */
    std::string generate_assembly(const std::vector<RTLInsn>& rtl_insns) {
        CodeGenContext ctx;
        
        // Process each RTL instruction
        for (const auto& insn : rtl_insns) {
            // Find the best pattern for this instruction
            const InsnPattern* pattern = select_pattern(insn);
            
            if (!pattern) {
                throw std::runtime_error("No pattern found for instruction");
            }
            
            // Apply the pattern template to generate assembly code
            std::string code = pattern->template_fn(insn, ctx);
            ctx.emit_text(code);
        }
        
        // Assemble final output with proper ELF sections
        std::ostringstream output;
        output << "section .data\n" << ctx.data_section.str();          // Data section (strings, constants)
        output << "\nsection .text\nglobal _start\n_start:\n" << ctx.text_section.str(); // Code section
        
        return output.str();
    }
};

/**
 * Main Code Generation Entry Point
 * 
 * Public interface function that orchestrates the entire code generation process:
 * 1. Creates a pattern-based code generator
 * 2. Lowers AST to RTL intermediate representation
 * 3. Generates final x86-64 assembly code
 * 
 * @param ast - Abstract syntax tree to compile
 * @return Complete x86-64 assembly code for Linux
 */
std::string generate_asm(const AST& ast) {
    PatternCodeGen codegen;
    
    // Convert AST to RTL instructions
    auto rtl_insns = codegen.lower_to_rtl(ast);
    
    // Generate final assembly code
    return codegen.generate_assembly(rtl_insns);
}
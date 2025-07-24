#pragma once
#include "rtl.hpp"
#include "context.hpp"
#include <functional>
#include <vector>
#include <string>

// Generic instruction template system
class InstructionTemplate {
public:
    std::string pattern;
    std::function<std::string(const RTLInsn&, CodeGenContext&)> generator;
    std::function<bool(const RTLInsn&)> matcher;
    int cost;
    std::string description;
    
    InstructionTemplate(const std::string& pat, 
                       std::function<bool(const RTLInsn&)> match,
                       std::function<std::string(const RTLInsn&, CodeGenContext&)> gen,
                       int c, const std::string& desc)
        : pattern(pat), matcher(match), generator(gen), cost(c), description(desc) {}
};

// Template manager
class TemplateManager {
private:
    std::vector<InstructionTemplate> templates;
    
    void init_instruction_templates();
    
public:
    TemplateManager();
    
    const InstructionTemplate* select_template(const RTLInsn& insn);
    void add_template(const InstructionTemplate& tmpl);
    
private:
    // Template initialization helpers
    void init_syscall_templates();
    void init_print_templates();
    void init_variable_templates();
    void init_function_templates();
    void init_misc_templates();
};

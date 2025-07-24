#pragma once
#include "register_alloc.hpp"
#include <sstream>
#include <unordered_map>
#include <string>

// Enhanced code generation context
class CodeGenContext {
public:
    std::ostringstream data_section, text_section, bss_section;
    std::unordered_map<std::string, std::string> variable_types;
    std::unordered_map<std::string, std::string> string_labels;
    std::unordered_map<std::string, Register> variable_registers;
    RegisterAllocator reg_alloc;
    int next_label_id = 0;
    int next_string_id = 0;
    bool in_function = false;
    std::string current_function;
    
    // Optimization settings
    bool optimize_for_size = false;
    bool optimize_for_speed = true;
    int optimization_level = 2; // 0=none, 1=basic, 2=full
    
    std::string new_label(const std::string& prefix = "L");
    std::string new_string_label();
    void emit_data(const std::string& label, const std::string& data);
    void emit_bss(const std::string& label, const std::string& data);
    std::string get_optimal_mov(const std::string& dest, const std::string& src);
};

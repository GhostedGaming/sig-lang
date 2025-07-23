#include "context.hpp"

std::string CodeGenContext::new_label(const std::string& prefix) {
    return prefix + std::to_string(next_label_id++);
}

std::string CodeGenContext::new_string_label() {
    return "str" + std::to_string(next_string_id++);
}

void CodeGenContext::emit_data(const std::string& label, const std::string& data) {
    data_section << label << ": " << data << "\n";
}

void CodeGenContext::emit_bss(const std::string& label, const std::string& data) {
    bss_section << label << ": " << data << "\n";
}

std::string CodeGenContext::get_optimal_mov(const std::string& dest, const std::string& src) {
    // Optimize common move patterns
    if (src == "0") {
        return "    xor " + dest + ", " + dest + "\n";
    }
    return "    mov " + dest + ", " + src + "\n";
}

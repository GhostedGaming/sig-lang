#pragma once
#include <string>

namespace CodeGenUtils {
    
    // Helper to escape strings for assembly
    std::string escape_string_for_asm(const std::string& str);
    
    // Helper to generate unique labels
    std::string generate_unique_label(const std::string& prefix);
    
    // Helper to calculate string length including escape sequences
    size_t calculate_string_length(const std::string& str);
}

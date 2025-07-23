#include "utils.hpp"

namespace CodeGenUtils {
    
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
    
    std::string generate_unique_label(const std::string& prefix) {
        static int counter = 0;
        return prefix + "_" + std::to_string(counter++);
    }
    
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

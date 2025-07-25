#include "lexer_helpers.hpp"
#include <iostream>
#include <cstdlib>

// Character classification lookup tables
constexpr std::array<bool, 256> make_identifier_start_table() {
    std::array<bool, 256> table{};
    
    // Set a-z as valid identifier start chars
    for (int i = 'a'; i <= 'z'; ++i) {
        table[i] = true;
    }
    
    // Underscore is also valid
    table['_'] = true;
    
    return table;
}

constexpr std::array<bool, 256> make_identifier_char_table() {
    std::array<bool, 256> table{};
    
    // Set a-z, A-Z, 0-9, _ as valid identifier chars
    for (int i = 'a'; i <= 'z'; ++i) {
        table[i] = true;
    }
    for (int i = 'A'; i <= 'Z'; ++i) {
        table[i] = true;
    }
    for (int i = '0'; i <= '9'; ++i) {
        table[i] = true;
    }
    table['_'] = true;
    
    return table;
}

constexpr std::array<bool, 256> make_digit_table() {
    std::array<bool, 256> table{};
    
    // Set 0-9 as valid digits
    for (int i = '0'; i <= '9'; ++i) {
        table[i] = true;
    }
    
    return table;
}

constexpr std::array<bool, 256> make_space_table() {
    std::array<bool, 256> table{};
    
    table[' '] = true;
    table['\t'] = true;
    table['\n'] = true;
    table['\r'] = true;
    table['\v'] = true;
    table['\f'] = true;
    
    return table;
}

const std::array<bool, 256> identifier_start_table = make_identifier_start_table();
const std::array<bool, 256> identifier_char_table = make_identifier_char_table();
const std::array<bool, 256> digit_table = make_digit_table();
const std::array<bool, 256> space_table = make_space_table();

const std::unordered_map<std::string_view, TokenType> keywords = {
    {"return",   TokenType::KeywordReturn},
    {"print",    TokenType::KeywordPrint},
    {"asm",      TokenType::KeywordAsm},
    {"pub",      TokenType::KeywordPub},
    {"fn",       TokenType::Function},
    {"let",      TokenType::KeywordLet},
    {"if",       TokenType::KeywordIf},
    {"else",     TokenType::KeywordElse},
    {"elif",     TokenType::KeywordElif},
    {"while",    TokenType::KeywordWhile},
    {"for",      TokenType::KeywordFor},
};



std::string read_multiline_comment(const std::string& input, size_t& i) {
    const char* data = input.data();
    size_t size = input.size();
    std::string content;
    
    while (i < size) {
        // Check for end of multiline comment (*\)
        if (i + 1 < size && data[i] == '*' && data[i + 1] == '\\') {
            // Found end of comment (*\)
            break;
        }
        content += data[i];
        ++i;
    }
    
    return content;
}

[[noreturn]] void report_lexer_error(const std::string& message, size_t position, const std::string& input) {
    std::cerr << "\nLexer Error at position " << position << ":\n";
    std::cerr << "   " << message << "\n";
    
    // Show context around the error
    const size_t context_range = 20;
    size_t start = (position >= context_range) ? position - context_range : 0;
    size_t end = std::min(position + context_range, input.size());
    
    std::cerr << "\nContext:\n";
    std::cerr << "   ";
    
    for (size_t i = start; i < end; ++i) {
        if (i == position) {
            std::cerr << " <<HERE>> ";
        }
        
        char c = input[i];
        if (c == '\n') {
            std::cerr << "\\n";
        } else if (c == '\t') {
            std::cerr << "\\t";
        } else if (c < 32 || c > 126) {
            std::cerr << "\\x" << std::hex << static_cast<int>(c) << std::dec;
        } else {
            std::cerr << c;
        }
    }
    
    std::cerr << "\n" << std::endl;
    std::exit(1);
}

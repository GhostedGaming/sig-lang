#include <lexer/public/lexer.hpp>
#include <cctype>
#include <iostream>
#include <unordered_map>
#include <array>

// Use constexpr functions to initialize lookup tables at compile time
constexpr std::array<bool, 256> make_identifier_start_table() {
    std::array<bool, 256> table{};
    
    // Set a-z
    for (int i = 'a'; i <= 'z'; ++i) {
        table[i] = true;
    }
    
    // Set A-Z
    for (int i = 'A'; i <= 'Z'; ++i) {
        table[i] = true;
    }
    
    // Set underscore
    table['_'] = true;
    
    return table;
}

constexpr std::array<bool, 256> make_identifier_char_table() {
    std::array<bool, 256> table{};
    
    // Set a-z
    for (int i = 'a'; i <= 'z'; ++i) {
        table[i] = true;
    }
    
    // Set A-Z
    for (int i = 'A'; i <= 'Z'; ++i) {
        table[i] = true;
    }
    
    // Set 0-9
    for (int i = '0'; i <= '9'; ++i) {
        table[i] = true;
    }
    
    // Set underscore
    table['_'] = true;
    
    return table;
}

constexpr std::array<bool, 256> make_digit_table() {
    std::array<bool, 256> table{};
    
    // Set 0-9
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

// Compile-time generated lookup tables
static constexpr auto identifier_start_table = make_identifier_start_table();
static constexpr auto identifier_char_table = make_identifier_char_table();
static constexpr auto digit_table = make_digit_table();
static constexpr auto space_table = make_space_table();

// Keyword lookup table
static const std::unordered_map<std::string_view, TokenType> keywords = {
    {"return", TokenType::KeywordReturn}, // Return keyword
    {"print", TokenType::KeywordPrint}, // Print keyword
    {"asm", TokenType::KeywordAsm}, // Asm keyword
    {"//", TokenType::Comment}, // Comment keyword
    {"pub", TokenType::KeywordPub}, // Public keyword
    {"fn", TokenType::Function}, // Function keyword
    {"let", TokenType::KeywordLet}, // Let keyword
};

// Inline functions for fast character classification
inline bool is_identifier_start(unsigned char c) {
    return identifier_start_table[c];
}

inline bool is_identifier_char(unsigned char c) {
    return identifier_char_table[c];
}

inline bool is_digit(unsigned char c) {
    return digit_table[c];
}

inline bool is_space(unsigned char c) {
    return space_table[c];
}

// Optimized identifier reading
static std::string_view read_identifier(const std::string& input, size_t& i) {
    size_t start = i;
    const char* data = input.data();
    size_t size = input.size();
    
    while (i < size && is_identifier_char(static_cast<unsigned char>(data[i]))) {
        ++i;
    }
    
    return std::string_view(data + start, i - start);
}

static std::string_view read_number(const std::string& input, size_t& i) {
    size_t start = i;
    const char* data = input.data();
    size_t size = input.size();
    
    while (i < size && is_digit(static_cast<unsigned char>(data[i]))) {
        ++i;
    }
    
    return std::string_view(data + start, i - start);
}

[[noreturn]] static void report_error(const std::string& message) {
    std::cerr << "Tokenizer error: " << message << "\n";
    std::exit(1);
}

std::vector<Token> tokenize(const std::string& input) {
    std::vector<Token> tokens;
    tokens.reserve(input.size() / 4); // Rough estimate
    
    size_t i = 0;
    const size_t size = input.size();
    const char* data = input.data();
    
    while (i < size) {
        unsigned char c = static_cast<unsigned char>(data[i]);
        
        // Skip whitespace efficiently
        if (is_space(c)) {
            ++i;
            continue;
        }
        
        if (c == '"') {
            // Add opening quote token
            tokens.emplace_back(TokenType::Quote);
            ++i;
            
            // Read string content until closing quote
            size_t string_start = i;
            std::string string_content;
            
            while (i < size && data[i] != '"') {
                if (data[i] == '\\' && i + 1 < size) {
                    // Handle escape sequences
                    ++i; // Skip backslash
                    switch (data[i]) {
                        case 'n': string_content += '\n'; break;
                        case 't': string_content += '\t'; break;
                        case 'r': string_content += '\r'; break;
                        case '\\': string_content += '\\'; break;
                        case '"': string_content += '"'; break;
                        default: 
                            string_content += '\\';
                            string_content += data[i];
                            break;
                    }
                } else {
                    string_content += data[i];
                }
                ++i;
            }
            
            if (i >= size) {
                report_error("Unterminated string literal");
            }
            
            // Add string content token
            tokens.emplace_back(TokenType::String, string_content);
            
            // Add closing quote token  
            tokens.emplace_back(TokenType::Quote);
            ++i; // Skip closing quote
        }
        else if (is_identifier_start(c)) {
            std::string_view identifier = read_identifier(input, i);
            
            // Check if it's a keyword first
            auto it = keywords.find(identifier);
            if (it != keywords.end()) {
                // It's a keyword - emit the keyword token
                tokens.emplace_back(it->second);
            } else {
                // It's a regular identifier (function name, variable, etc.)
                tokens.emplace_back(TokenType::Identifier, std::string(identifier));
            }
        } 
        else if (is_digit(c)) {
            std::string_view number = read_number(input, i);
            // Convert to string only when storing
            tokens.emplace_back(TokenType::IntegerLiteral, std::string(number));
        } 
        else if (c == ';') {
            tokens.emplace_back(TokenType::Semicolon);
            ++i;
        } 
        else if (c == '(') {
            tokens.emplace_back(TokenType::LeftParen);
            ++i;
        } 
        else if (c == ')') {
            tokens.emplace_back(TokenType::RightParen);
            ++i;
        }
        else if (c == '{') {
            tokens.emplace_back(TokenType::LeftBrace);
            ++i;
        }
        else if (c == '}') {
            tokens.emplace_back(TokenType::RightBrace);
            ++i;
        } else if (c == '=') {
            tokens.emplace_back(TokenType::Equal);
            ++i;
        }
        else {
            report_error("Unexpected character: '" + std::string(1, c) + "'");
        }
    }
    
    return tokens;
}
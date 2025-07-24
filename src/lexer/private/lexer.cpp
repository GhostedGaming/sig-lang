#include <lexer/public/lexer.hpp>
#include <cctype>
#include <iostream>
#include <unordered_map>
#include <array>
#include <algorithm>

constexpr std::array<bool, 256> make_identifier_start_table() {
    std::array<bool, 256> table{};
    
    for (int i = 'a'; i <= 'z'; ++i) {
        table[i] = true;
    }
    
    for (int i = 'A'; i <= 'Z'; ++i) {
        table[i] = true;
    }
    
    table['_'] = true;
    
    return table;
}

constexpr std::array<bool, 256> make_identifier_char_table() {
    std::array<bool, 256> table{};
    
    for (int i = 'a'; i <= 'z'; ++i) {
        table[i] = true;
    }
    
    for (int i = 'A'; i <= 'Z'; ++i) {
        table[i] = true;
    }
    
    for (int i = '0'; i <= '9'; ++i) {
        table[i] = true;
    }
    
    // Set underscore as valid identifier character
    table['_'] = true;
    
    return table;
}

/**
 * Creates a compile-time lookup table for digit characters
 * Valid characters: 0-9
 */
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

static constexpr auto identifier_start_table = make_identifier_start_table();
static constexpr auto identifier_char_table = make_identifier_char_table();
static constexpr auto digit_table = make_digit_table();
static constexpr auto space_table = make_space_table();

static const std::unordered_map<std::string_view, TokenType> keywords = {
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
};

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

/**
 * Reads multiline comment content until the closing *\ sequence
 * Updates i to point to the character after the comment content
 * Returns the content of the comment as a string
 */
static std::string read_multiline_comment(const std::string& input, size_t& i) {
    size_t start = i;
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

/**
 * Reports a tokenizer error and terminates the program
 * This is a [[noreturn]] function - it never returns to the caller
 */
/**
 * Report lexical analysis errors with position and context
 */
[[noreturn]] static void report_lexer_error(const std::string& message, size_t position, const std::string& input) {
    std::cerr << "\nLexer Error at position " << position << ":\n";
    std::cerr << "   " << message << "\n";
    
    // Show context around the error position
    if (position < input.length()) {
        size_t start = (position >= 20) ? position - 20 : 0;
        size_t end = std::min(position + 20, input.length());
        
        std::cerr << "\nContext:\n   ";
        for (size_t i = start; i < end; ++i) {
            if (i == position) {
                std::cerr << " <<HERE>> ";
            }
            char c = input[i];
            if (c == '\n') {
                std::cerr << "\\n";
            } else if (c == '\t') {
                std::cerr << "\\t";
            } else if (c >= 32 && c <= 126) {
                std::cerr << c;
            } else {
                std::cerr << "\\x" << std::hex << (unsigned char)c << std::dec;
            }
        }
        std::cerr << "\n";
    }
    
    std::cerr << std::endl;
    std::exit(1);
}

[[noreturn]] static void report_error(const std::string& message) {
    std::cerr << "\nLexer Error: " << message << "\n" << std::endl;
    std::exit(1);
}

/**
 * Main tokenization function
 * Takes an input string and returns a vector of tokens
 * Handles all supported token types including logical operators
 */
std::vector<Token> tokenize(const std::string& input) {
    std::vector<Token> tokens;
    tokens.reserve(input.size() / 4); // Rough estimate to reduce reallocations
    
    size_t i = 0;
    const size_t size = input.size();
    const char* data = input.data();
    
    while (i < size) {
        unsigned char c = static_cast<unsigned char>(data[i]);
        
        // Skip whitespace efficiently using lookup table
        if (is_space(c)) {
            ++i;
            continue;
        }
        
        // Handle string literals
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
                            // Unknown escape sequence - keep both characters
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
                report_lexer_error("Unterminated string literal - missing closing quote '\"'", string_start - 1, input);
            }
            
            // Add string content token
            tokens.emplace_back(TokenType::String, string_content);
            
            // Add closing quote token  
            tokens.emplace_back(TokenType::Quote);
            ++i; // Skip closing quote
        }
        // Handle identifiers and keywords
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
        // Handle numeric literals
        else if (is_digit(c)) {
            std::string_view number = read_number(input, i);
            // Convert to string only when storing
            tokens.emplace_back(TokenType::IntegerLiteral, std::string(number));
        } 
        // Handle single-character tokens and multi-character operators
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
        } 
        // Handle equality and assignment operators
        else if (c == '=') {
            // Check for == (equality comparison)
            if (i + 1 < size && data[i + 1] == '=') {
                tokens.emplace_back(TokenType::EqualEqual);
                i += 2;
            } else {
                // Single = (assignment)
                tokens.emplace_back(TokenType::Equal);
                ++i;
            }
        }
        // Handle logical NOT and inequality operators
        else if (c == '!') {
            // Check for != (not equal)
            if (i + 1 < size && data[i + 1] == '=') {
                tokens.emplace_back(TokenType::NotEqual);
                i += 2;
            } else {
                // Single ! (logical NOT)
                tokens.emplace_back(TokenType::Not);
                ++i;
            }
        }
        // Handle less than and less than or equal operators
        else if (c == '<') {
            // Check for <= (less than or equal)
            if (i + 1 < size && data[i + 1] == '=') {
                tokens.emplace_back(TokenType::LessThanEqual);
                i += 2;
            } else {
                // Single < (less than)
                tokens.emplace_back(TokenType::LessThan);
                ++i;
            }
        }
        // Handle greater than and greater than or equal operators
        else if (c == '>') {
            // Check for >= (greater than or equal)
            if (i + 1 < size && data[i + 1] == '=') {
                tokens.emplace_back(TokenType::GreaterThanEqual);
                i += 2;
            } else {
                // Single > (greater than)
                tokens.emplace_back(TokenType::GreaterThan);
                ++i;
            }
        }
        // Handle logical AND operator
        else if (c == '&') {
            // Check for && (logical AND)
            if (i + 1 < size && data[i + 1] == '&') {
                tokens.emplace_back(TokenType::And);
                i += 2;
            } else {
                // Single & is not supported in this language
                report_lexer_error("Unexpected character '&' - did you mean '&&' for logical AND?", i, input);
            }
        }
        // Handle logical OR operator  
        else if (c == '|') {
            // Check for || (logical OR)
            if (i + 1 < size && data[i + 1] == '|') {
                tokens.emplace_back(TokenType::Or);
                i += 2;
            } else {
                // Single | is not supported in this language
                report_lexer_error("Unexpected character '|' - did you mean '||' for logical OR?", i, input);
            }
        }
        // Handle comments and division (if division were supported)
        else if (c == '/') {
            if (i + 1 < size && data[i + 1] == '/') {
                // Single-line comment
                tokens.emplace_back(TokenType::Comment);
                i += 2;

                // Skip to end of line
                while (i < size && data[i] != '\n') {
                    ++i;
                }
            } 
            else if (i + 1 < size && data[i + 1] == '*') {
                // Multiline comment start
                tokens.emplace_back(TokenType::MultilineComment);
                i += 2; // Skip /*
                
                // Read comment content
                std::string comment_content = read_multiline_comment(input, i);
                
                // Check if we found the end (*\)
                if (i + 1 < size && data[i] == '*' && data[i + 1] == '\\') {
                    // Add the comment content if you want to store it
                    // tokens.emplace_back(TokenType::CommentContent, comment_content);
                    
                    // Add end multiline comment token
                    tokens.emplace_back(TokenType::EndMultilineComment);
                    i += 2; // Skip *\
                } else {
                    report_lexer_error("Unterminated multiline comment - missing closing '*/'", i, input);
                }
            } 
            else {
                // Single / is not supported (no division operator in this language)
                report_lexer_error("Unexpected character '/' - did you mean '//' for a comment?", i, input);
            }
        }
        // Handle any other unexpected characters
        else {
            // Provide context-specific suggestions for common mistakes
            std::string suggestion = "";
            if (c >= 'A' && c <= 'Z') {
                suggestion = " - identifiers should start with lowercase letters";
            } else if (c == '@' || c == '#' || c == '$') {
                suggestion = " - special characters are not allowed in identifiers";
            } else if (c == '`') {
                suggestion = " - did you mean '\"' for a string?";
            }
            
            report_lexer_error("Unexpected character '" + std::string(1, c) + "'" + suggestion, i, input);
        }
    }

    // Ensure every token stream has a return statement
    // If no return keyword was found, add one automatically
    if (std::find_if(tokens.begin(), tokens.end(),
        [](const Token& token) { return token.type == TokenType::KeywordReturn; }) != tokens.end()) {
        // Return statement already exists - do nothing
    } else {
        // No return statement found - add one
        tokens.emplace_back(TokenType::KeywordReturn);
    }

    return tokens;
}
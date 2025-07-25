#include "lexer_core.hpp"
#include <algorithm>

LexerCore::LexerCore(const std::string& input) 
    : position(0), size(input.size()), data(input.data()), input_ref(&input) {
    tokens.reserve(input.size() / 4); // Rough estimate to reduce reallocations
}

std::vector<Token> LexerCore::tokenize() {
    while (position < size) {
        unsigned char c = static_cast<unsigned char>(data[position]);
        
        // Skip whitespace efficiently using lookup table
        if (is_space(c)) {
            ++position;
            continue;
        }
        
        // Handle string literals
        if (c == '"') {
            process_string();
            continue;
        }
        
        // Handle identifiers and keywords
        if (is_identifier_start(c)) {
            process_identifier();
            continue;
        }
        
        // Handle integer literals
        if (is_digit(c)) {
            process_integer();
            continue;
        }
        
        // Handle operators and punctuation
        process_single_char_operators();
    }
    
    // Add EOF token if not present
    if (tokens.empty() || tokens.back().type != TokenType::EndOfFile) {
        tokens.emplace_back(TokenType::EndOfFile);
    }
    
    return std::move(tokens);
}

void LexerCore::process_identifier() {
    size_t start = position;
    
    // Read identifier characters
    while (position < size && is_identifier_char(static_cast<unsigned char>(data[position]))) {
        ++position;
    }
    
    std::string identifier(data + start, position - start);
    
    // Check if it's a keyword
    auto keyword_it = keywords.find(identifier);
    if (keyword_it != keywords.end()) {
        tokens.emplace_back(keyword_it->second, identifier);
    } else {
        tokens.emplace_back(TokenType::Identifier, identifier);
    }
}

void LexerCore::process_integer() {
    size_t start = position;
    
    // Read digits
    while (position < size && is_digit(static_cast<unsigned char>(data[position]))) {
        ++position;
    }
    
    std::string number(data + start, position - start);
    tokens.emplace_back(TokenType::IntegerLiteral, number);
}

void LexerCore::process_string() {
    // Add opening quote token
    tokens.emplace_back(TokenType::Quote);
    ++position;
    
    // Read string content until closing quote
    std::string string_content;
    
    while (position < size && data[position] != '"') {
        if (data[position] == '\\' && position + 1 < size) {
            // Handle escape sequences
            ++position; // Skip backslash
            switch (data[position]) {
                case 'n': string_content += '\n'; break;
                case 't': string_content += '\t'; break;
                case 'r': string_content += '\r'; break;
                case '\\': string_content += '\\'; break;
                case '"': string_content += '"'; break;
                default: 
                    // Unknown escape sequence - keep both characters
                    string_content += '\\';
                    string_content += data[position];
                    break;
            }
        } else {
            string_content += data[position];
        }
        ++position;
    }
    
    if (position >= size) {
        report_lexer_error("Unterminated string literal - missing closing quote", position, *input_ref);
    }
    
    // Add string content token
    tokens.emplace_back(TokenType::String, string_content);
    
    // Add closing quote token
    tokens.emplace_back(TokenType::Quote);
    ++position; // Skip closing quote
}

void LexerCore::process_single_char_operators() {
    unsigned char c = static_cast<unsigned char>(data[position]);
    
    switch (c) {
        case '(':
            tokens.emplace_back(TokenType::LeftParen);
            ++position;
            break;
        case ')':
            tokens.emplace_back(TokenType::RightParen);
            ++position;
            break;
        case '{':
            tokens.emplace_back(TokenType::LeftBrace);
            ++position;
            break;
        case '}':
            tokens.emplace_back(TokenType::RightBrace);
            ++position;
            break;
        case ';':
            tokens.emplace_back(TokenType::Semicolon);
            ++position;
            break;
        case ',':
            tokens.emplace_back(TokenType::Comma);
            ++position;
            break;
        case '=':
            if (position + 1 < size && data[position + 1] == '=') {
                tokens.emplace_back(TokenType::EqualEqual);
                position += 2;
            } else {
                tokens.emplace_back(TokenType::Equal);
                ++position;
            }
            break;
        case '!':
            if (position + 1 < size && data[position + 1] == '=') {
                tokens.emplace_back(TokenType::NotEqual);
                position += 2;
            } else {
                tokens.emplace_back(TokenType::Not);
                ++position;
            }
            break;
        case '<':
            if (position + 1 < size && data[position + 1] == '=') {
                tokens.emplace_back(TokenType::LessThanEqual);
                position += 2;
            } else {
                tokens.emplace_back(TokenType::LessThan);
                ++position;
            }
            break;
        case '>':
            if (position + 1 < size && data[position + 1] == '=') {
                tokens.emplace_back(TokenType::GreaterThanEqual);
                position += 2;
            } else {
                tokens.emplace_back(TokenType::GreaterThan);
                ++position;
            }
            break;
        case '&':
            if (position + 1 < size && data[position + 1] == '&') {
                tokens.emplace_back(TokenType::And);
                position += 2;
            } else {
                report_lexer_error("Unexpected character '&' - did you mean '&&' for logical AND?", position, *input_ref);
            }
            break;
        case '|':
            if (position + 1 < size && data[position + 1] == '|') {
                tokens.emplace_back(TokenType::Or);
                position += 2;
            } else {
                report_lexer_error("Unexpected character '|' - did you mean '||' for logical OR?", position, *input_ref);
            }
            break;
        case '/':
            if (position + 1 < size && data[position + 1] == '/') {
                // Single-line comment
                tokens.emplace_back(TokenType::Comment);
                position += 2;
                
                // Skip to end of line
                while (position < size && data[position] != '\n') {
                    ++position;
                }
            } else if (position + 1 < size && data[position + 1] == '*') {
                // Multiline comment start
                tokens.emplace_back(TokenType::MultilineComment);
                position += 2; // Skip /*
                
                // Read comment content
                std::string comment_content = read_multiline_comment(*input_ref, position);
                
                // Check if we found the end (*\)
                if (position + 1 < size && data[position] == '*' && data[position + 1] == '\\') {                    
                    // Add end multiline comment token
                    tokens.emplace_back(TokenType::EndMultilineComment);
                    position += 2; // Skip *\
                } else {
                    report_lexer_error("Unterminated multiline comment - missing closing '*/'", position, *input_ref);
                }
            } else {
                report_lexer_error("Unexpected character '/' - did you mean '//' for a comment?", position, *input_ref);
            }
            break;
        default:
            // Provide context-specific suggestions for common mistakes
            std::string suggestion = "";
            if (c >= 'A' && c <= 'Z') {
                suggestion = " - identifiers should start with lowercase letters";
            } else if (c == '@' || c == '#' || c == '$') {
                suggestion = " - special characters are not allowed in identifiers";
            } else if (c == '`') {
                suggestion = " - did you mean '\"' for a string?";
            }
            
            report_lexer_error("Unexpected character '" + std::string(1, c) + "'" + suggestion, position, *input_ref);
    }
}

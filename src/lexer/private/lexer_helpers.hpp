#pragma once
#include "../public/token.hpp"
#include <array>
#include <unordered_map>
#include <string_view>

// Character classification tables for fast lookup
constexpr std::array<bool, 256> make_identifier_start_table();
constexpr std::array<bool, 256> make_identifier_char_table();
constexpr std::array<bool, 256> make_digit_table();
constexpr std::array<bool, 256> make_space_table();

// Static tables (will be initialized by implementation)
extern const std::array<bool, 256> identifier_start_table;
extern const std::array<bool, 256> identifier_char_table;
extern const std::array<bool, 256> digit_table;
extern const std::array<bool, 256> space_table;

// Fast character classification functions
inline bool is_identifier_start(unsigned char c) { return identifier_start_table[c]; }
inline bool is_identifier_char(unsigned char c) { return identifier_char_table[c]; }
inline bool is_digit(unsigned char c) { return digit_table[c]; }
inline bool is_space(unsigned char c) { return space_table[c]; }

// Keywords mapping
extern const std::unordered_map<std::string_view, TokenType> keywords;

// Helper functions
std::string read_multiline_comment(const std::string& input, size_t& i);
[[noreturn]] void report_lexer_error(const std::string& message, size_t position, const std::string& input);

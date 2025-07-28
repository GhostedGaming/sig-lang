#pragma once
#include <lexer/public/lexer.hpp>
#include <ast/public/ast_simple.hpp>

AST parse(const std::vector<Token>& tokens, const std::string& file_path = "");

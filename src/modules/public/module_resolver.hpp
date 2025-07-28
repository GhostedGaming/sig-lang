#pragma once
#include <ast/public/ast_simple.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>

class ModuleResolver {
private:
    std::unordered_map<std::string, AST> loaded_modules;
    std::unordered_set<std::string> loading_modules;

    void find_all_modules(const AST& ast, std::vector<std::string>& module_paths);
    void find_modules_in_node(const ASTNode& node, std::vector<std::string>& module_paths);
    AST load_and_parse_module(const std::string& module_path);
    AST merge_asts(const AST& main_ast, const std::unordered_map<std::string, AST>& modules);

public:
    AST resolve_modules(const AST& main_ast, const std::string& main_file_path);
};

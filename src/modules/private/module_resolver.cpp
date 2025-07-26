#include "../public/module_resolver.hpp"
#include <parser/public/parser.hpp>
#include <lexer/public/lexer.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

std::string read_module_file(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        std::cerr << "Could not open module file: " << path << "\n";
        std::exit(1);
    }
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

AST ModuleResolver::resolve_modules(const AST& main_ast, const std::string& main_file_path) {
    // Step 1: Find all module references
    std::vector<std::string> module_paths;
    find_all_modules(main_ast, module_paths);
    
    if (module_paths.empty()) {
        return main_ast; // No modules to resolve
    }
    
    std::cout << "Found " << module_paths.size() << " module(s) to resolve\n";
    
    // Step 2: Load and parse each module
    for (const std::string& module_path : module_paths) {
        if (loaded_modules.find(module_path) == loaded_modules.end()) {
            // Check for circular dependency
            if (loading_modules.find(module_path) != loading_modules.end()) {
                std::cerr << "Error: Circular dependency detected for module: " << module_path << "\n";
                std::exit(1);
            }
            
            std::cout << "Loading module: " << module_path << "\n";
            loading_modules.insert(module_path);
            
            AST module_ast = load_and_parse_module(module_path);
            
            // Recursively resolve modules in this module
            ModuleResolver recursive_resolver;
            recursive_resolver.loaded_modules = loaded_modules;
            recursive_resolver.loading_modules = loading_modules;
            module_ast = recursive_resolver.resolve_modules(module_ast, module_path);
            
            // Update our state from recursive resolution
            loaded_modules = recursive_resolver.loaded_modules;
            loading_modules = recursive_resolver.loading_modules;
            
            loaded_modules[module_path] = module_ast;
            loading_modules.erase(module_path);
        }
    }
    
    // Step 3: Merge all ASTs
    return merge_asts(main_ast, loaded_modules);
}

void ModuleResolver::find_all_modules(const AST& ast, std::vector<std::string>& module_paths) {
    for (const auto& node : ast) {
        find_modules_in_node(node, module_paths);
    }
}

void ModuleResolver::find_modules_in_node(const ASTNode& node, std::vector<std::string>& module_paths) {
    std::visit([&](const auto& stmt) {
        using T = std::decay_t<decltype(stmt)>;
        
        if constexpr (std::is_same_v<T, ModStatement>) {
            module_paths.push_back(stmt.filename);
        }
        else if constexpr (std::is_same_v<T, FunctionDefinition>) {
            find_all_modules(stmt.body, module_paths);
        }
        else if constexpr (std::is_same_v<T, IfStatement>) {
            find_all_modules(stmt.thenBlock, module_paths);
            for (const auto& elif : stmt.elifClauses) {
                find_all_modules(elif.block, module_paths);
            }
            if (stmt.elseBlock.has_value()) {
                find_all_modules(stmt.elseBlock.value(), module_paths);
            }
        }
        else if constexpr (std::is_same_v<T, WhileStatement>) {
            find_all_modules(stmt.body, module_paths);
        }
        else if constexpr (std::is_same_v<T, ForStatement>) {
            find_all_modules(stmt.body, module_paths);
        }
        // Other statement types don't contain nested statements
    }, node);
}

AST ModuleResolver::load_and_parse_module(const std::string& module_path) {
    std::string module_code = read_module_file(module_path);
    auto tokens = tokenize(module_code);
    return parse(tokens, module_path);
}

AST ModuleResolver::merge_asts(const AST& main_ast, const std::unordered_map<std::string, AST>& modules) {
    AST merged_ast = main_ast;
    
    // Remove ModStatement nodes from main AST and replace with actual module content
    AST final_ast;
    
    for (const auto& node : main_ast) {
        std::visit([&](const auto& stmt) {
            using T = std::decay_t<decltype(stmt)>;
            
            if constexpr (std::is_same_v<T, ModStatement>) {
                // Replace module statement with actual module content
                auto it = modules.find(stmt.filename);
                if (it != modules.end()) {
                    // Add all statements from the module (except ModStatements)
                    for (const auto& module_node : it->second) {
                        if (!std::holds_alternative<ModStatement>(module_node)) {
                            final_ast.push_back(module_node);
                        }
                    }
                }
            } else {
                // Keep the original statement
                final_ast.push_back(node);
            }
        }, node);
    }
    
    return final_ast;
}
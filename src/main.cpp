#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <lexer/public/lexer.hpp>
#include <parser/public/parser.hpp>
#include <codegen/public/codegen.hpp>

std::string read_file(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        std::cerr << "Could not open file: " << path << "\n";
        std::exit(1);
    }
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

void print_help(const char* program_name) {
    std::cout << "Sig Language Compiler v0.2.0-alpha\n";
    std::cout << "A modern systems programming language powered by LLVM\n\n";
    std::cout << "USAGE:\n";
    std::cout << "    " << program_name << " <file.sg> [MODE]\n\n";
    std::cout << "MODES:\n";
    std::cout << "    --jit      Execute with LLVM JIT (default)\n";
    std::cout << "    --ir       Display generated LLVM IR\n";
    std::cout << "    --legacy   Use legacy x86-64 assembly backend\n\n";
    std::cout << "OPTIONS:\n";
    std::cout << "    -h, --help    Show this help message\n";
    std::cout << "    -v, --version Show version information\n\n";
    std::cout << "EXAMPLES:\n";
    std::cout << "    " << program_name << " examples/hello_world.sg\n";
    std::cout << "    " << program_name << " examples/variables.sg --ir\n";
    std::cout << "    " << program_name << " my_program.sg --jit\n\n";
    std::cout << "For more information, visit: https://github.com/GhostedGaming/sig-language\n";
}

void print_version() {
    std::cout << "Sig Language Compiler v0.2.0-alpha\n";
    std::cout << "Built with LLVM backend for cross-platform compilation\n";
    std::cout << "Copyright (c) 2024 - Licensed under MIT\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_help(argv[0]);
        return 1;
    }
    
    // Handle help and version flags
    std::string first_arg = argv[1];
    if (first_arg == "-h" || first_arg == "--help") {
        print_help(argv[0]);
        return 0;
    }
    if (first_arg == "-v" || first_arg == "--version") {
        print_version();
        return 0;
    }
    
    if (argc > 4) {
        std::cerr << "Error: Too many arguments.\n";
        print_help(argv[0]);
        return 1;
    }
    
    std::string input_file = argv[1];
    std::string mode = (argc >= 3) ? argv[2] : "--jit";
    
    // Read and process the source file
    std::string code = read_file(input_file);
    
    // Tokenize the code
    auto tokens = tokenize(code);
    
    // Parse tokens into AST
    auto ast = parse(tokens);
    
    if (mode == "--legacy") {
        // Legacy assembly generation
        std::string output_file = "out.asm";
        std::string asm_code = generate_asm(ast);
        
        std::cout << asm_code << std::endl;
        
        std::ofstream file(output_file);
        if (!file) {
            std::cerr << "Could not create output file: " << output_file << "\n";
            return 1;
        }
        file << asm_code;
        file.close();
        
        // Legacy assembly and linking
        std::string obj_file = "out.o";
        std::string nasm_command = "nasm -felf64 " + output_file + " -o " + obj_file;
        int result = std::system(nasm_command.c_str());
        
        if (result == 0) {
            std::cout << "Assembly successful. Object file created.\n";
            std::string exe_file = "out";
            std::string linker_command = "ld -o " + exe_file + " " + obj_file;
            int link_result = std::system(linker_command.c_str());
            
            if (link_result == 0) {
                std::cout << "Linking successful. Executable created: " << exe_file << "\n";
            } else {
                std::cerr << "Linking failed.\n";
                return 1;
            }
            
            std::string remove_command = "rm " + obj_file;
            system(remove_command.c_str());
        } else {
            std::cerr << "Assembly failed with nasm.\n";
            return 1;
        }
    } else {
        // LLVM-based compilation
        LLVMCodeGen codegen;
        codegen.compile(ast);
        
        if (mode == "--ir") {
            // Dump LLVM IR
            std::cout << "Generated LLVM IR:\n";
            codegen.dump_ir();
        } else {
            // JIT execution (default)
            std::cout << "Executing with LLVM JIT:\n";
            codegen.execute();
        }
    }

    return 0;
}
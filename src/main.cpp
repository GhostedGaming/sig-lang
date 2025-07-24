#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include "args.hpp"
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

void compile_legacy(const AST& ast, const std::string& output_name) {
    std::string output_file = output_name + ".asm";
    std::string asm_code = generate_asm(ast);
    
    std::ofstream file(output_file);
    if (!file) {
        std::cerr << "Could not create output file: " << output_file << "\n";
        exit(1);
    }
    file << asm_code;
    file.close();
    
    std::string obj_file = output_name + ".o";
    std::string nasm_command = "nasm -felf64 " + output_file + " -o " + obj_file;
    int result = std::system(nasm_command.c_str());
    
    if (result == 0) {
        std::cout << "Assembly successful. Object file created.\n";
        std::string linker_command = "ld -o " + output_name + " " + obj_file;
        int link_result = std::system(linker_command.c_str());
        
        if (link_result == 0) {
            std::cout << "Linking successful. Executable created: " << output_name << "\n";
        } else {
            std::cerr << "Linking failed.\n";
            exit(1);
        }
        
        std::remove(obj_file.c_str());
        std::remove(output_file.c_str());
    } else {
        std::cerr << "Assembly failed with nasm.\n";
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    CompilerArgs args = parse_args(argc, argv);
    
    if (args.show_help) {
        print_help(argv[0]);
        return args.input_file.empty() ? 1 : 0;
    }
    
    if (args.show_version) {
        print_version();
        return 0;
    }
    
    std::string code = read_file(args.input_file);
    auto tokens = tokenize(code);
    auto ast = parse(tokens);
    
    if (args.mode == "legacy") {
        compile_legacy(ast, args.output_name);
    } else {
        LLVMCodeGen codegen;
        codegen.compile(ast);
        
        if (args.mode == "ir") {
            std::cout << "Generated LLVM IR:\n";
            codegen.dump_ir();
        } else if (args.mode == "jit") {
            std::cout << "Executing with LLVM JIT:\n";
            codegen.execute();
        } else {
            std::cout << "Compiling " << args.input_file << " to " << args.output_name << "...\n";
            codegen.create_executable(args.output_name);
        }
    }

    return 0;
}

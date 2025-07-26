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
    
    CodeGen codegen(args.target_32bit);
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

    return 0;
}

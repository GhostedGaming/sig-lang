#include "args.hpp"
#include <iostream>

void print_help(const char* program_name) {
    std::cout << "Sig Language Compiler v0.2.0-alpha\n";
    std::cout << "A modern systems programming language powered by LLVM\n\n";
    std::cout << "USAGE:\n";
    std::cout << "    " << program_name << " <file.sg> [OPTIONS]\n\n";
    std::cout << "COMPILATION MODES:\n";
    std::cout << "    (default)      Compile to executable\n";
    std::cout << "    --jit          Execute with LLVM JIT\n";
    std::cout << "    --ir           Display generated LLVM IR\n";

    std::cout << "OPTIONS:\n";
    std::cout << "    -o <name>      Output executable name (default: program name)\n";
    std::cout << "    -m32           Target 32-bit architecture\n";
    std::cout << "    -h, --help     Show this help message\n";
    std::cout << "    -v, --version  Show version information\n\n";
    std::cout << "EXAMPLES:\n";
    std::cout << "    " << program_name << " hello.sg                    # Creates 'hello' executable\n";
    std::cout << "    " << program_name << " hello.sg -o myprogram       # Creates 'myprogram' executable\n";
    std::cout << "    " << program_name << " program.sg --jit            # Execute with JIT\n";
    std::cout << "    " << program_name << " program.sg --ir             # Show LLVM IR\n\n";
    std::cout << "For more information, visit: https://github.com/GhostedGaming/sig-language\n";
}

void print_version() {
    std::cout << "Sig Language Compiler v0.2.0-alpha\n";
    std::cout << "Built with LLVM backend for cross-platform compilation\n";
    std::cout << "Copyright (c) 2024 - Licensed under MIT\n";
}

std::string get_default_output_name(const std::string& input_file) {
    size_t last_slash = input_file.find_last_of("/\\");
    size_t start = (last_slash == std::string::npos) ? 0 : last_slash + 1;
    
    size_t last_dot = input_file.find_last_of('.');
    size_t end = (last_dot == std::string::npos || last_dot <= start) ? input_file.length() : last_dot;
    
    return input_file.substr(start, end - start);
}

CompilerArgs parse_args(int argc, char* argv[]) {
    CompilerArgs args;
    
    if (argc < 2) {
        args.show_help = true;
        return args;
    }
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            args.show_help = true;
            return args;
        }
        else if (arg == "-v" || arg == "--version") {
            args.show_version = true;
            return args;
        }
        else if (arg == "-o") {
            if (i + 1 >= argc) {
                std::cerr << "Error: -o requires an output filename\n";
                args.show_help = true;
                return args;
            }
            args.output_name = argv[++i];
        }
        else if (arg == "--jit") {
            args.mode = "jit";
        }
        else if (arg == "--ir") {
            args.mode = "ir";
        }
        else if (arg == "-m32") {
            args.target_32bit = true;
        }
        else if (arg[0] == '-') {
            std::cerr << "Error: Unknown option " << arg << "\n";
            args.show_help = true;
            return args;
        }
        else if (args.input_file.empty()) {
            args.input_file = arg;
        }
        else {
            std::cerr << "Error: Multiple input files not supported\n";
            args.show_help = true;
            return args;
        }
    }
    
    if (args.input_file.empty()) {
        std::cerr << "Error: No input file specified\n";
        args.show_help = true;
        return args;
    }
    
    if (args.output_name.empty()) {
        args.output_name = get_default_output_name(args.input_file);
    }
    
    return args;
}

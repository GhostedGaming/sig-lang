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

int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 4) {
        std::cerr << "Usage: " << argv[0] << " <file.sig> [output.asm]\n";
        return 1;
    }
    
    std::string input_file = argv[1];
    std::string output_file = (argc == 3) ? argv[2] : "out.asm";
    
    // Read and process the source file
    std::string code = read_file(input_file);
    
    // Tokenize the code
    auto tokens = tokenize(code);
    
    // Parse tokens into AST
    auto ast = parse(tokens);
    
    // Generate assembly code
    std::string asm_code = generate_asm(ast);
    
    // Output to console
    std::cout << asm_code << std::endl;
    
    // Write to file
    std::ofstream file(output_file);
    if (!file) {
        std::cerr << "Could not create output file: " << output_file << "\n";
        return 1;
    }
    file << asm_code;
    file.close();
    
    // Assemble the output file
    std::string obj_file = output_file.substr(0, output_file.find_last_of('.')) + ".o";
    std::string nasm_command_64 = "nasm -felf64 " + output_file + " -o " + obj_file;
    std::string nasm_command_32 = "nasm -felf32 " + output_file + " -o " + obj_file;
    int result_64 = std::system(nasm_command_64.c_str());
    int result_32 = std::system(nasm_command_32.c_str());

    if (argv[3] == "-32") {
        if (result_32 == 0) {
            std::cout << "Assembly successful. Object file created.\n";
        } else {
            std::cerr << "Assembly failed with nasm.\n";
            return 1;
        }
    } else {
        if (result_64 == 0) {
            std::cout << "Assembly successful. Object file created.\n";
        } else {
            std::cerr << "Assembly failed with nasm.\n";
            return 1;
        }
    }
    
    // Link the object file
    std::string exe_file = output_file.substr(0, output_file.find_last_of('.'));
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

    return 0;
}
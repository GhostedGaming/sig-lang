#include <iostream>
#include <fstream>
#include <sstream>
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
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file.sig>\n";
        return 1;
    }

    std::string code = read_file(argv[1]);
    auto tokens = tokenize(code);
    auto ast = parse(tokens);
    std::string asm_code = generate_asm(ast);

    std::cout << asm_code;
    std::fstream file("out.asm", std::ios::out);
    file << asm_code;

    system("");

    return 0;
}
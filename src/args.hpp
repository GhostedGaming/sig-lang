#pragma once
#include <string>

struct CompilerArgs {
    std::string input_file;
    std::string output_name;
    std::string mode = "compile";
    bool show_help = false;
    bool show_version = false;
};

CompilerArgs parse_args(int argc, char* argv[]);
void print_help(const char* program_name);
void print_version();
std::string get_default_output_name(const std::string& input_file);

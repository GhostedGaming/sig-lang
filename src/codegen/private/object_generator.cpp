#include "../public/codegen.hpp"
#include <llvm/Support/FileSystem.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/TargetParser/Triple.h>
#include <iostream>
#include <cstdlib>

using namespace llvm;

void CodeGen::create_executable(const std::string& output_name) {
    if (!module) {
        std::cerr << "Error: No module compiled" << std::endl;
        return;
    }
    
    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmParsers();
    InitializeAllAsmPrinters();
    
    // Use the target triple already configured in the module
    auto TargetTriple = module->getTargetTriple();
    
    std::string Error;
    auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);
    
    if (!Target) {
        std::cerr << "Error: " << Error << std::endl;
        return;
    }
    
    auto CPU = "generic";
    auto Features = "";
    
    TargetOptions opt;
    auto RM = std::optional<Reloc::Model>();
    auto TargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);
    
    module->setDataLayout(TargetMachine->createDataLayout());
    
    std::string obj_filename = output_name + ".o";
    std::error_code EC;
    raw_fd_ostream dest(obj_filename, EC, sys::fs::OF_None);
    
    if (EC) {
        std::cerr << "Could not open file: " << EC.message() << std::endl;
        return;
    }
    
    legacy::PassManager pass;
    auto FileType = CodeGenFileType::ObjectFile;
    
    if (TargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
        std::cerr << "TargetMachine can't emit a file of this type" << std::endl;
        return;
    }
    
    pass.run(*module);
    dest.flush();
    
    std::cout << "Object file created: " << obj_filename << std::endl;
    
    std::string link_command = "gcc -no-pie";
    if (target_32bit) {
        link_command += " -m32";
    }
    link_command += " -o " + output_name + " " + obj_filename;
    int result = std::system(link_command.c_str());
    
    if (result == 0) {
        std::cout << "Executable created: " << output_name << std::endl;
        std::cout << "Object file retained: " << obj_filename << std::endl;
        // Keep object file for debugging or further linking
        // std::remove(obj_filename.c_str());
    } else {
        std::cerr << "Linking failed. Object file left at: " << obj_filename << std::endl;
    }
}

void CodeGen::create_object_file(const std::string& output_name) {
    if (!module) {
        std::cerr << "Error: No module compiled" << std::endl;
        return;
    }
    
    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmParsers();
    InitializeAllAsmPrinters();
    
    auto TargetTriple = module->getTargetTriple();
    
    std::string Error;
    auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);
    
    if (!Target) {
        std::cerr << "Error: " << Error << std::endl;
        return;
    }
    
    auto CPU = "generic";
    auto Features = "";
    
    TargetOptions opt;
    auto RM = std::optional<Reloc::Model>();
    auto TargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);
    
    module->setDataLayout(TargetMachine->createDataLayout());
    
    std::error_code EC;
    raw_fd_ostream dest(output_name, EC, sys::fs::OF_None);
    
    if (EC) {
        std::cerr << "Could not open file: " << EC.message() << std::endl;
        return;
    }
    
    legacy::PassManager pass;
    auto FileType = CodeGenFileType::ObjectFile;
    
    if (TargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
        std::cerr << "TargetMachine can't emit a file of this type" << std::endl;
        return;
    }
    
    pass.run(*module);
    dest.flush();
    
    std::cout << "Object file created: " << output_name << std::endl;
}

void CodeGen::compile_to_object(const std::string& filename) {
    std::error_code EC;
    raw_fd_ostream dest(filename + ".ll", EC, sys::fs::OF_None);
    
    if (EC) {
        std::cerr << "Could not open file: " << EC.message() << std::endl;
        return;
    }
    
    module->print(dest, nullptr);
}

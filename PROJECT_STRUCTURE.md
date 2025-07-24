# Sig Language Project Structure

This document describes the organization of the Sig language compiler codebase.

## Root Directory

```
sig-language/
├── README.md           # Main project documentation
├── CMakeLists.txt      # Build configuration
├── build.sh           # Build script
├── install.sh         # Installation script  
├── uninstall.sh       # Uninstallation script
├── .gitignore         # Git ignore rules
├── Sig-Logo.jpg       # Project logo
├── sig                # Compiled executable
├── src/               # Source code
├── examples/          # Example programs  
└── docs/              # Documentation
```

## Source Code (`src/`)

```
src/
├── main.cpp           # Compiler entry point
├── lexer/             # Tokenization
│   ├── public/
│   │   └── lexer.hpp
│   └── private/
│       └── lexer.cpp
├── parser/            # Syntax analysis
│   ├── public/
│   │   └── parser.hpp
│   └── private/
│       └── parser.cpp
├── ast/               # Abstract Syntax Tree
│   └── public/
│       └── ast.hpp
└── codegen/           # Code generation
    ├── public/
    │   ├── codegen.hpp      # Legacy interface
    │   └── llvm_codegen.hpp # LLVM backend
    ├── private/
    │   ├── codegen.cpp      # Legacy implementation
    │   ├── llvm_codegen.cpp # LLVM implementation
    │   └── legacy_stubs.cpp # Legacy compatibility
    └── legacy/             # Legacy x86-64 backend
        ├── context.hpp/cpp
        ├── optimization.hpp/cpp
        ├── register_alloc.hpp/cpp
        ├── rtl.hpp
        └── templates.hpp/cpp
```

## Examples (`examples/`)

Working example programs demonstrating language features:

- `hello_world.sg` - Basic hello world
- `variables.sg` - Variable declarations and types
- `functions.sg` - Function definitions and calls
- `comprehensive.sg` - Full feature showcase
- `README.md` - Examples documentation

## Documentation (`docs/`)

- `CONTRIBUTING.md` - Contributor guidelines

## Build System

- **CMakeLists.txt**: CMake build configuration with LLVM integration
- **build.sh**: Convenient build script with dependency checking and optional installation
- **install.sh**: System-wide installation to user's bin directory
- **uninstall.sh**: Clean removal of installed compiler
- **.gitignore**: Excludes build artifacts and test files

## Key Components

### Frontend
- **Lexer**: Tokenizes source code into meaningful symbols
- **Parser**: Builds AST from token stream
- **AST**: Type-safe representation of program structure

### Backend (LLVM)
- **LLVM CodeGen**: Modern LLVM-based code generation
- **JIT Execution**: LLVM ORC for immediate execution
- **IR Generation**: Produces optimized LLVM intermediate representation

### Backend (Legacy)
- **Legacy CodeGen**: Original x86-64 assembly backend (deprecated)
- **RTL**: Register Transfer Language intermediate representation
- **Templates**: Pattern-based assembly generation

## Development Workflow

1. **Edit source**: Modify components in `src/`
2. **Add examples**: Create test cases in `examples/`
3. **Build**: Run `./build.sh` or `cmake . && make`
4. **Install**: Run `./install.sh` or `./build.sh --install`
5. **Test**: Execute `sig examples/program.sg` from anywhere
6. **Debug**: Use `--ir` flag to inspect LLVM IR

## Architecture Notes

- **Modular design**: Clear separation between frontend and backend
- **LLVM integration**: Modern optimization and multi-target support
- **Legacy compatibility**: Gradual migration from custom backend
- **Type safety**: Strong typing throughout AST and code generation
- **Cross-platform**: LLVM enables targeting multiple architectures

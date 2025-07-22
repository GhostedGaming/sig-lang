<div align="center">
<img src="Sig-Logo.jpg" alt="Sig-Logo" width="200"/>

# Sig Programming Language

*A high-level compiled systems programming language built for ease-of-use*

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)]()
[![License](https://img.shields.io/badge/license-MIT-blue)]()
[![Version](https://img.shields.io/badge/version-0.1.0--alpha-orange)]()

</div>

## 🚀 About

Sig is a modern systems programming language that bridges the gap between high-level ease-of-use and low-level system control. Designed for developers who want the expressiveness of high-level languages with the power to build operating systems, kernels, and system software.

### Key Features
- 🔧 **Systems Programming**: Direct hardware access and inline assembly support
- 🎯 **High-Level Syntax**: Familiar syntax for developers coming from modern languages  
- ⚡ **Compiled Performance**: Generates efficient x86-64 assembly
- 🛠️ **OS Development Ready**: Built with kernel and OS development in mind
- 🔒 **Memory Safe**: (Planned) Safe memory management with zero-cost abstractions

## 📋 Current Status

**⚠️ Development Phase**: Sig is currently in early development. Basic features are being implemented.

### Implemented Features
- [x] Basic lexer and parser
- [x] Print statements (integers and strings)
- [x] Return statements  
- [x] Inline assembly support
- [x] x86-64 code generation
- [x] Linux system call integration

### Roadmap
- [x] Variables and basic types
- [ ] Control flow (if/else, loops)
- [x] Functions and procedures
- [ ] Structs and enums
- [ ] Memory management
- [ ] Module system
- [ ] Standard library

## 🔧 Installation

### Prerequisites
- CMake 3.15+
- GCC/Clang with C++17 support
- NASM (for assembly)
- Linux (currently supported platform)

### Build from Source
```bash
git clone https://github.com/yourusername/sig-language.git
cd sig-language
mkdir build && cd build
cmake ..
make
```

## 📖 Usage

### Basic Example
```sig
// Hello World
print("Hello, World!");
return 0;
```

### Inline Assembly
```sig
// Direct assembly integration
asm("mov rax, 42");
print("Assembly executed!");
return 0;
```

### Compile and Run
```bash
./sig program.sg output_name
./output_name
```

## 🏗️ Architecture

Sig uses a multi-stage compilation pipeline:
1. **Lexer**: Tokenizes source code
2. **Parser**: Builds Abstract Syntax Tree (AST)
3. **Code Generator**: Converts AST to RTL intermediate representation
4. **Assembly Generator**: Produces x86-64 assembly using pattern matching
5. **Assembler/Linker**: Creates executable binary

## 🤝 Contributing

We welcome contributions! Sig is in early development and there's lots to do.

### How to Contribute
1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Development Areas
- Language features implementation
- Standard library development
- Documentation and examples
- Testing and benchmarks
- Platform support (Windows, macOS, Linux)

## 📚 Documentation

- [Language Specification](docs/spec.md) *(Coming Soon)*
- [API Reference](docs/api.md) *(Coming Soon)*
- [Examples](examples/) *(Coming Soon)*

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Inspired by systems languages like C, C++ and Rust
- Built with modern compiler design principles
- Thanks to all contributors and early adopters

---

<div align="center">
<strong>⭐ Star this repo if you're interested in Sig's development! ⭐</strong>
</div>

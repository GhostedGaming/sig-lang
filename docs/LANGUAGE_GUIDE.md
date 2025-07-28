# Sig Language Guide

## Table of Contents
- [Introduction](#introduction)
- [Getting Started](#getting-started)
- [Language Syntax](#language-syntax)
  - [Variables](#variables)
  - [Data Types](#data-types)
  - [Functions](#functions)
  - [Control Flow](#control-flow)
  - [Operators](#operators)
  - [Structs](#structs)
  - [Modules](#modules)
- [Built-in Functions](#built-in-functions)
- [Memory Management](#memory-management)
- [Advanced Features](#advanced-features)
- [Standard Library](#standard-library)

## Introduction

Sig is a modern systems programming language that combines high-level expressiveness with low-level control. Built on LLVM infrastructure, Sig provides portable, optimized compilation while maintaining a clean, intuitive syntax.

### Design Philosophy
- **Safety with Performance**: Memory-safe constructs without sacrificing speed
- **Simplicity**: Clean syntax that's easy to read and write
- **Flexibility**: Support for both high-level programming and systems work
- **LLVM Integration**: World-class optimization and cross-platform support

## Getting Started

### Hello World
Your first Sig program is simple:

```sig
print("Hello, World!");
```

### Compilation and Execution
```bash
# Compile to executable
sig hello.sg

# Execute immediately with JIT
sig hello.sg --jit

# Custom output name
sig hello.sg -o myprogram

# View LLVM IR
sig hello.sg --ir

# Compile without standard library (for OS development)
sig kernel.sg --no-std -o kernel.bin
```

## Language Syntax

### Variables

Variables in Sig are declared using the `let` keyword:

```sig
// Basic variable declaration
let x = 42;
let message = "Hello, Sig!";

// Type annotations (optional in many cases)
let x: u32 = 42;
let screen_addr: u32 = 0xb8000;
```

### Data Types

Sig supports various data types optimized for systems programming:

#### Integer Types
```sig
let byte_val: u8 = 255;        // 8-bit unsigned
let word_val: u16 = 65535;     // 16-bit unsigned  
let dword_val: u32 = 42;       // 32-bit unsigned
let qword_val: u64 = 1000000;  // 64-bit unsigned

// Signed integers
let signed_byte: i8 = -128;
let signed_word: i16 = -32768;
let signed_dword: i32 = -2147483648;
let signed_qword: i64 = -9223372036854775808;
```

#### Floating Point Types
```sig
let pi: f32 = 3.14159;
let precision: f64 = 2.718281828459045;
```

#### Boolean Type
```sig
let is_true: bool = true;
let is_false: bool = false;
```

#### String Type
```sig
let greeting = "Hello, World!";
let multiline = "This is a
multiline string";
```

#### Hexadecimal Literals
```sig
let hex_value: u32 = 0x1234ABCD;
let memory_addr: u32 = 0xb8000;
```

#### Pointers
```sig
let screen: *u16 = screen_addr as *u16;
```

### Functions

Functions are defined using the `fn` keyword:

```sig
// Simple function
fn greet() {
    print("Hello from function!");
}

// Function with parameters
fn add(a: u32, b: u32) -> u32 {
    return a + b;
}

// Function calls
greet();
let result = add(10, 20);
```

### Control Flow

#### Conditional Statements
```sig
// If-else statements
if (condition) {
    print("True branch");
} elif (other_condition) {
    print("Elif branch");
} else {
    print("Else branch");
}

// Comparison operators
if (x == 42) {
    print("x is 42");
}

if (x != 0) {
    print("x is not zero");
}
```

#### Loops

**While Loops:**
```sig
let i = 0;
while (i < 10) {
    print(i);
    i = i + 1;
}
```

**For Loops:**
```sig
// For loop with range
for (i, 1, 100) {
    print(i);
}
```

### Operators

#### Arithmetic Operators
```sig
let a = 10;
let b = 3;

let sum = a + b;        // Addition
let diff = a - b;       // Subtraction
let product = a * b;    // Multiplication
let quotient = a / b;   // Division
let remainder = a % b;  // Modulo
```

#### Comparison Operators
```sig
let x = 10;
let y = 20;

x == y    // Equal to
x != y    // Not equal to
x < y     // Less than
x <= y    // Less than or equal to
x > y     // Greater than
x >= y    // Greater than or equal to
```

#### Logical Operators
```sig
let a = true;
let b = false;

a && b    // Logical AND
a || b    // Logical OR
!a        // Logical NOT
```

#### Bitwise Operators
```sig
let flags: u32 = 0x12345678;

flags | 0x01     // Bitwise OR
flags & 0xfe     // Bitwise AND
flags ^ 0x0f     // Bitwise XOR
flags << 2       // Left shift
flags >> 1       // Right shift
```

### Structs

Structs allow you to group related data:

```sig
// Struct definition
struct VGAEntry {
    char: u8,
    color: u8,
}

// Struct usage
let entry = VGAEntry {
    char: 65,     // ASCII 'A'
    color: 0x07,  // White on black
};
```

### Modules

Sig supports modular programming:

```sig
// Import module
import "module_name";

// Use module functions
module_name::function_name();
```

## Built-in Functions

### Print Functions
```sig
print("Hello");          // Print string
print(42);               // Print integer
println("Line ending");  // Print with newline
```

### Type Casting
```sig
let addr: u32 = 0xb8000;
let ptr: *u16 = addr as *u16;
```

## Memory Management

Sig provides manual memory management with safety features:

```sig
// Pointer operations
let ptr: *u32 = address as *u32;

// Dereferencing (planned feature)
let value = *ptr;
*ptr = 42;
```

## Advanced Features

### Inline Assembly (Planned)
```sig
unsafe {
    asm!("mov rax, 42");
}
```

### Unsafe Code Blocks (Planned)
```sig
unsafe {
    // Direct memory access
    let ptr: *u8 = 0x1000 as *u8;
    *ptr = 42;
}
```

## Standard Library

The Sig standard library provides essential functionality:

### I/O Operations
```sig
print("Hello");           // Console output
println("Hello");         // Console output with newline
```

### Math Operations
```sig
// Basic arithmetic supported
let result = (a + b) * c / d;
```

### String Operations
```sig
let message = "Hello, " + "World!";  // String concatenation (planned)
```

## Error Handling

Sig uses a simple approach to error handling:

```sig
// Compilation errors are caught at compile time
// Runtime errors cause program termination
```

## Best Practices

### Code Style
- Use snake_case for variables and functions
- Use PascalCase for structs and types
- Use descriptive names
- Comment complex logic

### Performance Tips
- Use appropriate integer sizes for your data
- Prefer stack allocation when possible
- Use const for immutable data (planned feature)

### Safety Guidelines
- Always validate pointer operations
- Check array bounds manually
- Use type annotations for clarity

## Example Programs

### Systems Programming Example
```sig
// VGA text mode programming
let screen_addr: u32 = 0xb8000;
let screen: *u16 = screen_addr as *u16;

struct VGAEntry {
    char: u8,
    color: u8,
}

fn write_char(c: u8, color: u8, x: u32, y: u32) {
    let offset = (y * 80 + x) * 2;
    let entry: u16 = (color as u16 << 8) | (c as u16);
    // *(screen + offset) = entry;  // When pointer ops are ready
}
```

### Algorithm Example
```sig
fn fibonacci(n: u32) -> u32 {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

for (i, 1, 10) {
    println(fibonacci(i));
}
```

### Data Structure Example
```sig
struct Point {
    x: f32,
    y: f32,
}

fn distance(p1: Point, p2: Point) -> f32 {
    let dx = p1.x - p2.x;
    let dy = p1.y - p2.y;
    return sqrt(dx * dx + dy * dy);  // When math lib is ready
}
```

## What's Next?

Sig is actively developed. Upcoming features include:
- Enhanced type system
- Memory safety features
- Expanded standard library
- Package manager
- IDE tooling support

For the latest updates and examples, check the [examples directory](../examples/) and the project [README](../README.md).

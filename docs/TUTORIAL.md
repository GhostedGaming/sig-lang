# Sig Programming Language Tutorial

Welcome to the Sig programming language tutorial! This guide will take you from your first "Hello, World!" program to writing more complex systems programming code.

## Table of Contents
1. [Getting Started](#getting-started)
2. [Basic Programs](#basic-programs)
3. [Variables and Types](#variables-and-types)
4. [Functions](#functions)
5. [Control Flow](#control-flow)
6. [Data Structures](#data-structures)
7. [Systems Programming](#systems-programming)
8. [Best Practices](#best-practices)
9. [Next Steps](#next-steps)

## Getting Started

### Prerequisites
Before starting this tutorial, make sure you have Sig installed. If not, follow the [installation guide](../README.md#installation).

### Your First Program
Let's start with the classic "Hello, World!" program. Create a file called `hello.sg`:

```sig
print("Hello, World!");
```

To run this program:
```bash
# Compile and run
sig hello.sg
./hello

# Or execute directly with JIT
sig hello.sg --jit
```

**Expected output:**
```
Hello, World!
```

Congratulations! You've written your first Sig program.

## Basic Programs

### Comments
Comments help document your code. In Sig, use `//` for single-line comments:

```sig
// This is a comment
print("Hello!"); // This is also a comment
```

### Multiple Statements
Programs can have multiple statements, each ending with a semicolon:

```sig
print("First line");
print("Second line");
print("Third line");
```

### Print vs Println
Sig provides two print functions:

```sig
print("Hello");
print(" ");
print("World");
// Output: Hello World

println("Hello");
println("World");
// Output:
// Hello
// World
```

## Variables and Types

### Basic Variables
Variables store data that can be used throughout your program:

```sig
let message = "Hello, Sig!";
let number = 42;

print(message);
print(number);
```

### Type Annotations
While Sig can often infer types, you can be explicit:

```sig
let age: u32 = 25;
let temperature: f32 = 98.6;
let is_student: bool = true;
let name: string = "Alice";

println(age);
println(temperature);
println(is_student);
println(name);
```

### Integer Types in Detail
Different integer types serve different purposes:

```sig
let byte_value: u8 = 255;        // 0 to 255
let small_num: u16 = 65535;      // 0 to 65,535
let regular_num: u32 = 1000000;  // 0 to ~4 billion
let big_num: u64 = 1000000000;   // Very large numbers

println(byte_value);
println(small_num);
println(regular_num);
println(big_num);
```

### Working with Hexadecimal
Hex literals are useful for systems programming:

```sig
let memory_addr: u32 = 0xDEADBEEF;
let rgb_color: u32 = 0xFF5733;
let flags: u8 = 0xAB;

println(memory_addr);
println(rgb_color);
println(flags);
```

**Exercise:** Create variables for your favorite color in RGB hex format and your age in years.

## Functions

### Simple Functions
Functions group related code together:

```sig
fn say_hello() {
    println("Hello from a function!");
}

fn say_goodbye() {
    println("Goodbye!");
}

// Call the functions
say_hello();
say_goodbye();
```

### Functions with Parameters
Functions can accept input parameters:

```sig
fn greet_person(name: string) {
    print("Hello, ");
    print(name);
    println("!");
}

fn add_numbers(a: u32, b: u32) {
    let result = a + b;
    println(result);
}

greet_person("Alice");
greet_person("Bob");
add_numbers(10, 20);
add_numbers(100, 200);
```

### Return Values (When Available)
Future versions will support return values:

```sig
// This will work in future versions:
fn add(a: u32, b: u32) -> u32 {
    return a + b;
}

let sum = add(5, 3);
println(sum);
```

**Exercise:** Write a function that takes two numbers and prints their sum, difference, and product.

## Control Flow

### Conditional Statements
Use `if` statements to make decisions:

```sig
let age: u32 = 18;

if (age >= 18) {
    println("You are an adult");
} else {
    println("You are a minor");
}
```

### Multiple Conditions
Use `elif` for multiple conditions:

```sig
let score: u32 = 85;

if (score >= 90) {
    println("Grade: A");
} elif (score >= 80) {
    println("Grade: B");
} elif (score >= 70) {
    println("Grade: C");
} else {
    println("Grade: F");
}
```

### Comparison Operators
Learn all the ways to compare values:

```sig
let x: u32 = 10;
let y: u32 = 20;

if (x == y) {
    println("Equal");
}

if (x != y) {
    println("Not equal");
}

if (x < y) {
    println("x is less than y");
}

if (x <= y) {
    println("x is less than or equal to y");
}

if (x > y) {
    println("x is greater than y");
} else {
    println("x is not greater than y");
}
```

### While Loops
Repeat code while a condition is true:

```sig
let count: u32 = 1;

while (count <= 5) {
    println(count);
    count = count + 1;
}

println("Done counting!");
```

### For Loops
Use for loops to iterate over ranges:

```sig
println("Counting from 1 to 10:");
for (i, 1, 10) {
    println(i);
}

println("Counting from 10 to 20:");
for (num, 10, 20) {
    println(num);
}
```

**Exercise:** Write a program that prints the multiplication table for a given number (e.g., 5 × 1 = 5, 5 × 2 = 10, etc.).

## Data Structures

### Structs
Structs group related data together:

```sig
struct Person {
    age: u32,
    height: u32,  // in centimeters
}

struct Point {
    x: f32,
    y: f32,
}

// Creating struct instances (syntax may vary)
let alice = Person {
    age: 25,
    height: 165,
};

let origin = Point {
    x: 0.0,
    y: 0.0,
};
```

### Working with Memory Addresses
Pointers are essential for systems programming:

```sig
let address: u32 = 0x1000;
let ptr: *u32 = address as *u32;

println(address);
// Dereferencing will be available in future versions
```

**Exercise:** Define a struct for a `Rectangle` with width and height fields.

## Systems Programming

### No-Std Mode for OS Development

When developing operating systems, kernels, or embedded systems, you need to work without the standard library. Sig provides the `--no-std` option for this:

```sig
// kernel.sg - Simple kernel example
fn kernel_main() {
    // Direct hardware access
    let vga_buffer: *u16 = 0xB8000 as *u16;
    let screen_width: u32 = 80;
    let screen_height: u32 = 25;
    
    // No printf or puts available!
    // You'll need to implement your own I/O
}
```

Compile without standard library:
```bash
sig kernel.sg --no-std -o kernel.bin
```

**What's not available in no-std mode:**
- `print()` and `println()` functions
- Standard library functions
- C runtime dependencies

**What you can still use:**
- Variables and basic types
- Functions and control flow
- Structs and data manipulation
- Direct memory access
- Arithmetic and bitwise operations

### Low-Level Data Types
Systems programming often requires specific data sizes:

```sig
// VGA text mode programming example
let vga_buffer: u32 = 0xB8000;
let screen_width: u16 = 80;
let screen_height: u16 = 25;

// Character attributes
let white_on_black: u8 = 0x07;
let red_on_black: u8 = 0x04;

println(vga_buffer);
println(screen_width);
println(screen_height);
```

### Bitwise Operations
Essential for hardware interaction:

```sig
let flags: u32 = 0x00000000;

// Set individual bits
flags = flags | 0x01;      // Set bit 0
flags = flags | 0x04;      // Set bit 2

// Clear bits
flags = flags & 0xFE;      // Clear bit 0

// Toggle bits
flags = flags ^ 0x08;      // Toggle bit 3

// Shift operations
let shifted_left = flags << 2;   // Shift left by 2
let shifted_right = flags >> 1;  // Shift right by 1

println(flags);
println(shifted_left);
println(shifted_right);
```

### Hardware Registers Example
```sig
// Simulating hardware register manipulation
let control_register: u32 = 0x12345678;

// Extract specific bits (when bitfield ops are available)
let status_bits = control_register & 0xFF;        // Lower 8 bits
let config_bits = (control_register >> 8) & 0xFF; // Next 8 bits

println(status_bits);
println(config_bits);
```

**Exercise:** Write a program that sets, clears, and toggles various bits in a 32-bit flag register.

## Best Practices

### Code Organization
Keep your code organized and readable:

```sig
// Good: Clear, descriptive names
let screen_width: u32 = 1920;
let screen_height: u32 = 1080;

fn calculate_screen_area() {
    let area = screen_width * screen_height;
    println(area);
}

// Good: Logical grouping
fn initialize_graphics() {
    println("Initializing graphics system...");
}

fn cleanup_graphics() {
    println("Cleaning up graphics system...");
}
```

### Commenting Strategy
Use comments to explain why, not what:

```sig
// Good: Explains the purpose
let vga_base: u32 = 0xB8000;  // VGA text buffer base address

// Good: Explains complex logic
if (flags & 0x80) {           // Check if high bit is set
    println("System ready");  // indicating system ready state
}
```

### Type Safety
Choose appropriate types for your data:

```sig
// Good: Use specific sizes when they matter
let pixel_count: u32 = 1920 * 1080;  // Specific size for large numbers
let color_component: u8 = 255;       // RGB values fit in 8 bits
let temperature: f32 = 98.6;         // Floating point for precision

// Good: Use descriptive variable names
let max_retry_count: u8 = 5;
let timeout_seconds: u32 = 30;
```

### Error Prevention
Write defensive code:

```sig
// Check bounds before operations
let array_size: u32 = 100;
let index: u32 = 50;

if (index < array_size) {
    println("Safe to access array element");
} else {
    println("Index out of bounds!");
}
```

## Next Steps

### Intermediate Projects
Try these projects to practice your skills:

1. **Calculator**: Build a simple calculator with basic operations
2. **Number Guessing Game**: Create a game where the user guesses a number
3. **Binary Converter**: Convert decimal numbers to binary representation
4. **Simple Text Formatter**: Process and format text strings

### Advanced Topics to Explore
- Module system and code organization
- Memory management patterns
- Hardware abstraction layers
- Performance optimization techniques

### Project Ideas
1. **Kernel Module**: Write a simple kernel module using Sig
2. **Embedded Controller**: Program a microcontroller
3. **System Utility**: Create a command-line system tool
4. **Game Engine**: Build a simple 2D game engine

### Learning Resources
- Study the [examples directory](../examples/) for more code samples
- Read the [Language Guide](LANGUAGE_GUIDE.md) for comprehensive details
- Check the [API Reference](API_REFERENCE.md) for function documentation
- Explore the source code to understand implementation details

### Community and Support
- Report issues and request features on the GitHub repository
- Contribute to the language development
- Share your projects and get feedback

## Final Exercise

Create a complete program that demonstrates multiple concepts:

```sig
// Comprehensive example program
struct GameState {
    player_x: u32,
    player_y: u32,
    score: u32,
}

fn init_game() {
    println("Initializing game...");
}

fn game_loop() {
    let lives: u8 = 3;
    let level: u32 = 1;
    
    for (turn, 1, 10) {
        println("Turn: ");
        println(turn);
        
        if (turn % 3 == 0) {
            println("Boss battle!");
        } else {
            println("Regular enemy");
        }
    }
    
    println("Game over!");
}

// Main game
init_game();
game_loop();
```

Congratulations! You've completed the Sig programming language tutorial. You now have the foundation to start writing your own Sig programs and exploring the exciting world of systems programming.

Remember: the best way to learn is by practicing. Start with small programs and gradually work your way up to more complex projects. Good luck!

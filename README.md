# Ada Subset Compiler

**CSc 446 – Compiler Construction Project**  
**Author:** Landon Dahmen

## Overview

This project is a custom compiler for a **subset of the Ada programming language**, built as a capstone for a university-level compiler construction course. The compiler implements all major stages of the compilation process, from lexical analysis to code generation, and translates Ada-like source code into 8086 assembly language.

## Features

- **Lexical Analysis:** Tokenizes source code and recognizes Ada subset syntax.
- **Recursive Descent Parsing:** Implements a hand-written parser for the Ada subset grammar.
- **Symbol Table Management:** Tracks variables, constants, procedures, and types.
- **Semantic Analysis:** Performs type checking and enforces semantic rules.
- **Three Address Code (TAC) Generation:** Produces intermediate TAC output from source code.
- **Assembly Code Generation:** Translates TAC to 8086 assembly code.
- **Error Handling:** Detects and reports both syntactic and semantic errors with precise line numbers.
- **Testing Framework:** Includes test Ada files and scripts for validation.

## Project Structure
ada-subset-compiler/
├── src/ # Main source code (C++)
│ ├── Lexer.cpp
│ ├── Parser.cpp
│ ├── TACGenerator.cpp
│ ├── CodeGen8086.cpp
│ └── ...
├── tests/ # Sample Ada files and expected outputs
│ ├── *.ada
│ ├── *.tac
│ └── *.asm
├── Makefile # Build instructions
├── README.md # Project documentation
└── ...

## Getting Started

### Prerequisites

- **C++ Compiler** (g++, clang++, or similar)
- (Optional) **NASM** or other assembler to assemble the generated `.asm` files

### Build Instructions

1. **Clone the repository:**
    ```bash
    git clone https://github.com/landonwdahmen/ada-subset-compiler.git
    cd ada-subset-compiler
    ```

2. **Build the compiler:**
    ```bash
    make
    ```

3. **Run the compiler:**
    ```bash
    ./ada_compiler input.ada
    ```

    Output files (TAC and ASM) will be generated in the output directory or as specified by command-line options.

### Testing

- Sample Ada source files can be found in the `tests/` folder.
- Compare the generated `.tac` and `.asm` outputs against the expected results for validation.

## Key Technologies

- **C++** for all compiler stages
- **x86 Assembly (8086)**
- **Makefile** for build automation

## Educational Objectives

- Demonstrate understanding of lexical analysis, parsing, semantic analysis, and code generation.
- Practice writing compilers for real-world languages and producing machine code.
- Explore optimizations and error detection techniques used in modern compilers.

## Screenshots

![image](https://github.com/user-attachments/assets/d9d38253-8e44-45e1-b940-ffae6337e70e)

## License

This project is for educational purposes.

---

**If you have questions, want to contribute, or find bugs, feel free to open an issue or submit a pull request!**

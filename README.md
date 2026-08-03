# COA Simulator

A C++ console application for Computer Organization and Architecture coursework. It reads a single infix arithmetic expression, validates it, converts it to postfix form, generates multiple intermediate instruction formats, and simulates their execution step by step.

## Project Overview

This project demonstrates how the same arithmetic expression can be represented and executed in different instruction models used in computer architecture.

Supported expression rules:
- Operands: lowercase letters `a` to `z`
- Operators: `+`, `-`, `*`, `/`
- Parentheses: `(` and `)`
- One expression per run

The program currently includes:
- Expression validation
- Infix to postfix conversion
- Three-address code generation
- Two-address instruction generation
- One-address accumulator instruction generation
- Zero-address stack instruction generation
- Simulation traces for each instruction format
- Clean table-based console output

## Features

- Validates expressions before processing
- Converts infix expressions to postfix using the Shunting Yard algorithm
- Generates intermediate code in four instruction formats
- Simulates execution for each format
- Shows intermediate results after every step
- Displays results in tables for readability
- Uses portable ANSI color accents only when supported by the terminal

## Algorithms Used

### 1. Expression Validation
A single-pass stack-based validation approach checks:
- Balanced parentheses
- Valid operators
- Allowed operands only
- Invalid characters
- Consecutive operators
- Empty parentheses

### 2. Infix to Postfix Conversion
The program uses the **Shunting Yard algorithm** to convert infix expressions into postfix notation.

### 3. Three-Address Code Generation
The postfix expression is scanned with a stack to produce temporary assignments such as:
- `t1=b*c`
- `t2=a+t1`

### 4. Two-Address Instruction Generation
The postfix expression is converted into instructions using a simple register model with `R1`.

### 5. One-Address Instruction Generation
The postfix expression is converted into accumulator-based instructions using `AC`.

### 6. Zero-Address Instruction Generation
The postfix expression is converted into stack-machine instructions using:
- `PUSH`
- `POP`
- `ADD`
- `SUB`
- `MUL`
- `DIV`

### 7. Simulation
Each instruction format has a corresponding simulator that prints the intermediate state after every instruction.

## Folder Structure

```text
COA SIMULATOR/
└── src/
    ├── main.cpp
    ├── ExpressionValidator.h
    ├── ExpressionValidator.cpp
    ├── ExpressionConverter.h
    ├── ExpressionConverter.cpp
    ├── InstructionGenerator.h
    ├── InstructionGenerator.cpp
    ├── Simulator.h
    └── Simulator.cpp
```

## Sample Input

```text
a+b*c
```

## Sample Output

```text
Expression Summary
+----------------------+------------------------+
| Original             | a+b*c                  |
| Postfix              | abc*+                  |
+----------------------+------------------------+

Three Address Code
+--------+--------------------+
| #      | Instruction        |
+--------+--------------------+
| 1      | t1=b*c             |
| 2      | t2=a+t1            |
+--------+--------------------+

Three Address Simulation
+--------------------+----------------+
| Step | Instruction   | Result       | State
...

Two Address Code
| 1 | MOV R1,b |
| 2 | MUL R1,c |
| 3 | MOV t1,R1 |
| 4 | MOV R1,a |
| 5 | ADD R1,t1 |
| 6 | MOV t2,R1 |

One Address Code
| 1 | LOAD b |
| 2 | MUL c |
| 3 | STORE t1 |
| 4 | LOAD a |
| 5 | ADD t1 |
| 6 | STORE t2 |

Zero Address Code
| 1 | PUSH a |
| 2 | PUSH b |
| 3 | PUSH c |
| 4 | MUL |
| 5 | ADD |
| 6 | POP t1 |
```

## How to Run

### Prerequisites
- A C++17 compatible compiler such as `g++`

### Compile

From the project root:

```bash
g++ -std=c++17 src/main.cpp src/ExpressionValidator.cpp src/ExpressionConverter.cpp src/InstructionGenerator.cpp src/Simulator.cpp -o coa-simulator
```

### Run

```bash
./coa-simulator
```

Then enter an expression such as:

```text
a+b*c
```

### Optional Output Behavior
- To disable ANSI color accents, set `NO_COLOR` in your environment.
- Colors are only enabled when the program is writing to a terminal.

## Future Improvements

- Accept custom input values for variables instead of using default values
- Add error handling for malformed intermediate instruction strings
- Support multi-character operands and numeric literals
- Add unit tests for validation, conversion, generation, and simulation
- Provide a build script or `CMakeLists.txt`
- Expand the simulator to support memory and more realistic machine state
- Add output export to text or JSON

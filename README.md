# COA Simulator

A simple web-based Computer Organization and Architecture project that lets users validate arithmetic expressions, convert them to postfix form, generate instruction sets for several instruction formats, and simulate them step by step in the browser.

## What the app does

- Accepts arithmetic expressions using lowercase operands `a` to `z`
- Supports `+`, `-`, `*`, `/` and parentheses
- Validates the expression before processing
- Converts infix to postfix
- Detects the used variables automatically
- Shows input boxes only for the required variables
- Generates and simulates:
  - Three-address instructions
  - Two-address instructions
  - One-address instructions
  - Zero-address instructions
- Displays a comparison table for each format

## How to run

### 1. Build once

From the project root:

```bash
g++ -std=c++17 -I src src/*.cpp -o coa-simulator
```

### 2. Start the web app

```bash
./coa-simulator
```

You can also choose a custom port when launching it:

```bash
./coa-simulator 8080
```

The app starts a local web server and serves the simulator at:

```text
http://127.0.0.1:8080
```

If you use a different port, open that URL instead.

## Project structure

```text
COA_SIMULATOR/
├── public/
│   ├── index.html
│   ├── styles.css
│   └── app.js
├── src/
│   ├── main.cpp
│   ├── ExpressionValidator.h
│   ├── ExpressionValidator.cpp
│   ├── ExpressionConverter.h
│   ├── ExpressionConverter.cpp
│   ├── InstructionGenerator.h
│   ├── InstructionGenerator.cpp
│   ├── Simulator.h
│   ├── Simulator.cpp
│   ├── WebAppService.h
│   └── WebAppService.cpp
└── README.md
```

## Demo workflow

1. Open the page in the browser.
2. Enter an expression such as `a+b*c`.
3. Click Validate.
4. Enter values for the detected variables.
5. Click Generate & Execute.
6. Review the generated instructions and simulation steps.

## Notes

- The project is intentionally simple and readable for a semester viva/demo.
- It uses a lightweight C++ HTTP server and a Bootstrap 5 frontend.
- The backend logic is still based on the original COA expression converter and instruction generator.

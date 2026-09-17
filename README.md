# TrafficRule: Domain-Specific Language and Compiler

Domain-Specific Language and Compiler for Traffic Rule Enforcement (Course: **BCSE307L Compiler Design Laboratory**).

## Quick Build & Run

### 1. Build
```cmd
build.bat
```
or manually with MinGW GCC:
```cmd
gcc -std=c99 -Wall -Wextra src/*.c -o trafficrule.exe
```

### 2. Run All Automated Tests
```powershell
powershell -ExecutionPolicy Bypass -File .\test_all.ps1
```

### 3. Run Pipeline on Any Source File
```cmd
trafficrule.exe examples/sample2.tr --all
```

## CLI Flags
- `--tokens` : Display lexical tokens table
- `--ast` : Print hierarchical Abstract Syntax Tree
- `--dot <file>` : Export AST to Graphviz DOT format
- `--symbols` : Display populated Symbol Table
- `--ir` : Display Three-Address Code (TAC) and Quadruples table
- `--opt-ir` : Display optimized TAC and Quadruples
- `--run` : Execute rule simulation in the Interpreter
- `--all` : Run all compiler stages sequentially

## Project Structure
- `src/` : Complete C compiler source code (`lexer.c`, `parser.c`, `ast.c`, `symbol_table.c`, `semantic.c`, `ir.c`, `optimizer.c`, `interpreter.c`, `main.c`)
- `examples/` : Valid programs and error test cases (`sample1.tr`, `sample2.tr`, `sample3_else.tr`, `sample_opt.tr`, `err_lexical.tr`, `err_syntax.tr`, `err_semantic.tr`)
- `test_all.ps1` : Automated PowerShell test regression runner
- `build.bat` : One-click batch build script
- `report_phase2.md` : Implementation Progress Report for Review 2
- `viva_preparation.md` : Complete Viva Q&A preparation guide

# TrafficRule: Domain-Specific Language (Lexer & Parser Prototype)

Prototype implementation for **Review 1** (Course: **BCSE307L Compiler Design Laboratory**).

## Quick Build & Run

### 1. Build
```cmd
build.bat
```
or manually:
```cmd
gcc -std=c99 -Wall -Wextra src/*.c -o trafficrule.exe
```

### 2. Run Automated Test Suite
```powershell
powershell -ExecutionPolicy Bypass -File .\test_all.ps1
```

### 3. Run Pipeline on Sample Files
```powershell
# Show both Lexer (Tokens) and Parser (AST)
.\trafficrule.exe examples/sample1.tr --all

# Show Token Table only (Lexer)
.\trafficrule.exe examples/sample1.tr --tokens

# Show Abstract Syntax Tree only (Parser)
.\trafficrule.exe examples/sample1.tr --ast
```

## Supported CLI Flags
- `--tokens` : Display lexical tokens table
- `--ast`    : Print hierarchical Abstract Syntax Tree
- `--dot <f>`: Export AST to Graphviz DOT format
- `--all`    : Run both Lexer and Parser (Default)

## Project Structure
- `src/`
  - `common.h` / `common.c` : Shared token types and string utilities
  - `lexer.h` / `lexer.c`   : Lexical analyzer (token recognition, line/col tracking, lexical error reporting)
  - `ast.h` / `ast.c`       : Abstract Syntax Tree node definitions, constructors, and ASCII tree visualizer
  - `parser.h` / `parser.c` : Top-down predictive recursive descent parser and syntax error reporting
  - `main.c`                : CLI driver orchestrating the Lexer and Parser
- `examples/`
  - `sample1.tr`    : Speed violation rule
  - `sample2.tr`    : Compound AND signal crossing rule
  - `err_lexical.tr`: Lexical error test case (unrecognized symbol)
  - `err_syntax.tr` : Syntax error test case (malformed expression)
- `test_all.ps1`    : Automated test suite
- `build.bat`       : 1-click build script
- `viva_preparation.md` : Viva questions and answers guide

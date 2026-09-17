# TrafficRule: Review 1 Viva Preparation & Prototype Walkthrough

This document prepares you for **Review 1** (Course: **BCSE307L Compiler Design Laboratory**).

---

## 1. Quick Demonstration Commands (For Faculty)

```powershell
# 1. Clean build (0 warnings)
.\build.bat

# 2. Run automated test suite (4/4 tests pass)
powershell -ExecutionPolicy Bypass -File .\test_all.ps1

# 3. Full prototype demonstration (Lexer -> Parser -> AST)
.\trafficrule.exe examples/sample1.tr --all

# 4. Lexer token stream table only
.\trafficrule.exe examples/sample1.tr --tokens

# 5. Parser AST Tree only
.\trafficrule.exe examples/sample1.tr --ast

# 6. Lexical error detection (illegal character '@')
.\trafficrule.exe examples/err_lexical.tr

# 7. Syntax error detection (incomplete comparison 'IF speed > {')
.\trafficrule.exe examples/err_syntax.tr
```

---

## 2. File-by-File Code Walkthrough

### `src/common.h` & `src/common.c`
* **Purpose:** The Shared Dictionary.
* Contains the `TokenType` enum (lists all keywords `VEHICLE`, `IF`, `ELSE`, `AND`, `OR`, traffic states `RED`, `GREEN`, and operators `==`, `>`, etc.).
* Contains the `Token` struct (`type`, `lexeme`, `int_val`, `line`, `col`).
* `common.c` contains string conversion helper functions.

---

### `src/lexer.h` & `src/lexer.c` (Phase 1: Lexical Analysis)
* **Purpose:** The Scanner. Reads characters from source code and outputs tokens.
* **`advance_char()`**: Reads the next character, tracks line and column numbers.
* **`skip_whitespace_and_comments()`**: Skips spaces, tabs, newlines, and `#` comments.
* **`lexer_next_token()`**: Main function:
  * Letters $\to$ collects word, checks against keywords/states/actions, otherwise marks as `TOKEN_IDENTIFIER`.
  * Digits $\to$ collects digits, converts using `atoi()`, returns `TOKEN_NUMBER`.
  * Symbols $\to$ identifies single and multi-character operators (`=`, `==`, `!=`, `>`, `<`, `>=`, `<=`, `{`, `}`).
  * Unknown characters $\to$ raises a `Lexical Error` with exact line, column, and ASCII code.

---

### `src/ast.h` & `src/ast.c` (Syntax Tree Blueprint)
* **Purpose:** Defines the tree data structure in memory.
* **`ASTNode`**: Represents a node with a tagged union holding child pointers for statements, conditions, expressions, and actions.
* **`ast_create_*()`**: Constructors that allocate heap memory for nodes.
* **`ast_print()`**: Recursively prints the visual tree structure with indentation.
* **`ast_free()`**: Recursively frees all tree memory to avoid memory leaks.

---

### `src/parser.h` & `src/parser.c` (Phase 2: Syntax Analysis)
* **Purpose:** Top-down Predictive Recursive Descent Parser. Checks grammar rules and builds the AST.
* **`parse_statement()`**: Checks the current token to call `parse_declaration()`, `parse_if_stmt()`, `parse_assignment()`, or `parse_action()`.
* **`parse_if_stmt()` & `parse_block()`**: Recursively parses `IF condition { statement* }`.
* **`parse_condition()` & `parse_and_expr()`**: Enforces operator precedence (`AND` binds tighter than `OR`).
* **`consume()`**: Verifies expected tokens; if missing, reports exact syntax error with line and column.

---

### `src/main.c` (Driver)
* Reads the command-line flags (`--tokens`, `--ast`, `--all`).
* Reads the input source file into a buffer.
* Drives Stage 1 (Lexer) and Stage 2 (Parser) sequentially.

---

## 3. High-Scoring Viva Answers for Review 1

### Q1: What is the project topic and problem statement?
> *"TrafficRule is a Domain-Specific Language (DSL) and compiler for writing and enforcing traffic rules such as speed limits and red-light crossings. The objective is to design a language grammar and build a custom compiler in C to tokenize, parse into an AST, and evaluate traffic safety rules without needing external camera or GPS hardware."*

### Q2: Why is the grammar LL(1)?
> *"It is LL(1) because the FIRST sets of all statement alternatives are mutually disjoint: declarations always begin with `VEHICLE`, IF statements with `IF`, assignments with a variable identifier, and actions with action keywords. The parser only needs 1 lookahead token to make deterministic decisions without backtracking."*

### Q3: Why did you use a Recursive Descent Parser?
> *"Because the grammar is LL(1), recursive descent allows a clean 1-to-1 mapping from grammar rules to C functions, natural AST construction, and precise syntax error reporting with line and column numbers."*

### Q4: How is operator precedence handled?
> *"Precedence is handled hierarchically: `parse_condition()` handles `OR` by calling `parse_and_expr()`, which handles `AND` by calling `parse_rel_expr()`. This guarantees that `AND` binds tighter than `OR` in our AST without needing parentheses."*

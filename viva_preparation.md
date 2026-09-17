# TrafficRule Compiler: Review 2 Code Walkthrough & Viva Guide

This document gives you a **crisp, line-by-line explanation of every file and function** in your project. No fluff or overexplaining—just clear, direct points you can say to your teacher.

---

## 1. Quick Demonstration Commands

```powershell
# 1. Build the compiler (0 warnings)
.\build.bat

# 2. Run the automated test suite (7/7 tests pass)
powershell -ExecutionPolicy Bypass -File .\test_all.ps1

# 3. Run full 6-stage demonstration on sample
.\trafficrule.exe examples\sample2.tr --all
```

---

## 2. File-by-File Code Walkthrough (What to Say)

---

### `src/common.h` & `src/common.c`
**Purpose:** Defines shared enums, structs, and string conversion functions used across all files.

#### Key Structs & Enums:
1. **`TokenType`**: Enum listing every token the language supports:
   - Keywords: `TOKEN_KW_VEHICLE`, `TOKEN_KW_IF`, `TOKEN_KW_ELSE`, `TOKEN_KW_AND`, `TOKEN_KW_OR`
   - Actions: `TOKEN_ACT_ISSUE_FINE`, `TOKEN_ACT_FLAG_VIOLATION`, `TOKEN_ACT_ALLOW`
   - States: `TOKEN_STATE_RED`, `TOKEN_STATE_GREEN`, `TOKEN_STATE_YELLOW`, etc.
   - Operators: `TOKEN_ASSIGN` (`=`), `TOKEN_OP_GT` (`>`), `TOKEN_OP_EQ` (`==`), etc.
   - Literals: `TOKEN_IDENTIFIER`, `TOKEN_NUMBER`
2. **`Token` struct**:
   - `type`: The `TokenType` enum.
   - `lexeme[64]`: The exact text string from source code (e.g., `"speed"`, `"75"`).
   - `int_val`: Integer value if token is a number.
   - `line`, `col`: Source position for accurate error reporting.
3. **`DataType`**: `TYPE_INT`, `TYPE_STATE`, `TYPE_VEHICLE`, `TYPE_BOOL`. Used for semantic type checking.
4. **`common.c`**: Helper functions (`token_type_to_str`, etc.) that convert enum constants into printable strings.

---

### `src/lexer.h` & `src/lexer.c`
**Purpose:** The Scanner. Reads raw source code character-by-character and groups characters into tokens.

#### Key Struct:
* **`Lexer`**: Holds pointer to source string `source`, current index `cursor`, current line number `line`, and column number `col`.

#### Function-by-Function Explanation:
* **`lexer_init(Lexer* lexer, const char* source)`**:
  - Initializes cursor to index 0, line to 1, column to 1, and clears error flags.
* **`advance_char(Lexer* lexer)`**:
  - Fetches character at `cursor` and advances by 1.
  - If the character is `\n` (newline), increments `line` by 1 and resets `col` to 1. Otherwise, increments `col`.
* **`skip_whitespace_and_comments(Lexer* lexer)`**:
  - Loops past spaces (`' '`), tabs (`'\t'`), and newlines (`'\n'`).
  - If it encounters `'#'`, it skips all characters until the end of the line (handles comments).
* **`lexer_next_token(Lexer* lexer)`** (The main lexer function):
  - **Identifies words:** If character is a letter `[a-zA-Z_]`, loops until word ends. Compares lexeme with keywords (`VEHICLE`, `IF`, `ELSE`, `AND`, `OR`), states (`RED`, `GREEN`, etc.), actions (`ISSUE_FINE`, etc.). If no keyword matches, marks it as `TOKEN_IDENTIFIER`.
  - **Identifies numbers:** If character is a digit `[0-9]`, loops to collect all digits, calls `atoi()` to store integer value, returns `TOKEN_NUMBER`.
  - **Identifies multi-char operators:** For `=`, checks if next char is `=` to return `TOKEN_OP_EQ` (`==`), otherwise `TOKEN_ASSIGN` (`=`). Does the same for `!=`, `>=`, `<=`.
  - **Identifies delimiters:** Recognizes `{` and `}`.
  - **Handles Lexical Errors:** If any unrecognized character (like `@`) is encountered, sets `lexer->has_error = true` and records the line, column, and ASCII code.
* **`lexer_tokenize_all(...)`**:
  - Helper that calls `lexer_next_token` in a loop until `TOKEN_EOF`, allocating and returning a dynamic array of tokens (used for `--tokens` table display).

---

### `src/ast.h` & `src/ast.c`
**Purpose:** Defines the tree data structure representing the parsed syntax of the program.

#### Key Struct:
* **`ASTNode`**: Represents a node in the tree. Contains:
  - `type`: `AST_PROGRAM`, `AST_VAR_DECL`, `AST_ASSIGN`, `AST_IF_STMT`, `AST_BINARY_OP`, `AST_ACTION`, `AST_LITERAL_INT`, `AST_LITERAL_STATE`, `AST_IDENTIFIER`.
  - `line`, `col`: Source position.
  - **Tagged union**: Holds data specific to that node type (e.g., `if_stmt` holds pointers to `condition`, `then_block`, and `else_block`).

#### Function-by-Function Explanation:
* **`ast_create_*()` constructors**:
  - `ast_create_program()`: Allocates memory for a block of statements using a dynamic array.
  - `ast_create_var_decl()`, `ast_create_assign()`, `ast_create_if()`, `ast_create_binary_op()`: Allocates an `ASTNode` using `calloc(1, sizeof(ASTNode))` and fills in child node pointers and values.
* **`ast_print(node, indent)`**:
  - Recursively traverses the AST. Uses `indent * 2` spaces to visually print parent and child nodes in the terminal.
* **`ast_export_dot(node, filename)`**:
  - Traverses the AST and writes Graphviz DOT format to a file for external graphical rendering.
* **`ast_free(node)`**:
  - Recursively frees all allocated child pointers and statement arrays to prevent memory leaks.

---

### `src/parser.h` & `src/parser.c`
**Purpose:** Top-down Predictive Recursive Descent Parser. Checks grammar and builds the AST.

#### How It Works:
Maintains two lookahead tokens: `current` (token being inspected) and `peek` (next token in line).

#### Function-by-Function Explanation:
* **`advance(parser)`**: Moves `peek` into `current`, and fetches next token from lexer into `peek`.
* **`match(parser, type)`**: Returns true if `current.type == type`.
* **`consume(parser, type, error_msg)`**: If `current` matches expected token type, advances. If not, logs a syntax error with line/col and unexpected token.
* **`parse_value(parser)`**: Parses atomic values: checks if token is identifier, number, or traffic state (`RED`, `GREEN`, etc.).
* **`parse_rel_expr(parser)`**:
  - Parses left value $\to$ expects comparison operator (`>`, `<`, `==`, `!=`, `>=`, `<=`) $\to$ parses right value. Returns `AST_BINARY_OP`.
* **`parse_and_expr(parser)` & `parse_condition(parser)` (Operator Precedence)**:
  - `parse_condition()` handles `OR` (lower precedence).
  - Inside it, calls `parse_and_expr()` which handles `AND` (higher precedence).
  - This guarantees `AND` binds tighter than `OR` in grammar without parentheses.
* **`parse_declaration(parser)`**: Consumes `VEHICLE`, expects identifier.
* **`parse_assignment(parser)`**: Expects identifier, consumes `=`, parses value.
* **`parse_block(parser)`**: Consumes `{`, loops calling `parse_statement()` until `}`, consumes `}`.
* **`parse_if_stmt(parser)`**: Consumes `IF`, parses condition, parses then block. If next token is `ELSE`, consumes `ELSE` and parses else block.
* **`parse_statement(parser)`**: Dispatches to declaration, assignment, IF statement, or action depending on `current.type`.
* **`parser_parse(parser)`**: Entry point. Loops parsing statements until `TOKEN_EOF`, returning the root `AST_PROGRAM`.

---

### `src/symbol_table.h` & `src/symbol_table.c`
**Purpose:** Stores variable names, types, initialized values, and scope.

#### Key Structs:
* **`Symbol`**:
  - `name`: Variable name string.
  - `type`: Data type (`TYPE_INT`, `TYPE_STATE`, `TYPE_VEHICLE`).
  - `scope`: Always `"global"` in TrafficRule.
  - `int_val` / `state_val`: Current value stored.
  - `is_initialized`: Boolean flag indicating if a value has been assigned.
* **`SymbolTable`**: Dynamic array of `Symbol` records with `count` and `capacity`.

#### Function-by-Function Explanation:
* **`symbol_table_init(st)`**: Allocates initial capacity (16 entries).
* **`symbol_table_lookup(st, name)`**: Linear search through entries. Returns pointer to matching `Symbol` or `NULL`.
* **`symbol_table_insert(st, name, type, scope, line, col)`**: Checks if symbol already exists; if not, resizes array if full and appends new symbol.
* **`symbol_table_set_int(st, name, val)`** & **`symbol_table_set_state(st, name, state)`**: Updates the symbol's value and marks `is_initialized = true`.
* **`symbol_table_print(st)`**: Prints the clean ASCII border table with columns: `Name | Type | Value | Scope`.

---

### `src/semantic.h` & `src/semantic.c`
**Purpose:** Semantic Analyzer. Validates variable declarations, scopes, and enforces type checking.

#### Function-by-Function Explanation:
* **`infer_type(sem, node)`** (Type deduction function):
  - If node is integer literal $\to$ returns `TYPE_INT`.
  - If node is state literal $\to$ returns `TYPE_STATE`.
  - If node is identifier $\to$ looks up variable in symbol table. If not found, flags **"Undefined variable"** error.
  - If node is binary op (`AND`/`OR`) $\to$ checks both operands are `TYPE_BOOL`.
  - If node is relational op (`>`, `==`, etc.):
    - Rejects comparisons on vehicle objects.
    - **Enforces Type Matching:** If one side is `TYPE_INT` and other is `TYPE_STATE`, flags: **"Type Error: Incompatible comparison between 'integer' and 'state'"**.
    - **Validates Operations on States:** If comparing traffic states with `>` or `<`, flags: **"Type Error: Operator '>' is invalid for traffic states (only '==' and '!=' allowed)"**.
* **`check_node(sem, node)`**:
  - Traverses AST nodes:
  - For `AST_VAR_DECL`: Inserts vehicle into symbol table; flags error if already declared.
  - For `AST_ASSIGN`: Prevents assigning values to vehicle variables; checks RHS expression and updates symbol table.
  - For `AST_IF_STMT`: Asserts condition evaluates to `TYPE_BOOL`, then validates then/else blocks.
* **`semantic_analyze(sem, root)`**: Entry point running semantic checks across the entire AST.

---

### `src/ir.h` & `src/ir.c`
**Purpose:** Intermediate Representation. Generates linear Three-Address Code (TAC) and Quadruples table.

#### Key Structs:
* **`TACInstruction`**: Represents one TAC instruction / Quadruple:
  - `op`: Enum (`IR_ASSIGN`, `IR_BINARY_OP`, `IR_IF_FALSE_GOTO`, `IR_GOTO`, `IR_LABEL`, `IR_ACTION`).
  - `op_name`: String representation (e.g. `">"`, `"ASSIGN"`, `"IF_FALSE"`, `"LABEL"`).
  - `arg1`, `arg2`: Operand strings (variables, constants, or temporaries `t1`, `t2`).
  - `result`: Destination variable, temporary, or jump label (`L1`, `L2`).
* **`IRProgram`**: Array of `TACInstruction` with counters `temp_count` and `label_count`.

#### Function-by-Function Explanation:
* **`emit(ir, op, op_name, a1, a2, res)`**: Appends a new instruction to the dynamic TAC array.
* **`gen_temp(ir, out_buf)`**: Formats string `"t1"`, `"t2"`, etc., and increments counter.
* **`gen_label(ir, out_buf)`**: Formats string `"L1"`, `"L2"`, etc., and increments counter.
* **`gen_expr(ir, node, out_place)`**:
  - Recursively evaluates expression. For binary op: generates TAC `t1 = arg1 op arg2`, passes `t1` back up.
* **`gen_node(ir, node)`**:
  - For `AST_ASSIGN`: Emits `ASSIGN val -> result`.
  - For `AST_ACTION`: Emits `ACTION action_name`.
  - For `AST_IF_STMT`:
    1. Evaluates condition into temporary `t_cond`.
    2. Emits `IF_FALSE t_cond GOTO L_else`.
    3. Recursively generates code for then block.
    4. If else block exists: emits `GOTO L_end`, emits `L_else:`, generates else block, emits `L_end:`.
    5. If no else block: emits `L_else:` right after then block.
* **`ir_print_tac(ir)`**: Prints clean TAC syntax (`t1 = speed > 60`, `IF_FALSE t1 GOTO L1`).
* **`ir_print_quadruples(ir)`**: Prints standard 4-column Quadruple table `(# | Op | Arg1 | Arg2 | Result)`.

---

### `src/optimizer.h` & `src/optimizer.c`
**Purpose:** Intermediate code optimizer. Performs constant folding and dead branch pruning.

#### Function-by-Function Explanation:
* **`optimize_ir(input, output, stats)`**:
  - **Pass 1: Constant Folding**:
    - Maintains a table of known variable and temporary constant values.
    - If a binary operation has two constant operands (e.g., `80 > 60`), the optimizer computes the value at compile time (evaluates to `1`), replaces the binary op with an immediate assignment `t1 = 1`, and increments `stats->constant_folds`.
  - **Pass 2: Dead Branch Pruning & Redundant Jump Elimination**:
    - If an `IF_FALSE` condition has a known constant value of `1` (TRUE), it can never jump! The optimizer drops the `IF_FALSE` instruction entirely (`stats->branches_pruned++`).
    - If a `GOTO L1` instruction is immediately followed by label `L1:`, the jump is redundant and gets removed (`stats->dead_instructions_removed++`).

---

### `src/interpreter.h` & `src/interpreter.c`
**Purpose:** The Interpreter. Reads the Three-Address Code (TAC) line-by-line, evaluates conditions, and prints the simulated violation output.

#### Key Structs:
* **`RuntimeVar`**: Holds variable/temporary name, type, integer value, or state string at runtime.
* **`Interpreter`**: Array of `RuntimeVar` slots representing memory, plus violation/action counters.

#### Function-by-Function Explanation:
* **`interpreter_load_symbol_table(interp, st)`**: Loads the program's initialized variables (`speed = 75`, `signal = RED`) from the symbol table into runtime memory.
* **`resolve_operand(interp, op_str, is_num, num_val, str_val)`**: Helper to resolve whether an operand is a raw literal number, a state keyword (`RED`), or looks up a variable's value from runtime memory.
* **`interpreter_execute(interp, ir)`** (The interpreter loop):
  - Uses an instruction pointer / program counter `pc = 0`.
  - Loops line-by-line through the TAC instructions:
    - **`IR_ASSIGN`**: Updates target variable's value.
    - **`IR_BINARY_OP`**: Executes relational or logical comparison on operands and stores boolean result in destination temporary.
    - **`IR_IF_FALSE_GOTO`**: Checks condition temporary. If 0 (FALSE), searches `ir` for destination label and jumps `pc` to that instruction. Otherwise moves to next line.
    - **`IR_GOTO`**: Unconditionally sets `pc` to target label.
    - **`IR_ACTION`**: Executes rule enforcement action! Prints formatted violation notice:
      ```text
      Traffic Violation Detected
      Type: Speed / Threshold Violation
      Action: ISSUE_FINE
      ```
    - Increments violation and action counters.

---

### `src/main.c`
**Purpose:** Driver program and CLI orchestrator.

#### What It Does:
1. Parses command line flags:
   - `--tokens`: runs lexer only
   - `--ast`: runs parser and AST display
   - `--symbols`: runs semantic analysis and prints symbol table
   - `--ir`: runs IR generator and prints TAC + Quadruples
   - `--opt-ir`: runs optimizer and displays optimized TAC
   - `--run`: runs Interpreter execution engine
   - `--all`: runs all stages sequentially (default)
2. Reads the input `.tr` source file into a memory buffer.
3. Steps through each stage one by one:
   - Lexer $\to$ Parser $\to$ Semantic Analyzer $\to$ IR Generator $\to$ Optimizer $\to$ Interpreter.
4. If an error occurs at any stage, prints formatted error and halts without crashing.
5. Frees all allocated memory upon completion.

---

## 3. High-Scoring Viva Answers to Memorize

### Teacher: "Show me where you parse operator precedence."
**Point to:** `src/parser.c`, functions `parse_condition()` and `parse_and_expr()`.
> *"In `src/parser.c`, precedence is enforced grammatically. `parse_condition()` handles `OR` by calling `parse_and_expr()`. `parse_and_expr()` handles `AND` by calling `parse_rel_expr()`. Because the parser descends through `OR` first down into `AND`, `AND` expressions are grouped tighter in the AST than `OR` expressions."*

### Teacher: "Where is type checking done?"
**Point to:** `src/semantic.c`, function `infer_type()`.
> *"In `src/semantic.c`, inside `infer_type()`. When evaluating a relational operation, it checks if `left_type != right_type`. If a program attempts to compare `speed == RED`, it catches the mismatch and prints 'Type Error: Incompatible comparison between integer and state'."*

### Teacher: "What intermediate representation do you use?"
**Point to:** `src/ir.c`, function `ir_print_quadruples()`.
> *"We use Three-Address Code (TAC) with linear quadruples: `(Operator, Argument 1, Argument 2, Result)`. In `src/ir.c`, we generate temporaries `t1`, `t2` for expressions and control flow labels `L1`, `L2` for conditional jumps."*

### Teacher: "How does your optimizer work?"
**Point to:** `src/optimizer.c`, function `optimize_ir()`.
> *"In `src/optimizer.c`, it runs two passes. Pass 1 does Constant Folding: if both operands of a comparison are constant values (like `80 > 60`), it computes the result at compile-time and simplifies it to `t1 = 1`. Pass 2 eliminates dead branches: if an `IF_FALSE` condition is constant 1 (TRUE), the jump is never taken, so the instruction is pruned."*

### Teacher: "What happens during execution / simulation? What is the interpreter?"
**Point to:** `src/interpreter.c`, function `interpreter_execute()`.
> *"In `src/interpreter.c`, the interpreter maintains a memory table of variables and loops through the Three-Address Code line-by-line using a program counter (`pc`). It calculates comparisons, jumps on `IF_FALSE`, and when an action like `ISSUE_FINE` or `FLAG_VIOLATION` is reached, it outputs the traffic violation record."*

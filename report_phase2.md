# TrafficRule: Domain-Specific Language and Compiler for Traffic Rule Enforcement
## Phase 2: Core Implementation Progress Report

**Course Code:** BCSE307L  
**Course Title:** Compiler Design Laboratory  
**Author:** Alan James  
**Registration Number:** 24BCT0181  
**Semester:** Fall Semester 2026–27  

---

## 1. Executive Summary

In Phase 2 (Core Implementation), all proposed compiler modules for the **TrafficRule** Domain-Specific Language (DSL) were developed from scratch in **pure C (C99 standard)** with zero external dependencies. The compiler compiles into a standalone binary (`trafficrule.exe`) and features a complete end-to-end pipeline:
1. **Lexical Analysis (Scanner)**: Tokenizes language constructs with exact classification and line/column tracking.
2. **Syntax Analysis (Predictive Recursive Descent Parser)**: Validates programs against formal grammar specifications.
3. **Abstract Syntax Tree (AST)**: Builds hierarchical representations with ASCII visualizer and Graphviz `.dot` exporter.
4. **Symbol Table Management**: Tracks identifier types (`vehicle`, `integer`, `state`), values, and scopes (`global`).
5. **Semantic Analysis**: Enforces type compatibility, variable declaration rules, and action validation.
6. **Intermediate Code Generation (IR)**: Produces Three-Address Code (TAC) and standard Quadruple tables `(Op, Arg1, Arg2, Result)`.
7. **Code Optimization**: Implements compile-time constant expression folding and dead branch/jump elimination.
8. **Execution Engine (Virtual Machine)**: Interprets intermediate instructions to evaluate traffic enforcement rules on simulated telemetry.
9. **Multi-Stage Error Handler**: Accurately flags Lexical, Syntax, Semantic, and Type errors with line and column precision.

---

## 2. Compiler Architecture & Pipeline Flow

```
+-----------------------------+
| TrafficRule Source (*.tr)   |
+-----------------------------+
               |
               v
+-----------------------------+
| Lexical Analyzer (lexer.c)  | ---> Token Stream & Lexical Errors
+-----------------------------+
               |
               v
+-----------------------------+
| Syntax Analyzer (parser.c)  | ---> Grammar Validation & Syntax Errors
+-----------------------------+
               |
               v
+-----------------------------+
| Abstract Syntax Tree (ast.c)| ---> Hierarchical AST & Graphviz DOT
+-----------------------------+
               |
               v
+-----------------------------+
| Semantic Analyzer(semantic.c| <---> Symbol Table (symbol_table.c)
| Type & Scope Validation     | ---> Type & Semantic Errors
+-----------------------------+
               |
               v
+-----------------------------+
| IR Generator (ir.c)         | ---> Three-Address Code (TAC) & Quadruples
+-----------------------------+
               |
               v
+-----------------------------+
| Optimizer (optimizer.c)     | ---> Constant Folding & Branch Pruning
+-----------------------------+
               |
               v
+-----------------------------+
| Execution Engine (vm.c)     | ---> Violation Detection & Fine Notices
+-----------------------------+
```

---

## 3. Detailed Module-Wise Explanations

### 3.1 Lexical Analyzer (`lexer.h`, `lexer.c`)
- **Role:** Scans the input character stream and groups lexemes into strongly-typed tokens.
- **Tokens Recognized:**
  - `KEYWORD`: `VEHICLE`, `IF`, `ELSE`, `AND`, `OR`
  - `STATE`: `RED`, `GREEN`, `YELLOW`, `CROSSING`, `ROAD`
  - `ACTION`: `ISSUE_FINE`, `FLAG_VIOLATION`, `ALLOW`
  - `OPERATOR`: `=`, `==`, `!=`, `>`, `<`, `>=`, `<=`
  - `DELIMITER`: `{`, `}`
  - `IDENTIFIER`: `[a-zA-Z_][a-zA-Z0-9_]*`
  - `NUMBER`: `[0-9]+`
- **Error Detection:** Detects invalid characters (e.g., `@`, `$`, `~`), reports exact line, column, and ASCII code.

### 3.2 Syntax Analysis & AST Construction (`parser.h`, `parser.c`, `ast.h`, `ast.c`)
- **Role:** Employs predictive recursive descent parsing to verify whether tokens conform to the language grammar.
- **Formal Grammar:**
  ```ebnf
  program        -> statement*
  statement      -> declaration | assignment | if_stmt | action
  declaration    -> "VEHICLE" identifier
  assignment     -> identifier "=" value
  if_stmt        -> "IF" condition block ("ELSE" block)?
  condition      -> and_expr ("OR" and_expr)*
  and_expr       -> rel_expr ("AND" rel_expr)*
  rel_expr       -> value relational_op value
  block          -> "{" statement* "}"
  action         -> "ISSUE_FINE" | "FLAG_VIOLATION" | "ALLOW"
  value          -> identifier | number | RED | GREEN | YELLOW | CROSSING | ROAD
  relational_op  -> ">" | "<" | ">=" | "<=" | "==" | "!="
  ```
- **AST Generation:** Produces tree nodes with specific variants (`AST_PROGRAM`, `AST_VAR_DECL`, `AST_ASSIGN`, `AST_IF_STMT`, `AST_BINARY_OP`, `AST_ACTION`, `AST_LITERAL_INT`, `AST_LITERAL_STATE`).

### 3.3 Symbol Table Management (`symbol_table.h`, `symbol_table.c`)
- **Role:** Central repository for identifier metadata across compiler passes.
- **Fields Maintained:**
  - `Name`: Identifier string
  - `Type`: `integer`, `state`, `vehicle`
  - `Value`: Current initialized value
  - `Scope`: `global`
- **Display:** Implements tabular formatting matching laboratory submission specifications.

### 3.4 Semantic Analysis (`semantic.h`, `semantic.c`)
- **Role:** Verifies program semantics beyond grammatical structure:
  - **Undeclared Variables:** Rejects identifiers used without prior declaration or assignment.
  - **Type Checking:** Ensures operations are type-safe (e.g., integers can only be compared with integers; states can only be checked for equality/inequality).
  - **Disallows Illegal Operations:** Prohibits comparing an integer `speed` with a state `RED`, and forbids relational ordering (`>`, `<`) on states.
  - **Declaration Guard:** Prevents re-assigning values to vehicle declarations.

### 3.5 Intermediate Code Generator (`ir.h`, `ir.c`)
- **Role:** Translates the high-level AST into a machine-independent linear intermediate representation.
- **Output Formats:**
  1. **Three-Address Code (TAC):** Linear statements with generated temporaries (`t1`, `t2`, ...) and control-flow labels (`L1`, `L2`, ...).
  2. **Quadruples Table:** Standard `(Index, Op, Arg1, Arg2, Result)` tabular format.

### 3.6 Intermediate Code Optimizer (`optimizer.h`, `optimizer.c`)
- **Techniques Implemented:**
  - **Constant Folding:** Statically evaluates comparisons whose operands are constants (e.g., `80 > 60` folds to `1`).
  - **Dead Branch Pruning:** When an `IF_FALSE` condition is proven constant true (`1`), the branch jump is pruned.
  - **Redundant Jump Elimination:** Redundant jumps targeting immediate next instructions are removed.

### 3.7 Execution Engine / Virtual Machine (`vm.h`, `vm.c`)
- **Role:** Simulates runtime rule enforcement by stepping through TAC instructions.
- **Actions Enforced:**
  - `ISSUE_FINE`: Triggered on threshold violations (e.g., overspeeding).
  - `FLAG_VIOLATION`: Triggered on safety violations (e.g., running a red light while crossing).
  - `ALLOW`: Confirms traffic compliance.

---

## 4. Intermediate Results & Demonstration

### Sample Program (`sample2.tr`)
```trafficrule
VEHICLE car

speed = 75
signal = RED
position = CROSSING

IF speed > 60 {
    ISSUE_FINE
}

IF signal == RED AND position == CROSSING {
    FLAG_VIOLATION
}
```

### Generated Intermediate Results

#### 1. Symbol Table Output
```
+----------------------+------------+---------------+----------+
| Name                 | Type       | Value         | Scope    |
+----------------------+------------+---------------+----------+
| car                  | vehicle    | -             | global   |
| speed                | integer    | 75            | global   |
| signal               | state      | RED           | global   |
| position             | state      | CROSSING      | global   |
+----------------------+------------+---------------+----------+
```

#### 2. Three-Address Code (TAC) Output
```text
  speed = 75
  signal = RED
  position = CROSSING
  t1 = speed > 60
  IF_FALSE t1 GOTO L1
  ISSUE_FINE
L1:
  t2 = signal == RED
  t3 = position == CROSSING
  t4 = t2 AND t3
  IF_FALSE t4 GOTO L2
  FLAG_VIOLATION
L2:
```

#### 3. Quadruple Table Output
```
+----+---------------+---------------+---------------+---------------+
| #  | Op            | Arg 1         | Arg 2         | Result        |
+----+---------------+---------------+---------------+---------------+
| 0  | ASSIGN        | 75            | -             | speed         |
| 1  | ASSIGN        | RED           | -             | signal        |
| 2  | ASSIGN        | CROSSING      | -             | position      |
| 3  | >             | speed         | 60            | t1            |
| 4  | IF_FALSE      | t1            | -             | L1            |
| 5  | ACTION        | ISSUE_FINE    | -             | -             |
| 6  | LABEL         | -             | -             | L1            |
| 7  | ==            | signal        | RED           | t2            |
| 8  | ==            | position      | CROSSING      | t3            |
| 9  | AND           | t2            | t3            | t4            |
| 10 | IF_FALSE      | t4            | -             | L2            |
| 11 | ACTION        | FLAG_VIOLATION| -             | -             |
| 12 | LABEL         | -             | -             | L2            |
+----+---------------+---------------+---------------+---------------+
```

#### 4. Simulation Execution Output
```
=========================================
Traffic Violation Detected
Type: Speed / Threshold Violation
Action: ISSUE_FINE
=========================================

=========================================
Traffic Violation Detected
Type: Traffic Signal / Crossing Violation
Action: FLAG_VIOLATION
=========================================

Simulation Summary:
  - Actions Triggered     : 2
  - Violations Enforced   : 2
  - Execution Status      : SUCCESS
```

---

## 5. Test Suite & Verification Matrix

The test suite is automated via `test_all.ps1`. All 7 test categories pass with 100% precision:

| Test Case | Scenario Description | Expected Outcome | Actual Result |
| :--- | :--- | :--- | :--- |
| `sample1.tr` | Speed threshold violation | TAC generated, `ISSUE_FINE` executed | **PASS** |
| `sample2.tr` | Multi-rule speed & red signal crossing | Compound AND condition, 2 violations | **PASS** |
| `sample3_else.tr` | IF-ELSE branching with `ALLOW` | Condition fails, ELSE branch triggered | **PASS** |
| `sample_opt.tr` | Constant condition evaluation | Folded `80 > 60` to 1, pruned jump | **PASS** |
| `err_lexical.tr` | Invalid character (`@`) | Catches `Lexical Error` at line 3, col 12 | **PASS** |
| `err_syntax.tr` | Incomplete comparison (`IF speed > {`) | Catches `Syntax Error` at line 5, col 12 | **PASS** |
| `err_semantic.tr` | Type mismatch (`speed == RED`) | Catches `Type Error` at line 5, col 4 | **PASS** |

---

## 6. Review 2 Evaluation Criteria Mapping

| Evaluation Criterion | Implementation Evidence in TrafficRule |
| :--- | :--- |
| **Implementation Progress** | 100% of Phase 2 components implemented (Lexer, Parser, AST, Semantic, Symbol Table, IR, Optimizer, VM). |
| **Functional Correctness** | Rigorously verified via automated PowerShell regression test matrix with zero failures. |
| **Compiler Concepts** | Implements standard compiler theory: finite automaton tokenization, recursive descent LL parsing, AST transformations, TAC/Quadruples, and constant folding. |
| **Code Quality** | Modular, well-commented C99 code adhering to clean coding standards, free of memory leaks and compiling with 0 warnings under `-Wall -Wextra`. |
| **Problem Solving** | Handled complex multi-condition boolean logic (`AND`/`OR`) and type compatibility checks between integers and domain traffic states. |
| **Innovation** | Domain-specific traffic semantics, built-in optimizer for rule simplification, and native simulated rule execution. |

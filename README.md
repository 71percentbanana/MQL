# SQL2Mongo: SQL to MongoDB Query Compiler & Execution Engine

A formal source-to-source query compiler (transpiler) and execution engine written in **C** with zero external dependencies. It translates declarative relational SQL queries into MongoDB Aggregation Pipelines (`db.collection.aggregate([ ... ])`) and executes them against in-memory JSON document collections to produce actual MongoDB query results.

---

## Compiler Architecture & Pipeline Stages

```
Input SQL Query (*.sql)
       │
       ▼
1. Lexical Analysis (Scanner: sql_lexer.c)
   └── Produces typed token stream with line & column tracking
       │
       ▼
2. Syntax Analysis (Recursive Descent Parser: sql_parser.c)
   └── Validates SQL grammar & builds Abstract Syntax Tree (AST: sql_ast.c)
       │
       ▼
3. Symbol Table & Semantic Analysis (schema_table.c)
   └── Validates collection schemas, field existence, and operand type compatibility
       │
       ▼
4. Intermediate Code Generation (tac.c)
   └── Generates Three-Address Code (TAC) and Quadruple Table (Op, Arg1, Arg2, Result)
       │
       ▼
5. Code Optimization (optimizer.c)
   └── Applies Predicate Pushdown, Column Pruning, and Compile-Time Constant Folding
       │
       ▼
6. Target Code Generation (mongo_codegen.c)
   └── Serializes AST to standard MongoDB Aggregation Pipeline JSON ($match, $sort, $limit, $project)
       │
       ▼
7. Execution Engine (engine.c)
   └── Filters, sorts, and projects JSON document collections, returning real MongoDB documents
```

---

## Quick Build & Run

### 1. Build
```cmd
build.bat
```
or manually with GCC:
```cmd
gcc -std=c99 -Wall -Wextra src/*.c -o sql2mongo.exe
```

### 2. Run Automated Test Suite
```powershell
powershell -ExecutionPolicy Bypass -File .\test_all.ps1
```

### 3. Run Pipeline on Sample Files or Inline Queries
```powershell
# Run complete end-to-end compiler pipeline on a file
.\sql2mongo.exe examples/query1_basic.sql --all

# Run specific stage flags
.\sql2mongo.exe examples/query1_basic.sql --tokens   # Scanner Token Table
.\sql2mongo.exe examples/query1_basic.sql --ast      # Hierarchical AST
.\sql2mongo.exe examples/query1_basic.sql --schema   # Symbol Table
.\sql2mongo.exe examples/query1_basic.sql --tac      # Three-Address Code & Quadruples
.\sql2mongo.exe examples/query1_basic.sql --opt      # Code Optimization Report
.\sql2mongo.exe examples/query1_basic.sql --mongo    # Target MQL JSON
.\sql2mongo.exe examples/query1_basic.sql --run      # In-Memory Execution Results

# Execute direct inline SQL query string
.\sql2mongo.exe "SELECT name, age FROM students WHERE age >= 20 ORDER BY age DESC LIMIT 2;" --all
```

---

## Project Structure

```
compiler-project/
├── README.md                    # Project documentation
├── build.bat                    # 1-Click GCC build script
├── test_all.ps1                 # Automated PowerShell test matrix
├── data/                        # JSON document collections
│   ├── students.json
│   └── employees.json
├── examples/                    # Sample test queries
│   ├── query1_basic.sql         # SELECT, WHERE, ORDER BY, LIMIT
│   ├── query2_compound.sql      # Multi-condition with AND
│   ├── query3_wildcard.sql      # SELECT * wildcard projection
│   ├── query4_or.sql            # Multi-condition with OR
│   ├── err_syntax.sql           # Syntax error diagnostic test
│   └── err_semantic.sql         # Semantic schema error diagnostic test
└── src/                         # C compiler source code
    ├── sql_lexer.h / .c         # Lexical scanner & token dictionary
    ├── sql_ast.h / .c           # AST node definitions & ASCII tree visualizer
    ├── sql_parser.h / .c        # Top-down recursive descent parser
    ├── schema_table.h / .c      # Symbol table catalog & semantic type checker
    ├── tac.h / .c               # Three-Address Code & Quadruple table
    ├── optimizer.h / .c         # Constant folding & predicate pushdown
    ├── query_plan.h / .c        # Relational algebra IR
    ├── mongo_codegen.h / .c     # MongoDB Aggregation Pipeline JSON code generator
    ├── engine.h / .c            # Document execution engine & result renderer
    └── main.c                   # CLI orchestration driver
```

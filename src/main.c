#include "common.h"
#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "symbol_table.h"
#include "semantic.h"
#include "ir.h"
#include "optimizer.h"
#include "interpreter.h"

static char* read_entire_file(const char* filepath) {
    FILE* f = fopen(filepath, "rb");
    if (!f) {
        fprintf(stderr, "Error: Could not open file '%s'\n", filepath);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* buf = (char*)malloc(sz + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }
    size_t read_bytes = fread(buf, 1, sz, f);
    buf[read_bytes] = '\0';
    fclose(f);
    return buf;
}

static void print_header(const char* stage_name) {
    printf("\n======================================================================\n");
    printf("  STAGE: %s\n", stage_name);
    printf("======================================================================\n");
}

static void print_usage(const char* prog_name) {
    printf("TrafficRule Compiler (BCSE307L Compiler Design Laboratory)\n");
    printf("Usage: %s <source_file.tr> [options]\n\n", prog_name);
    printf("Options:\n");
    printf("  --tokens       Display lexical token stream\n");
    printf("  --ast          Display Abstract Syntax Tree (AST)\n");
    printf("  --dot <file>   Export AST to Graphviz DOT format\n");
    printf("  --symbols      Display populated Symbol Table\n");
    printf("  --ir           Display Three-Address Code (TAC) and Quadruples\n");
    printf("  --opt-ir       Display Optimized Intermediate Representation\n");
    printf("  --run          Execute compiled rule on simulation engine\n");
    printf("  --all          Run all compiler stages sequentially (Default)\n");
    printf("  --help         Show this help message\n");
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    const char* filepath = NULL;
    bool flag_tokens = false;
    bool flag_ast = false;
    bool flag_symbols = false;
    bool flag_ir = false;
    bool flag_opt = false;
    bool flag_run = false;
    bool flag_all = false;
    const char* dot_filename = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--tokens") == 0) flag_tokens = true;
        else if (strcmp(argv[i], "--ast") == 0) flag_ast = true;
        else if (strcmp(argv[i], "--symbols") == 0) flag_symbols = true;
        else if (strcmp(argv[i], "--ir") == 0) flag_ir = true;
        else if (strcmp(argv[i], "--opt-ir") == 0) flag_opt = true;
        else if (strcmp(argv[i], "--run") == 0) flag_run = true;
        else if (strcmp(argv[i], "--all") == 0) flag_all = true;
        else if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
        else if (strcmp(argv[i], "--dot") == 0 && i + 1 < argc) {
            dot_filename = argv[++i];
        }
        else if (argv[i][0] != '-') {
            filepath = argv[i];
        }
    }

    if (!filepath) {
        fprintf(stderr, "Error: No source file provided.\n");
        print_usage(argv[0]);
        return 1;
    }

    // Default to --all if no specific stage flag requested
    if (!flag_tokens && !flag_ast && !flag_symbols && !flag_ir && !flag_opt && !flag_run && !dot_filename) {
        flag_all = true;
    }

    char* source = read_entire_file(filepath);
    if (!source) return 1;

    printf(">>> Compiling TrafficRule Program: '%s' <<<\n", filepath);

    // =========================================================================
    // Stage 1: Lexical Analysis
    // =========================================================================
    if (flag_tokens || flag_all) {
        print_header("1. Lexical Analysis (Scanner)");
        Token* tokens = NULL;
        int token_count = 0;
        char lex_err[256];

        if (lexer_tokenize_all(source, &tokens, &token_count, lex_err, sizeof(lex_err)) != 0) {
            fprintf(stderr, "\n[FAILED] %s\n", lex_err);
            free(source);
            return 1;
        }

        printf("+-----+--------------------+--------------------+------+------+\n");
        printf("| #   | Token Type         | Lexeme             | Line | Col  |\n");
        printf("+-----+--------------------+--------------------+------+------+\n");
        for (int i = 0; i < token_count; i++) {
            printf("| %-3d | %-18s | %-18s | %-4d | %-4d |\n",
                   i + 1,
                   token_type_to_str(tokens[i].type),
                   tokens[i].lexeme,
                   tokens[i].line,
                   tokens[i].col);
        }
        printf("+-----+--------------------+--------------------+------+------+\n");
        printf("Total Tokens Identified: %d\n", token_count);
        free(tokens);

        if (flag_tokens && !flag_all) {
            free(source);
            return 0;
        }
    }

    // =========================================================================
    // Stage 2: Syntax Analysis & AST Construction
    // =========================================================================
    Parser parser;
    parser_init(&parser, source);
    ASTNode* ast = parser_parse(&parser);

    if (parser.has_error || !ast) {
        print_header("2. Syntax Analysis (Parser)");
        fprintf(stderr, "\n[SYNTAX ERROR] Line %d, Col %d: %s\n",
                parser.error_line, parser.error_col, parser.error_msg);
        free(source);
        return 1;
    }

    if (flag_ast || flag_all) {
        print_header("2. Syntax Analysis & Abstract Syntax Tree (AST)");
        printf("AST Structure:\n");
        ast_print(ast, 1);

        if (dot_filename) {
            ast_export_dot(ast, dot_filename);
            printf("\n[INFO] AST exported to Graphviz DOT file: '%s'\n", dot_filename);
        }
        if (flag_ast && !flag_all) {
            ast_free(ast);
            free(source);
            return 0;
        }
    }

    // =========================================================================
    // Stage 3: Semantic Analysis & Symbol Table Management
    // =========================================================================
    SymbolTable symtab;
    symbol_table_init(&symtab);

    SemanticAnalyzer sem;
    semantic_init(&sem, &symtab);

    bool sem_ok = semantic_analyze(&sem, ast);
    if (!sem_ok || sem.has_error) {
        print_header("3. Semantic Analysis");
        fprintf(stderr, "\n[SEMANTIC ERROR] Line %d, Col %d: %s\n",
                sem.error_line, sem.error_col, sem.error_msg);
        symbol_table_free(&symtab);
        ast_free(ast);
        free(source);
        return 1;
    }

    if (flag_symbols || flag_all) {
        print_header("3. Semantic Analysis & Symbol Table Management");
        printf("Semantic checks passed successfully.\n");
        printf("Symbol Table:\n");
        symbol_table_print(&symtab);

        if (flag_symbols && !flag_all) {
            symbol_table_free(&symtab);
            ast_free(ast);
            free(source);
            return 0;
        }
    }

    // =========================================================================
    // Stage 4: Intermediate Code Generation (Three-Address Code & Quadruples)
    // =========================================================================
    IRProgram ir;
    ir_init(&ir);
    ir_generate(&ir, ast);

    if (flag_ir || flag_all) {
        print_header("4. Intermediate Code Generation (TAC & Quadruples)");
        printf("Three-Address Code (TAC):\n");
        ir_print_tac(&ir);
        printf("\nQuadruple Representation:\n");
        ir_print_quadruples(&ir);

        if (flag_ir && !flag_all) {
            ir_free(&ir);
            symbol_table_free(&symtab);
            ast_free(ast);
            free(source);
            return 0;
        }
    }

    // =========================================================================
    // Stage 5: Code Optimization
    // =========================================================================
    IRProgram opt_ir;
    OptimizationStats stats;
    optimize_ir(&ir, &opt_ir, &stats);

    if (flag_opt || flag_all) {
        print_header("5. Intermediate Code Optimization");
        printf("Optimized Three-Address Code (TAC):\n");
        ir_print_tac(&opt_ir);
        printf("\nOptimized Quadruple Representation:\n");
        ir_print_quadruples(&opt_ir);

        if (flag_opt && !flag_all) {
            ir_free(&opt_ir);
            ir_free(&ir);
            symbol_table_free(&symtab);
            ast_free(ast);
            free(source);
            return 0;
        }
    }

    // =========================================================================
    // Stage 6: Interpreter / Execution Engine (Simulation)
    // =========================================================================
    if (flag_run || flag_all) {
        print_header("6. Interpreter / Execution Engine (Simulated Environment)");
        Interpreter interp;
        interpreter_init(&interp);
        interpreter_load_symbol_table(&interp, &symtab);

        printf("Initializing Interpreter with environment symbols...\n");
        printf("Executing compiled instructions line-by-line...\n");
        bool exec_ok = interpreter_execute(&interp, &opt_ir);

        if (exec_ok) {
            printf("\nSimulation Summary:\n");
            printf("  - Actions Triggered     : %d\n", interp.actions_executed);
            printf("  - Violations Enforced   : %d\n", interp.violations_count);
            printf("  - Execution Status      : SUCCESS\n");
        } else {
            printf("\nSimulation terminated with runtime error.\n");
        }
    }

    // Cleanup
    ir_free(&opt_ir);
    ir_free(&ir);
    symbol_table_free(&symtab);
    ast_free(ast);
    free(source);

    printf("\n>>> Compilation and Pipeline Execution Completed Successfully <<<\n");
    return 0;
}

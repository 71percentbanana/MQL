#include "common.h"
#include "lexer.h"
#include "ast.h"
#include "parser.h"

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
    printf("TrafficRule Compiler - Prototype (Lexer & Parser)\n");
    printf("Course: BCSE307L Compiler Design Laboratory\n\n");
    printf("Usage: %s <source_file.tr> [options]\n\n", prog_name);
    printf("Options:\n");
    printf("  --tokens       Display lexical token stream table\n");
    printf("  --ast          Display Abstract Syntax Tree (AST)\n");
    printf("  --dot <file>   Export AST to Graphviz DOT format\n");
    printf("  --all          Run both Lexer and Parser stages (Default)\n");
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
    bool flag_all = false;
    const char* dot_filename = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--tokens") == 0) flag_tokens = true;
        else if (strcmp(argv[i], "--ast") == 0) flag_ast = true;
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

    // Default to --all if neither --tokens nor --ast is specified
    if (!flag_tokens && !flag_ast && !dot_filename) {
        flag_all = true;
    }

    char* source = read_entire_file(filepath);
    if (!source) return 1;

    printf(">>> Processing TrafficRule Program: '%s' <<<\n", filepath);

    // =========================================================================
    // Stage 1: Lexical Analysis (Scanner)
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
    // Stage 2: Syntax Analysis (Parser & AST)
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
    }

    ast_free(ast);
    free(source);

    printf("\n>>> Parsing Completed Successfully <<<\n");
    return 0;
}

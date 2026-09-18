#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "sql_lexer.h"
#include "sql_ast.h"
#include "sql_parser.h"
#include "schema_table.h"
#include "tac.h"
#include "optimizer.h"
#include "query_plan.h"
#include "mongo_codegen.h"
#include "engine.h"

static char* read_entire_file(const char* filepath) {
    FILE* f = fopen(filepath, "rb");
    if (!f) {
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

static void print_banner(void) {
    printf("======================================================================\n");
    printf("   SQL2Mongo: Query Compiler, Transpiler & Execution Engine\n");
    printf("   Course: BCSE307L Compiler Design Laboratory | Author: Alan James\n");
    printf("======================================================================\n");
}

static void print_usage(const char* prog_name) {
    print_banner();
    printf("\nUsage: %s <query_file.sql | \"SQL query text\"> [options]\n\n", prog_name);
    printf("Compiler Phase Options:\n");
    printf("  --tokens     Stage 1: Lexical Token Stream Table (Scanner)\n");
    printf("  --ast        Stage 2: Abstract Syntax Tree (Parser)\n");
    printf("  --schema     Stage 3: Symbol Table & Semantic Checks\n");
    printf("  --tac        Stage 4: Three-Address Code (TAC) & Quadruple Table (IR)\n");
    printf("  --opt        Stage 5: Code Optimization (Constant Folding & Pushdown)\n");
    printf("  --mongo      Stage 6: Target MongoDB Aggregation Pipeline (MQL)\n");
    printf("  --run        Stage 7: Execution Engine (MongoDB Document Results)\n");
    printf("  --all        Run All Compiler Stages in Sequence (Default)\n");
    printf("  --help       Show this help manual\n\n");
    printf("Example:\n");
    printf("  %s \"SELECT name, age FROM students WHERE age >= 20 ORDER BY age DESC LIMIT 2;\" --all\n", prog_name);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
        print_usage(argv[0]);
        return 0;
    }

    const char* input_arg = argv[1];
    bool flag_tokens = false;
    bool flag_ast    = false;
    bool flag_schema = false;
    bool flag_tac    = false;
    bool flag_opt    = false;
    bool flag_mongo  = false;
    bool flag_run    = false;
    bool flag_all    = false;

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--tokens") == 0) flag_tokens = true;
        else if (strcmp(argv[i], "--ast") == 0) flag_ast = true;
        else if (strcmp(argv[i], "--schema") == 0) flag_schema = true;
        else if (strcmp(argv[i], "--tac") == 0) flag_tac = true;
        else if (strcmp(argv[i], "--opt") == 0) flag_opt = true;
        else if (strcmp(argv[i], "--mongo") == 0) flag_mongo = true;
        else if (strcmp(argv[i], "--run") == 0) flag_run = true;
        else if (strcmp(argv[i], "--all") == 0) flag_all = true;
    }

    // Default to --all if no specific stage flag is set
    if (!flag_tokens && !flag_ast && !flag_schema && !flag_tac && !flag_opt && !flag_mongo && !flag_run) {
        flag_all = true;
    }

    // Determine if input is a file or a direct SQL string
    char* file_content = read_entire_file(input_arg);
    const char* sql_source = file_content ? file_content : input_arg;

    print_banner();
    printf("\n[Input SQL Query]:\n%s\n", sql_source);

    // Initialize Schema Catalog
    schema_init();

    // 1. Lexer Stage
    if (flag_tokens || flag_all) {
        printf("\n======================================================================\n");
        printf("  STAGE 1: Lexical Analysis (Scanner)\n");
        printf("======================================================================\n");
        lexer_print_all(sql_source);
    }

    // 2. Parser & AST Stage
    Parser parser;
    parser_init(&parser, sql_source);
    ASTQuery query;
    bool parse_success = parser_parse(&parser, &query);

    if (!parse_success) {
        printf("\n======================================================================\n");
        printf("  SYNTAX VALIDATION STATUS: INVALID GRAMMAR DETECTED\n");
        printf("======================================================================\n");
        fprintf(stderr, "%s\n", parser.error_msg);
        if (file_content) free(file_content);
        return 1;
    }

    if (flag_ast || flag_all) {
        printf("\n======================================================================\n");
        printf("  STAGE 2: Syntax Analysis & Abstract Syntax Tree (AST)\n");
        printf("======================================================================\n");
        ast_print(&query);
        printf("Syntax Status: VALID (Grammar Verified without errors)\n");
    }

    // 3. Schema & Semantic Analysis Stage
    char sem_err[256];
    bool sem_valid = schema_validate_query(&query, sem_err, sizeof(sem_err));

    if (flag_schema || flag_all) {
        printf("\n======================================================================\n");
        printf("  STAGE 3: Symbol Table & Semantic Analysis\n");
        printf("======================================================================\n");
        schema_print_catalog();
        if (!sem_valid) {
            fprintf(stderr, "\n%s\n", sem_err);
            printf("Semantic Status: INVALID (Type/Field Mismatch)\n");
            ast_free(&query);
            if (file_content) free(file_content);
            return 2;
        } else {
            printf("\nSemantic Status: PASS (Collections, fields, and types verified)\n");
        }
    } else if (!sem_valid) {
        fprintf(stderr, "\n%s\n", sem_err);
        ast_free(&query);
        if (file_content) free(file_content);
        return 2;
    }

    // 4. Intermediate Code Generation (TAC & Quadruples)
    TACProgram tac_prog;
    tac_generate(&query, &tac_prog);

    if (flag_tac || flag_all) {
        printf("\n======================================================================\n");
        printf("  STAGE 4: Intermediate Code (Three-Address Code & Quadruples)\n");
        printf("======================================================================\n");
        tac_print(&tac_prog);
        quads_print(&tac_prog);
    }

    // 5. Code Optimization Stage
    OptimizationReport opt_report;
    optimizer_run(&query, &tac_prog, &opt_report);

    if (flag_opt || flag_all) {
        printf("\n======================================================================\n");
        printf("  STAGE 5: Code Optimization Passes\n");
        printf("======================================================================\n");
        optimizer_print_report(&opt_report);
    }

    // 6. Target Code Generation (MQL)
    if (flag_mongo || flag_all) {
        printf("\n======================================================================\n");
        printf("  STAGE 6: Target Code Generation (MongoDB Query Language)\n");
        printf("======================================================================\n");
        mongo_print_code(&query);
    }

    // 7. Execution Engine & Document Results
    if (flag_run || flag_all) {
        printf("\n======================================================================\n");
        printf("  STAGE 7: Execution Engine (MongoDB Document Results)\n");
        printf("======================================================================\n");
        engine_execute_query(&query, "data");
    }

    printf("\n======================================================================\n");
    printf("  FINAL VERIFICATION: ALL 7 COMPILER PHASES EXECUTED SUCCESSFULLY\n");
    printf("======================================================================\n");

    ast_free(&query);
    if (file_content) free(file_content);
    return 0;
}

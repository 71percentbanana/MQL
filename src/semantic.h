#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "common.h"
#include "ast.h"
#include "symbol_table.h"

typedef struct {
    SymbolTable* symtab;
    bool has_error;
    char error_msg[512];
    int error_line;
    int error_col;
} SemanticAnalyzer;

void semantic_init(SemanticAnalyzer* sem, SymbolTable* st);
bool semantic_analyze(SemanticAnalyzer* sem, const ASTNode* root);

#endif // SEMANTIC_H

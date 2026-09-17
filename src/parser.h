#ifndef PARSER_H
#define PARSER_H

#include "common.h"
#include "ast.h"
#include "lexer.h"

typedef struct {
    Lexer lexer;
    Token current;
    Token peek;

    bool has_error;
    char error_msg[512];
    int error_line;
    int error_col;
} Parser;

void parser_init(Parser* parser, const char* source);
ASTNode* parser_parse(Parser* parser);

#endif // PARSER_H

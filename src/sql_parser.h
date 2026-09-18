#ifndef SQL_PARSER_H
#define SQL_PARSER_H

#include "sql_lexer.h"
#include "sql_ast.h"

typedef struct {
    Lexer lexer;
    Token current;
    bool has_error;
    char error_msg[256];
    int error_line;
    int error_col;
} Parser;

void parser_init(Parser* parser, const char* source);
bool parser_parse(Parser* parser, ASTQuery* query);

#endif // SQL_PARSER_H

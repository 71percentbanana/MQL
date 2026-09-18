#ifndef SQL_LEXER_H
#define SQL_LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    TOK_EOF = 0,
    TOK_ERROR,

    // Keywords
    TOK_SELECT,
    TOK_FROM,
    TOK_WHERE,
    TOK_ORDER,
    TOK_BY,
    TOK_ASC,
    TOK_DESC,
    TOK_LIMIT,
    TOK_AND,
    TOK_OR,
    TOK_NOT,

    // Identifiers & Literals
    TOK_IDENT,
    TOK_NUMBER,
    TOK_STRING,

    // Operators & Delimiters
    TOK_COMMA,       // ,
    TOK_SEMICOLON,   // ;
    TOK_STAR,        // *
    TOK_LPAREN,      // (
    TOK_RPAREN,      // )
    TOK_EQ,          // =
    TOK_NEQ,         // != or <>
    TOK_GT,          // >
    TOK_GTE,         // >=
    TOK_LT,          // <
    TOK_LTE          // <=
} TokenType;

typedef struct {
    TokenType type;
    char lexeme[128];
    double num_val;
    int line;
    int col;
} Token;

typedef struct {
    const char* src;
    size_t pos;
    size_t len;
    int line;
    int col;
} Lexer;

void lexer_init(Lexer* lexer, const char* source);
bool lexer_next_token(Lexer* lexer, Token* token);
const char* token_type_name(TokenType type);
void lexer_print_all(const char* source);

#endif // SQL_LEXER_H

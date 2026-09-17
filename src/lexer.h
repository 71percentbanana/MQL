#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_LEXEME_LEN 64

// All supported token types in TrafficRule
typedef enum {
    TOKEN_EOF,
    TOKEN_ERROR,

    // Keywords
    TOKEN_KW_VEHICLE,
    TOKEN_KW_IF,
    TOKEN_KW_ELSE,
    TOKEN_KW_AND,
    TOKEN_KW_OR,

    // Actions
    TOKEN_ACT_ISSUE_FINE,
    TOKEN_ACT_FLAG_VIOLATION,
    TOKEN_ACT_ALLOW,

    // Traffic States
    TOKEN_STATE_RED,
    TOKEN_STATE_GREEN,
    TOKEN_STATE_YELLOW,
    TOKEN_STATE_CROSSING,
    TOKEN_STATE_ROAD,

    // Identifiers and numbers
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,

    // Operators
    TOKEN_ASSIGN,       // =
    TOKEN_OP_GT,        // >
    TOKEN_OP_LT,        // <
    TOKEN_OP_GTE,       // >=
    TOKEN_OP_LTE,       // <=
    TOKEN_OP_EQ,        // ==
    TOKEN_OP_NEQ,       // !=

    // Delimiters
    TOKEN_LBRACE,       // {
    TOKEN_RBRACE        // }
} TokenType;

// Token structure
typedef struct {
    TokenType type;
    char lexeme[MAX_LEXEME_LEN];
    int int_val;
    int line;
    int col;
} Token;

// Lexer scanner structure
typedef struct {
    const char* source;
    size_t length;
    size_t cursor;
    int line;
    int col;

    bool has_error;
    char error_msg[256];
    int error_line;
    int error_col;
} Lexer;

// Function signatures
const char* token_type_to_str(TokenType type);
void lexer_init(Lexer* lexer, const char* source);
Token lexer_next_token(Lexer* lexer);
Token lexer_peek_token(Lexer* lexer);
int lexer_tokenize_all(const char* source, Token** out_tokens, int* out_count, char* err_buf, size_t err_buf_size);

#endif // LEXER_H

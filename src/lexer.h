#ifndef LEXER_H
#define LEXER_H

#include "common.h"

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

void lexer_init(Lexer* lexer, const char* source);
Token lexer_next_token(Lexer* lexer);
Token lexer_peek_token(Lexer* lexer);

// Helper to tokenize an entire source code string into an array of tokens
int lexer_tokenize_all(const char* source, Token** out_tokens, int* out_count, char* err_buf, size_t err_buf_size);

#endif // LEXER_H

#include "lexer.h"

const char* token_type_to_str(TokenType type) {
    switch (type) {
        case TOKEN_EOF: return "EOF";
        case TOKEN_ERROR: return "ERROR";
        case TOKEN_KW_VEHICLE: return "KEYWORD(VEHICLE)";
        case TOKEN_KW_IF: return "KEYWORD(IF)";
        case TOKEN_KW_ELSE: return "KEYWORD(ELSE)";
        case TOKEN_KW_AND: return "KEYWORD(AND)";
        case TOKEN_KW_OR: return "KEYWORD(OR)";
        case TOKEN_ACT_ISSUE_FINE: return "ACTION(ISSUE_FINE)";
        case TOKEN_ACT_FLAG_VIOLATION: return "ACTION(FLAG_VIOLATION)";
        case TOKEN_ACT_ALLOW: return "ACTION(ALLOW)";
        case TOKEN_STATE_RED: return "STATE(RED)";
        case TOKEN_STATE_GREEN: return "STATE(GREEN)";
        case TOKEN_STATE_YELLOW: return "STATE(YELLOW)";
        case TOKEN_STATE_CROSSING: return "STATE(CROSSING)";
        case TOKEN_STATE_ROAD: return "STATE(ROAD)";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_ASSIGN: return "ASSIGN(=)";
        case TOKEN_OP_GT: return "OPERATOR(>)";
        case TOKEN_OP_LT: return "OPERATOR(<)";
        case TOKEN_OP_GTE: return "OPERATOR(>=)";
        case TOKEN_OP_LTE: return "OPERATOR(<=)";
        case TOKEN_OP_EQ: return "OPERATOR(==)";
        case TOKEN_OP_NEQ: return "OPERATOR(!=)";
        case TOKEN_LBRACE: return "DELIMITER({)";
        case TOKEN_RBRACE: return "DELIMITER(})";
        default: return "UNKNOWN";
    }
}

void lexer_init(Lexer* lexer, const char* source) {
    lexer->source = source;
    lexer->length = source ? strlen(source) : 0;
    lexer->cursor = 0;
    lexer->line = 1;
    lexer->col = 1;
    lexer->has_error = false;
    lexer->error_msg[0] = '\0';
    lexer->error_line = 0;
    lexer->error_col = 0;
}

static char peek_char(Lexer* lexer) {
    if (lexer->cursor >= lexer->length) return '\0';
    return lexer->source[lexer->cursor];
}

static char advance_char(Lexer* lexer) {
    if (lexer->cursor >= lexer->length) return '\0';
    char c = lexer->source[lexer->cursor++];
    if (c == '\n') {
        lexer->line++;
        lexer->col = 1;
    } else {
        lexer->col++;
    }
    return c;
}

static void skip_whitespace_and_comments(Lexer* lexer) {
    while (lexer->cursor < lexer->length) {
        char c = peek_char(lexer);
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance_char(lexer);
        } else if (c == '#') {
            while (lexer->cursor < lexer->length && peek_char(lexer) != '\n') {
                advance_char(lexer);
            }
            if (peek_char(lexer) == '\n') {
                advance_char(lexer);
            }
        } else {
            break;
        }
    }
}

Token lexer_next_token(Lexer* lexer) {
    skip_whitespace_and_comments(lexer);

    Token token;
    memset(&token, 0, sizeof(Token));
    token.line = lexer->line;
    token.col = lexer->col;

    if (lexer->cursor >= lexer->length) {
        token.type = TOKEN_EOF;
        strcpy(token.lexeme, "EOF");
        return token;
    }

    char c = peek_char(lexer);

    // Identifiers, Keywords, States, Actions
    if (isalpha((unsigned char)c) || c == '_') {
        int start_col = lexer->col;
        int start_line = lexer->line;
        size_t start_idx = lexer->cursor;

        while (isalnum((unsigned char)peek_char(lexer)) || peek_char(lexer) == '_') {
            advance_char(lexer);
        }

        size_t len = lexer->cursor - start_idx;
        if (len >= MAX_LEXEME_LEN) len = MAX_LEXEME_LEN - 1;
        strncpy(token.lexeme, &lexer->source[start_idx], len);
        token.lexeme[len] = '\0';
        token.line = start_line;
        token.col = start_col;

        if (strcmp(token.lexeme, "VEHICLE") == 0) token.type = TOKEN_KW_VEHICLE;
        else if (strcmp(token.lexeme, "IF") == 0) token.type = TOKEN_KW_IF;
        else if (strcmp(token.lexeme, "ELSE") == 0) token.type = TOKEN_KW_ELSE;
        else if (strcmp(token.lexeme, "AND") == 0) token.type = TOKEN_KW_AND;
        else if (strcmp(token.lexeme, "OR") == 0) token.type = TOKEN_KW_OR;
        else if (strcmp(token.lexeme, "ISSUE_FINE") == 0) token.type = TOKEN_ACT_ISSUE_FINE;
        else if (strcmp(token.lexeme, "FLAG_VIOLATION") == 0) token.type = TOKEN_ACT_FLAG_VIOLATION;
        else if (strcmp(token.lexeme, "ALLOW") == 0) token.type = TOKEN_ACT_ALLOW;
        else if (strcmp(token.lexeme, "RED") == 0) token.type = TOKEN_STATE_RED;
        else if (strcmp(token.lexeme, "GREEN") == 0) token.type = TOKEN_STATE_GREEN;
        else if (strcmp(token.lexeme, "YELLOW") == 0) token.type = TOKEN_STATE_YELLOW;
        else if (strcmp(token.lexeme, "CROSSING") == 0) token.type = TOKEN_STATE_CROSSING;
        else if (strcmp(token.lexeme, "ROAD") == 0) token.type = TOKEN_STATE_ROAD;
        else token.type = TOKEN_IDENTIFIER;

        return token;
    }

    // Numbers
    if (isdigit((unsigned char)c)) {
        int start_col = lexer->col;
        int start_line = lexer->line;
        size_t start_idx = lexer->cursor;

        while (isdigit((unsigned char)peek_char(lexer))) {
            advance_char(lexer);
        }

        size_t len = lexer->cursor - start_idx;
        if (len >= MAX_LEXEME_LEN) len = MAX_LEXEME_LEN - 1;
        strncpy(token.lexeme, &lexer->source[start_idx], len);
        token.lexeme[len] = '\0';
        token.line = start_line;
        token.col = start_col;
        token.type = TOKEN_NUMBER;
        token.int_val = atoi(token.lexeme);
        return token;
    }

    // Operators and delimiters
    switch (c) {
        case '=':
            advance_char(lexer);
            if (peek_char(lexer) == '=') {
                advance_char(lexer);
                token.type = TOKEN_OP_EQ;
                strcpy(token.lexeme, "==");
            } else {
                token.type = TOKEN_ASSIGN;
                strcpy(token.lexeme, "=");
            }
            return token;

        case '!':
            advance_char(lexer);
            if (peek_char(lexer) == '=') {
                advance_char(lexer);
                token.type = TOKEN_OP_NEQ;
                strcpy(token.lexeme, "!=");
                return token;
            } else {
                token.type = TOKEN_ERROR;
                snprintf(token.lexeme, MAX_LEXEME_LEN, "!");
                lexer->has_error = true;
                lexer->error_line = token.line;
                lexer->error_col = token.col;
                snprintf(lexer->error_msg, sizeof(lexer->error_msg),
                         "Unexpected character '!' (did you mean '!= '?) at line %d, col %d",
                         token.line, token.col);
                return token;
            }

        case '>':
            advance_char(lexer);
            if (peek_char(lexer) == '=') {
                advance_char(lexer);
                token.type = TOKEN_OP_GTE;
                strcpy(token.lexeme, ">=");
            } else {
                token.type = TOKEN_OP_GT;
                strcpy(token.lexeme, ">");
            }
            return token;

        case '<':
            advance_char(lexer);
            if (peek_char(lexer) == '=') {
                advance_char(lexer);
                token.type = TOKEN_OP_LTE;
                strcpy(token.lexeme, "<=");
            } else {
                token.type = TOKEN_OP_LT;
                strcpy(token.lexeme, "<");
            }
            return token;

        case '{':
            advance_char(lexer);
            token.type = TOKEN_LBRACE;
            strcpy(token.lexeme, "{");
            return token;

        case '}':
            advance_char(lexer);
            token.type = TOKEN_RBRACE;
            strcpy(token.lexeme, "}");
            return token;

        default: {
            char invalid_ch = advance_char(lexer);
            token.type = TOKEN_ERROR;
            snprintf(token.lexeme, MAX_LEXEME_LEN, "%c", invalid_ch);
            lexer->has_error = true;
            lexer->error_line = token.line;
            lexer->error_col = token.col;
            snprintf(lexer->error_msg, sizeof(lexer->error_msg),
                     "Lexical Error: Unrecognized character '%c' (ASCII %d) at line %d, col %d",
                     invalid_ch, (int)invalid_ch, token.line, token.col);
            return token;
        }
    }
}

Token lexer_peek_token(Lexer* lexer) {
    Lexer save = *lexer;
    Token t = lexer_next_token(lexer);
    *lexer = save;
    return t;
}

int lexer_tokenize_all(const char* source, Token** out_tokens, int* out_count, char* err_buf, size_t err_buf_size) {
    Lexer lexer;
    lexer_init(&lexer, source);

    int capacity = 32;
    int count = 0;
    Token* tokens = (Token*)malloc(sizeof(Token) * capacity);

    while (true) {
        Token t = lexer_next_token(&lexer);
        if (t.type == TOKEN_ERROR) {
            if (err_buf) {
                strncpy(err_buf, lexer.error_msg, err_buf_size - 1);
                err_buf[err_buf_size - 1] = '\0';
            }
            free(tokens);
            *out_tokens = NULL;
            *out_count = 0;
            return -1;
        }

        if (count >= capacity) {
            capacity *= 2;
            tokens = (Token*)realloc(tokens, sizeof(Token) * capacity);
        }
        tokens[count++] = t;

        if (t.type == TOKEN_EOF) break;
    }

    *out_tokens = tokens;
    *out_count = count;
    return 0;
}

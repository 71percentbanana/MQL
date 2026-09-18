#include "sql_lexer.h"

void lexer_init(Lexer* lexer, const char* source) {
    lexer->src = source ? source : "";
    lexer->pos = 0;
    lexer->len = strlen(lexer->src);
    lexer->line = 1;
    lexer->col = 1;
}

static char peek_char(const Lexer* lexer) {
    if (lexer->pos >= lexer->len) return '\0';
    return lexer->src[lexer->pos];
}

static char peek_next(const Lexer* lexer) {
    if (lexer->pos + 1 >= lexer->len) return '\0';
    return lexer->src[lexer->pos + 1];
}

static char advance_char(Lexer* lexer) {
    if (lexer->pos >= lexer->len) return '\0';
    char c = lexer->src[lexer->pos++];
    if (c == '\n') {
        lexer->line++;
        lexer->col = 1;
    } else {
        lexer->col++;
    }
    return c;
}

static void skip_whitespace_and_comments(Lexer* lexer) {
    while (lexer->pos < lexer->len) {
        char c = peek_char(lexer);
        if (isspace((unsigned char)c)) {
            advance_char(lexer);
        } else if (c == '-' && peek_next(lexer) == '-') {
            // SQL single-line comment: -- ...
            advance_char(lexer);
            advance_char(lexer);
            while (lexer->pos < lexer->len && peek_char(lexer) != '\n') {
                advance_char(lexer);
            }
        } else if (c == '/' && peek_next(lexer) == '*') {
            // Multi-line comment: /* ... */
            advance_char(lexer);
            advance_char(lexer);
            while (lexer->pos < lexer->len) {
                if (peek_char(lexer) == '*' && peek_next(lexer) == '/') {
                    advance_char(lexer);
                    advance_char(lexer);
                    break;
                }
                advance_char(lexer);
            }
        } else {
            break;
        }
    }
}

static TokenType check_keyword(const char* word) {
    // Case-insensitive SQL keyword check
    char upper[64];
    size_t i = 0;
    for (; word[i] && i < sizeof(upper) - 1; i++) {
        upper[i] = (char)toupper((unsigned char)word[i]);
    }
    upper[i] = '\0';

    if (strcmp(upper, "SELECT") == 0) return TOK_SELECT;
    if (strcmp(upper, "FROM") == 0)   return TOK_FROM;
    if (strcmp(upper, "WHERE") == 0)  return TOK_WHERE;
    if (strcmp(upper, "ORDER") == 0)  return TOK_ORDER;
    if (strcmp(upper, "BY") == 0)     return TOK_BY;
    if (strcmp(upper, "ASC") == 0)    return TOK_ASC;
    if (strcmp(upper, "DESC") == 0)   return TOK_DESC;
    if (strcmp(upper, "LIMIT") == 0)  return TOK_LIMIT;
    if (strcmp(upper, "AND") == 0)    return TOK_AND;
    if (strcmp(upper, "OR") == 0)     return TOK_OR;
    if (strcmp(upper, "NOT") == 0)    return TOK_NOT;

    return TOK_IDENT;
}

bool lexer_next_token(Lexer* lexer, Token* token) {
    skip_whitespace_and_comments(lexer);

    token->line = lexer->line;
    token->col = lexer->col;
    token->num_val = 0.0;
    token->lexeme[0] = '\0';

    if (lexer->pos >= lexer->len) {
        token->type = TOK_EOF;
        strcpy(token->lexeme, "EOF");
        return true;
    }

    char c = peek_char(lexer);

    // Single-character symbols
    if (c == ',') {
        token->type = TOK_COMMA;
        token->lexeme[0] = advance_char(lexer);
        token->lexeme[1] = '\0';
        return true;
    }
    if (c == ';') {
        token->type = TOK_SEMICOLON;
        token->lexeme[0] = advance_char(lexer);
        token->lexeme[1] = '\0';
        return true;
    }
    if (c == '*') {
        token->type = TOK_STAR;
        token->lexeme[0] = advance_char(lexer);
        token->lexeme[1] = '\0';
        return true;
    }
    if (c == '(') {
        token->type = TOK_LPAREN;
        token->lexeme[0] = advance_char(lexer);
        token->lexeme[1] = '\0';
        return true;
    }
    if (c == ')') {
        token->type = TOK_RPAREN;
        token->lexeme[0] = advance_char(lexer);
        token->lexeme[1] = '\0';
        return true;
    }

    // Comparison Operators
    if (c == '=') {
        token->type = TOK_EQ;
        token->lexeme[0] = advance_char(lexer);
        token->lexeme[1] = '\0';
        return true;
    }
    if (c == '!') {
        advance_char(lexer);
        if (peek_char(lexer) == '=') {
            advance_char(lexer);
            token->type = TOK_NEQ;
            strcpy(token->lexeme, "!=");
            return true;
        }
        token->type = TOK_ERROR;
        strcpy(token->lexeme, "!");
        return false;
    }
    if (c == '<') {
        advance_char(lexer);
        if (peek_char(lexer) == '=') {
            advance_char(lexer);
            token->type = TOK_LTE;
            strcpy(token->lexeme, "<=");
        } else if (peek_char(lexer) == '>') {
            advance_char(lexer);
            token->type = TOK_NEQ;
            strcpy(token->lexeme, "<>");
        } else {
            token->type = TOK_LT;
            strcpy(token->lexeme, "<");
        }
        return true;
    }
    if (c == '>') {
        advance_char(lexer);
        if (peek_char(lexer) == '=') {
            advance_char(lexer);
            token->type = TOK_GTE;
            strcpy(token->lexeme, ">=");
        } else {
            token->type = TOK_GT;
            strcpy(token->lexeme, ">");
        }
        return true;
    }

    // String literals ('string' or "string")
    if (c == '\'' || c == '"') {
        char quote = advance_char(lexer);
        size_t idx = 0;
        while (lexer->pos < lexer->len && peek_char(lexer) != quote) {
            char ch = advance_char(lexer);
            if (ch == '\\' && lexer->pos < lexer->len) {
                ch = advance_char(lexer);
            }
            if (idx < sizeof(token->lexeme) - 1) {
                token->lexeme[idx++] = ch;
            }
        }
        token->lexeme[idx] = '\0';
        if (peek_char(lexer) == quote) {
            advance_char(lexer); // consume closing quote
            token->type = TOK_STRING;
            return true;
        } else {
            token->type = TOK_ERROR;
            return false;
        }
    }

    // Number literals (integers and floats)
    if (isdigit((unsigned char)c)) {
        size_t idx = 0;
        bool has_dot = false;
        while (lexer->pos < lexer->len && (isdigit((unsigned char)peek_char(lexer)) || (!has_dot && peek_char(lexer) == '.'))) {
            char ch = advance_char(lexer);
            if (ch == '.') has_dot = true;
            if (idx < sizeof(token->lexeme) - 1) {
                token->lexeme[idx++] = ch;
            }
        }
        token->lexeme[idx] = '\0';
        token->type = TOK_NUMBER;
        token->num_val = atof(token->lexeme);
        return true;
    }

    // Identifiers or keywords
    if (isalpha((unsigned char)c) || c == '_') {
        size_t idx = 0;
        while (lexer->pos < lexer->len && (isalnum((unsigned char)peek_char(lexer)) || peek_char(lexer) == '_')) {
            char ch = advance_char(lexer);
            if (idx < sizeof(token->lexeme) - 1) {
                token->lexeme[idx++] = ch;
            }
        }
        token->lexeme[idx] = '\0';
        token->type = check_keyword(token->lexeme);
        return true;
    }

    // Unrecognized character
    token->type = TOK_ERROR;
    token->lexeme[0] = advance_char(lexer);
    token->lexeme[1] = '\0';
    return false;
}

const char* token_type_name(TokenType type) {
    switch (type) {
        case TOK_EOF:       return "EOF";
        case TOK_ERROR:     return "LEXICAL_ERROR";
        case TOK_SELECT:    return "KEYWORD(SELECT)";
        case TOK_FROM:      return "KEYWORD(FROM)";
        case TOK_WHERE:     return "KEYWORD(WHERE)";
        case TOK_ORDER:     return "KEYWORD(ORDER)";
        case TOK_BY:        return "KEYWORD(BY)";
        case TOK_ASC:       return "KEYWORD(ASC)";
        case TOK_DESC:      return "KEYWORD(DESC)";
        case TOK_LIMIT:     return "KEYWORD(LIMIT)";
        case TOK_AND:       return "KEYWORD(AND)";
        case TOK_OR:        return "KEYWORD(OR)";
        case TOK_NOT:       return "KEYWORD(NOT)";
        case TOK_IDENT:     return "IDENTIFIER";
        case TOK_NUMBER:    return "NUMBER";
        case TOK_STRING:    return "STRING";
        case TOK_COMMA:     return "COMMA(,)";
        case TOK_SEMICOLON: return "SEMICOLON(;)";
        case TOK_STAR:      return "STAR(*)";
        case TOK_LPAREN:    return "LPAREN(()";
        case TOK_RPAREN:    return "RPAREN())";
        case TOK_EQ:        return "OPERATOR(=)";
        case TOK_NEQ:       return "OPERATOR(!=)";
        case TOK_GT:        return "OPERATOR(>)";
        case TOK_GTE:       return "OPERATOR(>=)";
        case TOK_LT:        return "OPERATOR(<)";
        case TOK_LTE:       return "OPERATOR(<=)";
        default:            return "UNKNOWN";
    }
}

void lexer_print_all(const char* source) {
    Lexer lexer;
    lexer_init(&lexer, source);
    Token tok;

    printf("+----+-----------------------+---------------------+------+------+\n");
    printf("| #  | Token Type            | Lexeme              | Line | Col  |\n");
    printf("+----+-----------------------+---------------------+------+------+\n");

    int count = 0;
    int errors = 0;
    while (lexer_next_token(&lexer, &tok)) {
        if (tok.type == TOK_EOF) break;
        count++;
        printf("| %-2d | %-21s | %-19s | %-4d | %-4d |\n",
               count, token_type_name(tok.type), tok.lexeme, tok.line, tok.col);
    }
    if (tok.type == TOK_ERROR) {
        count++;
        errors++;
        printf("| %-2d | %-21s | %-19s | %-4d | %-4d |\n",
               count, "LEXICAL_ERROR", tok.lexeme, tok.line, tok.col);
    }

    printf("+----+-----------------------+---------------------+------+------+\n");
    printf("Total Tokens Scanned: %d | Lexical Errors: %d | Status: %s\n",
           count, errors, errors == 0 ? "PASS" : "FAIL");
}

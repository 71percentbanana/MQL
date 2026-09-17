#include "parser.h"
#include <stdarg.h>

static void advance(Parser* parser) {
    parser->current = parser->peek;
    parser->peek = lexer_next_token(&parser->lexer);
    if (parser->peek.type == TOKEN_ERROR && !parser->has_error) {
        parser->has_error = true;
        parser->error_line = parser->peek.line;
        parser->error_col = parser->peek.col;
        strncpy(parser->error_msg, parser->lexer.error_msg, sizeof(parser->error_msg) - 1);
    }
}

static void set_error(Parser* parser, int line, int col, const char* fmt, ...) {
    if (parser->has_error) return; // keep first error
    parser->has_error = true;
    parser->error_line = line;
    parser->error_col = col;

    va_list args;
    va_start(args, fmt);
    vsnprintf(parser->error_msg, sizeof(parser->error_msg), fmt, args);
    va_end(args);
}

void parser_init(Parser* parser, const char* source) {
    lexer_init(&parser->lexer, source);
    parser->has_error = false;
    parser->error_msg[0] = '\0';
    parser->error_line = 0;
    parser->error_col = 0;

    // Prime the tokens
    parser->current = lexer_next_token(&parser->lexer);
    if (parser->current.type == TOKEN_ERROR) {
        set_error(parser, parser->current.line, parser->current.col, "%s", parser->lexer.error_msg);
    }
    parser->peek = lexer_next_token(&parser->lexer);
    if (parser->peek.type == TOKEN_ERROR && !parser->has_error) {
        set_error(parser, parser->peek.line, parser->peek.col, "%s", parser->lexer.error_msg);
    }
}

static bool match(Parser* parser, TokenType type) {
    return parser->current.type == type;
}

static bool consume(Parser* parser, TokenType type, const char* err_msg) {
    if (match(parser, type)) {
        advance(parser);
        return true;
    }
    set_error(parser, parser->current.line, parser->current.col,
              "Syntax Error: %s (got '%s' instead) at line %d, col %d",
              err_msg, parser->current.lexeme, parser->current.line, parser->current.col);
    return false;
}

// Forward declarations
static ASTNode* parse_statement(Parser* parser);
static ASTNode* parse_condition(Parser* parser);
static ASTNode* parse_block(Parser* parser);
static ASTNode* parse_value(Parser* parser);

static ASTNode* parse_value(Parser* parser) {
    if (parser->has_error) return NULL;

    Token t = parser->current;
    if (t.type == TOKEN_IDENTIFIER) {
        advance(parser);
        return ast_create_ident(t.lexeme, t.line, t.col);
    } else if (t.type == TOKEN_NUMBER) {
        advance(parser);
        return ast_create_lit_int(t.int_val, t.line, t.col);
    } else if (t.type == TOKEN_STATE_RED) {
        advance(parser);
        return ast_create_lit_state(STATE_RED, t.line, t.col);
    } else if (t.type == TOKEN_STATE_GREEN) {
        advance(parser);
        return ast_create_lit_state(STATE_GREEN, t.line, t.col);
    } else if (t.type == TOKEN_STATE_YELLOW) {
        advance(parser);
        return ast_create_lit_state(STATE_YELLOW, t.line, t.col);
    } else if (t.type == TOKEN_STATE_CROSSING) {
        advance(parser);
        return ast_create_lit_state(STATE_CROSSING, t.line, t.col);
    } else if (t.type == TOKEN_STATE_ROAD) {
        advance(parser);
        return ast_create_lit_state(STATE_ROAD, t.line, t.col);
    } else {
        set_error(parser, t.line, t.col,
                  "Syntax Error: Expected identifier, number, or traffic state but found '%s' at line %d, col %d",
                  t.lexeme, t.line, t.col);
        return NULL;
    }
}

static ASTNode* parse_rel_expr(Parser* parser) {
    if (parser->has_error) return NULL;

    int line = parser->current.line;
    int col = parser->current.col;

    ASTNode* left = parse_value(parser);
    if (!left) return NULL;

    OperatorType op = OP_NONE;
    if (match(parser, TOKEN_OP_GT)) op = OP_GT;
    else if (match(parser, TOKEN_OP_LT)) op = OP_LT;
    else if (match(parser, TOKEN_OP_GTE)) op = OP_GTE;
    else if (match(parser, TOKEN_OP_LTE)) op = OP_LTE;
    else if (match(parser, TOKEN_OP_EQ)) op = OP_EQ;
    else if (match(parser, TOKEN_OP_NEQ)) op = OP_NEQ;
    else {
        set_error(parser, parser->current.line, parser->current.col,
                  "Syntax Error: Expected comparison operator (>, <, >=, <=, ==, !=) but found '%s' at line %d, col %d",
                  parser->current.lexeme, parser->current.line, parser->current.col);
        ast_free(left);
        return NULL;
    }
    advance(parser);

    ASTNode* right = parse_value(parser);
    if (!right) {
        ast_free(left);
        return NULL;
    }

    return ast_create_binary_op(op, left, right, line, col);
}

static ASTNode* parse_and_expr(Parser* parser) {
    if (parser->has_error) return NULL;

    ASTNode* left = parse_rel_expr(parser);
    if (!left) return NULL;

    while (match(parser, TOKEN_KW_AND)) {
        int line = parser->current.line;
        int col = parser->current.col;
        advance(parser);
        ASTNode* right = parse_rel_expr(parser);
        if (!right) {
            ast_free(left);
            return NULL;
        }
        left = ast_create_binary_op(OP_AND, left, right, line, col);
    }
    return left;
}

static ASTNode* parse_condition(Parser* parser) {
    if (parser->has_error) return NULL;

    ASTNode* left = parse_and_expr(parser);
    if (!left) return NULL;

    while (match(parser, TOKEN_KW_OR)) {
        int line = parser->current.line;
        int col = parser->current.col;
        advance(parser);
        ASTNode* right = parse_and_expr(parser);
        if (!right) {
            ast_free(left);
            return NULL;
        }
        left = ast_create_binary_op(OP_OR, left, right, line, col);
    }
    return left;
}

static ASTNode* parse_declaration(Parser* parser) {
    int line = parser->current.line;
    int col = parser->current.col;
    advance(parser); // consume 'VEHICLE'

    if (!match(parser, TOKEN_IDENTIFIER)) {
        set_error(parser, parser->current.line, parser->current.col,
                  "Syntax Error: Expected vehicle identifier after 'VEHICLE' but found '%s' at line %d, col %d",
                  parser->current.lexeme, parser->current.line, parser->current.col);
        return NULL;
    }

    char name[MAX_IDENT_LEN];
    strncpy(name, parser->current.lexeme, MAX_IDENT_LEN - 1);
    name[MAX_IDENT_LEN - 1] = '\0';
    advance(parser);

    return ast_create_var_decl(name, line, col);
}

static ASTNode* parse_assignment(Parser* parser) {
    int line = parser->current.line;
    int col = parser->current.col;

    char name[MAX_IDENT_LEN];
    strncpy(name, parser->current.lexeme, MAX_IDENT_LEN - 1);
    name[MAX_IDENT_LEN - 1] = '\0';
    advance(parser); // consume identifier

    if (!consume(parser, TOKEN_ASSIGN, "Expected '=' in assignment")) {
        return NULL;
    }

    ASTNode* val = parse_value(parser);
    if (!val) return NULL;

    return ast_create_assign(name, val, line, col);
}

static ASTNode* parse_action(Parser* parser) {
    int line = parser->current.line;
    int col = parser->current.col;
    ActionValue act = ACTION_NONE;

    if (match(parser, TOKEN_ACT_ISSUE_FINE)) act = ACTION_ISSUE_FINE;
    else if (match(parser, TOKEN_ACT_FLAG_VIOLATION)) act = ACTION_FLAG_VIOLATION;
    else if (match(parser, TOKEN_ACT_ALLOW)) act = ACTION_ALLOW;
    else {
        set_error(parser, line, col, "Syntax Error: Unknown action '%s'", parser->current.lexeme);
        return NULL;
    }
    advance(parser);
    return ast_create_action(act, line, col);
}

static ASTNode* parse_block(Parser* parser) {
    int line = parser->current.line;
    int col = parser->current.col;

    if (!consume(parser, TOKEN_LBRACE, "Expected '{' to start block")) {
        return NULL;
    }

    ASTNode* block = ast_create_program();
    block->line = line;
    block->col = col;

    while (!match(parser, TOKEN_RBRACE) && !match(parser, TOKEN_EOF) && !parser->has_error) {
        ASTNode* stmt = parse_statement(parser);
        if (stmt) {
            ast_program_add_stmt(block, stmt);
        } else {
            ast_free(block);
            return NULL;
        }
    }

    if (!consume(parser, TOKEN_RBRACE, "Expected '}' to close block")) {
        ast_free(block);
        return NULL;
    }

    return block;
}

static ASTNode* parse_if_stmt(Parser* parser) {
    int line = parser->current.line;
    int col = parser->current.col;
    advance(parser); // consume 'IF'

    ASTNode* cond = parse_condition(parser);
    if (!cond) return NULL;

    ASTNode* then_blk = parse_block(parser);
    if (!then_blk) {
        ast_free(cond);
        return NULL;
    }

    ASTNode* else_blk = NULL;
    if (match(parser, TOKEN_KW_ELSE)) {
        advance(parser); // consume 'ELSE'
        else_blk = parse_block(parser);
        if (!else_blk) {
            ast_free(cond);
            ast_free(then_blk);
            return NULL;
        }
    }

    return ast_create_if(cond, then_blk, else_blk, line, col);
}

static ASTNode* parse_statement(Parser* parser) {
    if (parser->has_error) return NULL;

    if (match(parser, TOKEN_KW_VEHICLE)) {
        return parse_declaration(parser);
    } else if (match(parser, TOKEN_KW_IF)) {
        return parse_if_stmt(parser);
    } else if (match(parser, TOKEN_ACT_ISSUE_FINE) ||
               match(parser, TOKEN_ACT_FLAG_VIOLATION) ||
               match(parser, TOKEN_ACT_ALLOW)) {
        return parse_action(parser);
    } else if (match(parser, TOKEN_IDENTIFIER)) {
        // Can be assignment: identifier = value
        return parse_assignment(parser);
    } else {
        set_error(parser, parser->current.line, parser->current.col,
                  "Syntax Error: Unexpected token '%s' while parsing statement at line %d, col %d",
                  parser->current.lexeme, parser->current.line, parser->current.col);
        return NULL;
    }
}

ASTNode* parser_parse(Parser* parser) {
    ASTNode* root = ast_create_program();

    while (!match(parser, TOKEN_EOF) && !parser->has_error) {
        ASTNode* stmt = parse_statement(parser);
        if (stmt) {
            ast_program_add_stmt(root, stmt);
        } else {
            ast_free(root);
            return NULL;
        }
    }

    if (parser->has_error) {
        ast_free(root);
        return NULL;
    }

    return root;
}

#include "sql_parser.h"

static void advance(Parser* p) {
    if (!lexer_next_token(&p->lexer, &p->current)) {
        p->has_error = true;
        snprintf(p->error_msg, sizeof(p->error_msg),
                 "Lexical error near '%s'", p->current.lexeme);
        p->error_line = p->current.line;
        p->error_col = p->current.col;
    }
}

static bool check(const Parser* p, TokenType type) {
    return p->current.type == type;
}

static bool match(Parser* p, TokenType type) {
    if (check(p, type)) {
        advance(p);
        return true;
    }
    return false;
}

static void syntax_error(Parser* p, const char* expected) {
    if (p->has_error) return;
    p->has_error = true;
    p->error_line = p->current.line;
    p->error_col = p->current.col;
    snprintf(p->error_msg, sizeof(p->error_msg),
             "Syntax error at [Line %d, Col %d]: Expected %s, but found '%s' (%s)",
             p->current.line, p->current.col, expected, p->current.lexeme,
             token_type_name(p->current.type));
}

void parser_init(Parser* parser, const char* source) {
    lexer_init(&parser->lexer, source);
    parser->has_error = false;
    parser->error_msg[0] = '\0';
    parser->error_line = 0;
    parser->error_col = 0;
    advance(parser); // prime first token
}

// Forward declarations for condition parsing
static ASTFilter* parse_or_expr(Parser* p);
static ASTFilter* parse_and_expr(Parser* p);
static ASTFilter* parse_primary_filter(Parser* p);

static bool parse_select_clause(Parser* p, ASTProjection* proj) {
    if (!match(p, TOK_SELECT)) {
        syntax_error(p, "keyword 'SELECT'");
        return false;
    }

    proj->is_wildcard = false;
    proj->field_count = 0;

    if (match(p, TOK_STAR)) {
        proj->is_wildcard = true;
        return true;
    }

    // Comma-separated list of column identifiers
    while (true) {
        if (!check(p, TOK_IDENT)) {
            syntax_error(p, "column identifier or '*'");
            return false;
        }

        if (proj->field_count < 32) {
            strncpy(proj->fields[proj->field_count], p->current.lexeme, 63);
            proj->fields[proj->field_count][63] = '\0';
            proj->field_count++;
        }
        advance(p);

        if (match(p, TOK_COMMA)) {
            continue;
        }
        break;
    }

    return true;
}

static bool parse_from_clause(Parser* p, char* out_collection, size_t max_len) {
    if (!match(p, TOK_FROM)) {
        syntax_error(p, "keyword 'FROM'");
        return false;
    }

    if (!check(p, TOK_IDENT)) {
        syntax_error(p, "collection/table name identifier");
        return false;
    }

    strncpy(out_collection, p->current.lexeme, max_len - 1);
    out_collection[max_len - 1] = '\0';
    advance(p);
    return true;
}

static ComparisonOp token_to_comp_op(TokenType t) {
    switch (t) {
        case TOK_EQ:  return OP_EQ;
        case TOK_NEQ: return OP_NEQ;
        case TOK_GT:  return OP_GT;
        case TOK_GTE: return OP_GTE;
        case TOK_LT:  return OP_LT;
        case TOK_LTE: return OP_LTE;
        default:      return OP_EQ;
    }
}

static ASTFilter* parse_primary_filter(Parser* p) {
    if (p->has_error) return NULL;

    // Parenthesized sub-expression: ( condition )
    if (match(p, TOK_LPAREN)) {
        ASTFilter* inner = parse_or_expr(p);
        if (!match(p, TOK_RPAREN)) {
            syntax_error(p, "closing parenthesis ')'");
            return NULL;
        }
        return inner;
    }

    // Binary comparison: field OP literal
    if (!check(p, TOK_IDENT)) {
        syntax_error(p, "field identifier in WHERE clause");
        return NULL;
    }
    char field_name[64];
    strncpy(field_name, p->current.lexeme, sizeof(field_name) - 1);
    field_name[sizeof(field_name) - 1] = '\0';
    advance(p);

    if (!check(p, TOK_EQ) && !check(p, TOK_NEQ) &&
        !check(p, TOK_GT) && !check(p, TOK_GTE) &&
        !check(p, TOK_LT) && !check(p, TOK_LTE)) {
        syntax_error(p, "comparison operator (=, !=, >, <, >=, <=)");
        return NULL;
    }
    ComparisonOp op = token_to_comp_op(p->current.type);
    advance(p);

    ASTLiteral lit;
    if (check(p, TOK_NUMBER)) {
        lit.num_val = p->current.num_val;
        if (strchr(p->current.lexeme, '.')) {
            lit.type = LIT_FLOAT;
        } else {
            lit.type = LIT_INT;
        }
        lit.str_val[0] = '\0';
        advance(p);
    } else if (check(p, TOK_STRING)) {
        lit.type = LIT_STRING;
        lit.num_val = 0.0;
        strncpy(lit.str_val, p->current.lexeme, sizeof(lit.str_val) - 1);
        lit.str_val[sizeof(lit.str_val) - 1] = '\0';
        advance(p);
    } else {
        syntax_error(p, "number or string literal for comparison");
        return NULL;
    }

    return ast_create_binary_filter(field_name, op, lit);
}

static ASTFilter* parse_and_expr(Parser* p) {
    ASTFilter* left = parse_primary_filter(p);
    if (!left || p->has_error) return left;

    while (match(p, TOK_AND)) {
        ASTFilter* right = parse_primary_filter(p);
        if (!right || p->has_error) return left;
        left = ast_create_logical_filter(LOGIC_AND, left, right);
    }
    return left;
}

static ASTFilter* parse_or_expr(Parser* p) {
    ASTFilter* left = parse_and_expr(p);
    if (!left || p->has_error) return left;

    while (match(p, TOK_OR)) {
        ASTFilter* right = parse_and_expr(p);
        if (!right || p->has_error) return left;
        left = ast_create_logical_filter(LOGIC_OR, left, right);
    }
    return left;
}

static bool parse_order_by_clause(Parser* p, ASTOrderBy* order_by) {
    order_by->has_order = false;
    order_by->field[0] = '\0';
    order_by->is_desc = false;

    if (match(p, TOK_ORDER)) {
        if (!match(p, TOK_BY)) {
            syntax_error(p, "keyword 'BY' after 'ORDER'");
            return false;
        }
        if (!check(p, TOK_IDENT)) {
            syntax_error(p, "field name identifier to order by");
            return false;
        }
        order_by->has_order = true;
        strncpy(order_by->field, p->current.lexeme, sizeof(order_by->field) - 1);
        order_by->field[sizeof(order_by->field) - 1] = '\0';
        advance(p);

        if (match(p, TOK_DESC)) {
            order_by->is_desc = true;
        } else if (match(p, TOK_ASC)) {
            order_by->is_desc = false;
        }
    }
    return true;
}

static bool parse_limit_clause(Parser* p, ASTLimit* limit) {
    limit->has_limit = false;
    limit->limit_count = 0;

    if (match(p, TOK_LIMIT)) {
        if (!check(p, TOK_NUMBER)) {
            syntax_error(p, "integer value for LIMIT");
            return false;
        }
        limit->has_limit = true;
        limit->limit_count = (int)p->current.num_val;
        advance(p);
    }
    return true;
}

bool parser_parse(Parser* parser, ASTQuery* query) {
    memset(query, 0, sizeof(ASTQuery));

    // 1. SELECT clause
    if (!parse_select_clause(parser, &query->projection)) {
        return false;
    }

    // 2. FROM clause
    if (!parse_from_clause(parser, query->collection, sizeof(query->collection))) {
        return false;
    }

    // 3. Optional WHERE clause
    if (match(parser, TOK_WHERE)) {
        query->where_clause = parse_or_expr(parser);
        if (parser->has_error) return false;
    }

    // 4. Optional ORDER BY clause
    if (!parse_order_by_clause(parser, &query->order_by)) {
        return false;
    }

    // 5. Optional LIMIT clause
    if (!parse_limit_clause(parser, &query->limit)) {
        return false;
    }

    // Optional trailing semicolon
    match(parser, TOK_SEMICOLON);

    // Verify end of file
    if (!check(parser, TOK_EOF)) {
        syntax_error(parser, "end of query (unexpected tokens trailing)");
        return false;
    }

    return !parser->has_error;
}

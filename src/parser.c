#include "parser.h"
#include <stdarg.h>

const char* state_to_str(StateValue state) {
    switch (state) {
        case STATE_RED: return "RED";
        case STATE_GREEN: return "GREEN";
        case STATE_YELLOW: return "YELLOW";
        case STATE_CROSSING: return "CROSSING";
        case STATE_ROAD: return "ROAD";
        default: return "NONE";
    }
}

const char* action_to_str(ActionValue action) {
    switch (action) {
        case ACTION_ISSUE_FINE: return "ISSUE_FINE";
        case ACTION_FLAG_VIOLATION: return "FLAG_VIOLATION";
        case ACTION_ALLOW: return "ALLOW";
        default: return "NONE";
    }
}

const char* operator_to_str(OperatorType op) {
    switch (op) {
        case OP_GT: return ">";
        case OP_LT: return "<";
        case OP_GTE: return ">=";
        case OP_LTE: return "<=";
        case OP_EQ: return "==";
        case OP_NEQ: return "!=";
        case OP_AND: return "AND";
        case OP_OR: return "OR";
        default: return "NONE";
    }
}

// AST constructors
ASTNode* ast_create_program(void) {
    ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));
    node->type = AST_PROGRAM;
    node->program.stmt_capacity = 8;
    node->program.statements = (ASTNode**)malloc(sizeof(ASTNode*) * node->program.stmt_capacity);
    node->program.stmt_count = 0;
    return node;
}

void ast_program_add_stmt(ASTNode* prog, ASTNode* stmt) {
    if (!prog || !stmt) return;
    if (prog->program.stmt_count >= prog->program.stmt_capacity) {
        prog->program.stmt_capacity *= 2;
        prog->program.statements = (ASTNode**)realloc(
            prog->program.statements,
            sizeof(ASTNode*) * prog->program.stmt_capacity
        );
    }
    prog->program.statements[prog->program.stmt_count++] = stmt;
}

ASTNode* ast_create_var_decl(const char* name, int line, int col) {
    ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));
    node->type = AST_VAR_DECL;
    node->line = line;
    node->col = col;
    strncpy(node->var_decl.name, name, MAX_IDENT_LEN - 1);
    return node;
}

ASTNode* ast_create_assign(const char* name, ASTNode* value, int line, int col) {
    ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));
    node->type = AST_ASSIGN;
    node->line = line;
    node->col = col;
    strncpy(node->assign.name, name, MAX_IDENT_LEN - 1);
    node->assign.value = value;
    return node;
}

ASTNode* ast_create_if(ASTNode* cond, ASTNode* then_blk, ASTNode* else_blk, int line, int col) {
    ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));
    node->type = AST_IF_STMT;
    node->line = line;
    node->col = col;
    node->if_stmt.condition = cond;
    node->if_stmt.then_block = then_blk;
    node->if_stmt.else_block = else_blk;
    return node;
}

ASTNode* ast_create_binary_op(OperatorType op, ASTNode* left, ASTNode* right, int line, int col) {
    ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));
    node->type = AST_BINARY_OP;
    node->line = line;
    node->col = col;
    node->binary_op.op = op;
    node->binary_op.left = left;
    node->binary_op.right = right;
    return node;
}

ASTNode* ast_create_action(ActionValue act, int line, int col) {
    ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));
    node->type = AST_ACTION;
    node->line = line;
    node->col = col;
    node->action.action = act;
    return node;
}

ASTNode* ast_create_lit_int(int val, int line, int col) {
    ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));
    node->type = AST_LITERAL_INT;
    node->line = line;
    node->col = col;
    node->lit_int.value = val;
    return node;
}

ASTNode* ast_create_lit_state(StateValue state, int line, int col) {
    ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));
    node->type = AST_LITERAL_STATE;
    node->line = line;
    node->col = col;
    node->lit_state.state = state;
    return node;
}

ASTNode* ast_create_ident(const char* name, int line, int col) {
    ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));
    node->type = AST_IDENTIFIER;
    node->line = line;
    node->col = col;
    strncpy(node->ident.name, name, MAX_IDENT_LEN - 1);
    return node;
}

void ast_print(const ASTNode* node, int indent) {
    if (!node) return;

    for (int i = 0; i < indent; i++) printf("  ");

    switch (node->type) {
        case AST_PROGRAM:
            printf("Program (%d statements)\n", node->program.stmt_count);
            for (int i = 0; i < node->program.stmt_count; i++) {
                ast_print(node->program.statements[i], indent + 1);
            }
            break;

        case AST_VAR_DECL:
            printf("Declaration: VEHICLE %s\n", node->var_decl.name);
            break;

        case AST_ASSIGN:
            printf("Assignment: %s =\n", node->assign.name);
            ast_print(node->assign.value, indent + 1);
            break;

        case AST_IF_STMT:
            printf("IF Statement\n");
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("[Condition]:\n");
            ast_print(node->if_stmt.condition, indent + 2);

            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("[Then Block]:\n");
            ast_print(node->if_stmt.then_block, indent + 2);

            if (node->if_stmt.else_block) {
                for (int i = 0; i < indent + 1; i++) printf("  ");
                printf("[Else Block]:\n");
                ast_print(node->if_stmt.else_block, indent + 2);
            }
            break;

        case AST_BINARY_OP:
            printf("BinaryOp: %s\n", operator_to_str(node->binary_op.op));
            ast_print(node->binary_op.left, indent + 1);
            ast_print(node->binary_op.right, indent + 1);
            break;

        case AST_ACTION:
            printf("Action: %s\n", action_to_str(node->action.action));
            break;

        case AST_LITERAL_INT:
            printf("LiteralInt: %d\n", node->lit_int.value);
            break;

        case AST_LITERAL_STATE:
            printf("LiteralState: %s\n", state_to_str(node->lit_state.state));
            break;

        case AST_IDENTIFIER:
            printf("Identifier: %s\n", node->ident.name);
            break;
    }
}

static int dot_node_counter = 0;

static int ast_to_dot_internal(const ASTNode* node, FILE* f) {
    if (!node) return -1;
    int my_id = ++dot_node_counter;

    switch (node->type) {
        case AST_PROGRAM:
            fprintf(f, "  n%d [label=\"Program\", shape=box, style=filled, fillcolor=\"#E3F2FD\"];\n", my_id);
            for (int i = 0; i < node->program.stmt_count; i++) {
                int child = ast_to_dot_internal(node->program.statements[i], f);
                if (child != -1) fprintf(f, "  n%d -> n%d;\n", my_id, child);
            }
            break;

        case AST_VAR_DECL:
            fprintf(f, "  n%d [label=\"Decl: VEHICLE %s\", shape=ellipse, fillcolor=\"#FFF9C4\", style=filled];\n", my_id, node->var_decl.name);
            break;

        case AST_ASSIGN:
            fprintf(f, "  n%d [label=\"Assign: %s\", shape=box, style=filled, fillcolor=\"#E8F5E9\"];\n", my_id, node->assign.name);
            if (node->assign.value) {
                int child = ast_to_dot_internal(node->assign.value, f);
                if (child != -1) fprintf(f, "  n%d -> n%d;\n", my_id, child);
            }
            break;

        case AST_IF_STMT:
            fprintf(f, "  n%d [label=\"IF\", shape=diamond, style=filled, fillcolor=\"#FFE0B2\"];\n", my_id);
            if (node->if_stmt.condition) {
                int c = ast_to_dot_internal(node->if_stmt.condition, f);
                fprintf(f, "  n%d -> n%d [label=\"cond\"];\n", my_id, c);
            }
            if (node->if_stmt.then_block) {
                int t = ast_to_dot_internal(node->if_stmt.then_block, f);
                fprintf(f, "  n%d -> n%d [label=\"then\"];\n", my_id, t);
            }
            if (node->if_stmt.else_block) {
                int e = ast_to_dot_internal(node->if_stmt.else_block, f);
                fprintf(f, "  n%d -> n%d [label=\"else\"];\n", my_id, e);
            }
            break;

        case AST_BINARY_OP:
            fprintf(f, "  n%d [label=\"Op: %s\", shape=circle, style=filled, fillcolor=\"#F3E5F5\"];\n", my_id, operator_to_str(node->binary_op.op));
            if (node->binary_op.left) {
                int l = ast_to_dot_internal(node->binary_op.left, f);
                fprintf(f, "  n%d -> n%d;\n", my_id, l);
            }
            if (node->binary_op.right) {
                int r = ast_to_dot_internal(node->binary_op.right, f);
                fprintf(f, "  n%d -> n%d;\n", my_id, r);
            }
            break;

        case AST_ACTION:
            fprintf(f, "  n%d [label=\"Action: %s\", shape=record, style=filled, fillcolor=\"#FFCDD2\"];\n", my_id, action_to_str(node->action.action));
            break;

        case AST_LITERAL_INT:
            fprintf(f, "  n%d [label=\"%d\", shape=oval];\n", my_id, node->lit_int.value);
            break;

        case AST_LITERAL_STATE:
            fprintf(f, "  n%d [label=\"%s\", shape=oval, color=blue];\n", my_id, state_to_str(node->lit_state.state));
            break;

        case AST_IDENTIFIER:
            fprintf(f, "  n%d [label=\"id: %s\", shape=note];\n", my_id, node->ident.name);
            break;
    }
    return my_id;
}

void ast_export_dot(const ASTNode* root, const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) return;
    dot_node_counter = 0;
    fprintf(f, "digraph TrafficRuleAST {\n");
    fprintf(f, "  node [fontname=\"Helvetica\", fontsize=11];\n");
    fprintf(f, "  edge [fontname=\"Helvetica\", fontsize=9];\n");
    ast_to_dot_internal(root, f);
    fprintf(f, "}\n");
    fclose(f);
}

void ast_free(ASTNode* node) {
    if (!node) return;
    switch (node->type) {
        case AST_PROGRAM:
            for (int i = 0; i < node->program.stmt_count; i++) {
                ast_free(node->program.statements[i]);
            }
            free(node->program.statements);
            break;
        case AST_ASSIGN:
            ast_free(node->assign.value);
            break;
        case AST_IF_STMT:
            ast_free(node->if_stmt.condition);
            ast_free(node->if_stmt.then_block);
            if (node->if_stmt.else_block) {
                ast_free(node->if_stmt.else_block);
            }
            break;
        case AST_BINARY_OP:
            ast_free(node->binary_op.left);
            ast_free(node->binary_op.right);
            break;
        default:
            break;
    }
    free(node);
}

// Parser implementation
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
    if (parser->has_error) return;
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
    advance(parser);

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
    advance(parser);

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
    advance(parser);

    ASTNode* cond = parse_condition(parser);
    if (!cond) return NULL;

    ASTNode* then_blk = parse_block(parser);
    if (!then_blk) {
        ast_free(cond);
        return NULL;
    }

    ASTNode* else_blk = NULL;
    if (match(parser, TOKEN_KW_ELSE)) {
        advance(parser);
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

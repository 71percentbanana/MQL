#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

#define MAX_IDENT_LEN 64

typedef enum {
    STATE_NONE,
    STATE_RED,
    STATE_GREEN,
    STATE_YELLOW,
    STATE_CROSSING,
    STATE_ROAD
} StateValue;

typedef enum {
    ACTION_NONE,
    ACTION_ISSUE_FINE,
    ACTION_FLAG_VIOLATION,
    ACTION_ALLOW
} ActionValue;

typedef enum {
    OP_NONE,
    OP_GT,
    OP_LT,
    OP_GTE,
    OP_LTE,
    OP_EQ,
    OP_NEQ,
    OP_AND,
    OP_OR
} OperatorType;

typedef enum {
    AST_PROGRAM,
    AST_VAR_DECL,
    AST_ASSIGN,
    AST_IF_STMT,
    AST_BINARY_OP,
    AST_ACTION,
    AST_LITERAL_INT,
    AST_LITERAL_STATE,
    AST_IDENTIFIER
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    int line;
    int col;

    union {
        struct {
            struct ASTNode** statements;
            int stmt_count;
            int stmt_capacity;
        } program;

        struct {
            char name[MAX_IDENT_LEN];
        } var_decl;

        struct {
            char name[MAX_IDENT_LEN];
            struct ASTNode* value;
        } assign;

        struct {
            struct ASTNode* condition;
            struct ASTNode* then_block;
            struct ASTNode* else_block;
        } if_stmt;

        struct {
            OperatorType op;
            struct ASTNode* left;
            struct ASTNode* right;
        } binary_op;

        struct {
            ActionValue action;
        } action;

        struct {
            int value;
        } lit_int;

        struct {
            StateValue state;
        } lit_state;

        struct {
            char name[MAX_IDENT_LEN];
        } ident;
    };
} ASTNode;

typedef struct {
    Lexer lexer;
    Token current;
    Token peek;

    bool has_error;
    char error_msg[512];
    int error_line;
    int error_col;
} Parser;

// AST constructors & helpers
ASTNode* ast_create_program(void);
void ast_program_add_stmt(ASTNode* prog, ASTNode* stmt);
ASTNode* ast_create_var_decl(const char* name, int line, int col);
ASTNode* ast_create_assign(const char* name, ASTNode* value, int line, int col);
ASTNode* ast_create_if(ASTNode* cond, ASTNode* then_blk, ASTNode* else_blk, int line, int col);
ASTNode* ast_create_binary_op(OperatorType op, ASTNode* left, ASTNode* right, int line, int col);
ASTNode* ast_create_action(ActionValue act, int line, int col);
ASTNode* ast_create_lit_int(int val, int line, int col);
ASTNode* ast_create_lit_state(StateValue state, int line, int col);
ASTNode* ast_create_ident(const char* name, int line, int col);

void ast_print(const ASTNode* node, int indent);
void ast_export_dot(const ASTNode* root, const char* filename);
void ast_free(ASTNode* node);

const char* state_to_str(StateValue state);
const char* action_to_str(ActionValue action);
const char* operator_to_str(OperatorType op);

// Parser functions
void parser_init(Parser* parser, const char* source);
ASTNode* parser_parse(Parser* parser);

#endif // PARSER_H

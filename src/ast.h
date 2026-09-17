#ifndef AST_H
#define AST_H

#include "common.h"

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

    // Node-specific payloads:
    union {
        // AST_PROGRAM or block of statements
        struct {
            struct ASTNode** statements;
            int stmt_count;
            int stmt_capacity;
        } program;

        // AST_VAR_DECL: VEHICLE <name>
        struct {
            char name[MAX_IDENT_LEN];
        } var_decl;

        // AST_ASSIGN: <name> = <val_node>
        struct {
            char name[MAX_IDENT_LEN];
            struct ASTNode* value;
        } assign;

        // AST_IF_STMT: IF <cond> <then_block> [ELSE <else_block>]
        struct {
            struct ASTNode* condition;
            struct ASTNode* then_block;
            struct ASTNode* else_block;
        } if_stmt;

        // AST_BINARY_OP: <left> <op> <right>
        struct {
            OperatorType op;
            struct ASTNode* left;
            struct ASTNode* right;
        } binary_op;

        // AST_ACTION: ISSUE_FINE | FLAG_VIOLATION | ALLOW
        struct {
            ActionValue action;
        } action;

        // AST_LITERAL_INT
        struct {
            int value;
        } lit_int;

        // AST_LITERAL_STATE
        struct {
            StateValue state;
        } lit_state;

        // AST_IDENTIFIER
        struct {
            char name[MAX_IDENT_LEN];
        } ident;
    };
} ASTNode;

// Constructor functions
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

// Visualization and memory management
void ast_print(const ASTNode* node, int indent);
void ast_export_dot(const ASTNode* root, const char* filename);
void ast_free(ASTNode* node);

#endif // AST_H

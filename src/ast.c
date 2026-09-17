#include "ast.h"

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
            printf("Action: %s\n", action_value_to_str(node->action.action));
            break;

        case AST_LITERAL_INT:
            printf("LiteralInt: %d\n", node->lit_int.value);
            break;

        case AST_LITERAL_STATE:
            printf("LiteralState: %s\n", state_value_to_str(node->lit_state.state));
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
            fprintf(f, "  n%d [label=\"Action: %s\", shape=record, style=filled, fillcolor=\"#FFCDD2\"];\n", my_id, action_value_to_str(node->action.action));
            break;

        case AST_LITERAL_INT:
            fprintf(f, "  n%d [label=\"%d\", shape=oval];\n", my_id, node->lit_int.value);
            break;

        case AST_LITERAL_STATE:
            fprintf(f, "  n%d [label=\"%s\", shape=oval, color=blue];\n", my_id, state_value_to_str(node->lit_state.state));
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

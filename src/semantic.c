#include "semantic.h"
#include <stdarg.h>

static void sem_error(SemanticAnalyzer* sem, int line, int col, const char* fmt, ...) {
    if (sem->has_error) return;
    sem->has_error = true;
    sem->error_line = line;
    sem->error_col = col;

    va_list args;
    va_start(args, fmt);
    vsnprintf(sem->error_msg, sizeof(sem->error_msg), fmt, args);
    va_end(args);
}

void semantic_init(SemanticAnalyzer* sem, SymbolTable* st) {
    sem->symtab = st;
    sem->has_error = false;
    sem->error_msg[0] = '\0';
    sem->error_line = 0;
    sem->error_col = 0;
}

static DataType infer_type(SemanticAnalyzer* sem, const ASTNode* node) {
    if (!node || sem->has_error) return TYPE_UNKNOWN;

    switch (node->type) {
        case AST_LITERAL_INT:
            return TYPE_INT;

        case AST_LITERAL_STATE:
            return TYPE_STATE;

        case AST_IDENTIFIER: {
            Symbol* s = symbol_table_lookup(sem->symtab, node->ident.name);
            if (!s) {
                sem_error(sem, node->line, node->col,
                          "Semantic Error: Undefined variable '%s' used before declaration or assignment at line %d, col %d",
                          node->ident.name, node->line, node->col);
                return TYPE_UNKNOWN;
            }
            return s->type;
        }

        case AST_BINARY_OP: {
            OperatorType op = node->binary_op.op;
            if (op == OP_AND || op == OP_OR) {
                DataType t_left = infer_type(sem, node->binary_op.left);
                DataType t_right = infer_type(sem, node->binary_op.right);
                if (t_left != TYPE_BOOL || t_right != TYPE_BOOL) {
                    sem_error(sem, node->line, node->col,
                              "Semantic Error: Logical operators AND/OR require boolean operands at line %d, col %d",
                              node->line, node->col);
                    return TYPE_UNKNOWN;
                }
                return TYPE_BOOL;
            }

            // Relational operators
            DataType t_left = infer_type(sem, node->binary_op.left);
            DataType t_right = infer_type(sem, node->binary_op.right);

            if (t_left == TYPE_UNKNOWN || t_right == TYPE_UNKNOWN) {
                return TYPE_UNKNOWN;
            }

            if (t_left == TYPE_VEHICLE || t_right == TYPE_VEHICLE) {
                sem_error(sem, node->line, node->col,
                          "Semantic Error: Cannot perform comparison on vehicle object at line %d, col %d",
                          node->line, node->col);
                return TYPE_UNKNOWN;
            }

            if (t_left != t_right) {
                sem_error(sem, node->line, node->col,
                          "Type Error: Incompatible comparison between '%s' and '%s' at line %d, col %d",
                          data_type_to_str(t_left), data_type_to_str(t_right), node->line, node->col);
                return TYPE_UNKNOWN;
            }

            if (t_left == TYPE_STATE && (op != OP_EQ && op != OP_NEQ)) {
                sem_error(sem, node->line, node->col,
                          "Type Error: Operator '%s' is invalid for traffic states (only '==' and '!=' allowed) at line %d, col %d",
                          operator_to_str(op), node->line, node->col);
                return TYPE_UNKNOWN;
            }

            return TYPE_BOOL;
        }

        default:
            return TYPE_UNKNOWN;
    }
}

static bool check_node(SemanticAnalyzer* sem, const ASTNode* node) {
    if (!node || sem->has_error) return false;

    switch (node->type) {
        case AST_PROGRAM:
            for (int i = 0; i < node->program.stmt_count; i++) {
                if (!check_node(sem, node->program.statements[i])) {
                    return false;
                }
            }
            return true;

        case AST_VAR_DECL: {
            Symbol* existing = symbol_table_lookup(sem->symtab, node->var_decl.name);
            if (existing) {
                sem_error(sem, node->line, node->col,
                          "Semantic Error: Variable '%s' is already defined at line %d",
                          node->var_decl.name, node->line);
                return false;
            }
            symbol_table_insert(sem->symtab, node->var_decl.name, TYPE_VEHICLE, "global", node->line, node->col);
            return true;
        }

        case AST_ASSIGN: {
            Symbol* s = symbol_table_lookup(sem->symtab, node->assign.name);
            if (s && s->type == TYPE_VEHICLE) {
                sem_error(sem, node->line, node->col,
                          "Semantic Error: Cannot assign value to vehicle variable '%s' at line %d, col %d",
                          node->assign.name, node->line, node->col);
                return false;
            }

            // Check value
            ASTNode* val = node->assign.value;
            if (val->type == AST_LITERAL_INT) {
                if (!s) {
                    s = symbol_table_insert(sem->symtab, node->assign.name, TYPE_INT, "global", node->line, node->col);
                }
                symbol_table_set_int(sem->symtab, node->assign.name, val->lit_int.value);
            } else if (val->type == AST_LITERAL_STATE) {
                if (!s) {
                    s = symbol_table_insert(sem->symtab, node->assign.name, TYPE_STATE, "global", node->line, node->col);
                }
                symbol_table_set_state(sem->symtab, node->assign.name, val->lit_state.state);
            } else if (val->type == AST_IDENTIFIER) {
                Symbol* rhs = symbol_table_lookup(sem->symtab, val->ident.name);
                if (!rhs) {
                    sem_error(sem, val->line, val->col,
                              "Semantic Error: Undefined variable '%s' on RHS of assignment at line %d, col %d",
                              val->ident.name, val->line, val->col);
                    return false;
                }
                if (!s) {
                    s = symbol_table_insert(sem->symtab, node->assign.name, rhs->type, "global", node->line, node->col);
                }
                if (rhs->type == TYPE_INT) {
                    symbol_table_set_int(sem->symtab, node->assign.name, rhs->int_val);
                } else if (rhs->type == TYPE_STATE) {
                    symbol_table_set_state(sem->symtab, node->assign.name, rhs->state_val);
                }
            } else {
                sem_error(sem, node->line, node->col,
                          "Semantic Error: Unsupported assignment expression at line %d, col %d",
                          node->line, node->col);
                return false;
            }
            return true;
        }

        case AST_IF_STMT: {
            DataType cond_type = infer_type(sem, node->if_stmt.condition);
            if (cond_type != TYPE_BOOL) {
                if (!sem->has_error) {
                    sem_error(sem, node->line, node->col,
                              "Semantic Error: IF condition must evaluate to boolean at line %d, col %d",
                              node->line, node->col);
                }
                return false;
            }

            if (!check_node(sem, node->if_stmt.then_block)) return false;
            if (node->if_stmt.else_block) {
                if (!check_node(sem, node->if_stmt.else_block)) return false;
            }
            return true;
        }

        case AST_ACTION:
            if (node->action.action == ACTION_NONE) {
                sem_error(sem, node->line, node->col, "Semantic Error: Unknown action at line %d", node->line);
                return false;
            }
            return true;

        default:
            return true;
    }
}

bool semantic_analyze(SemanticAnalyzer* sem, const ASTNode* root) {
    if (!root) return false;
    return check_node(sem, root);
}

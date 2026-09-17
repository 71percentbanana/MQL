#include "symbol_table.h"

const char* data_type_to_str(DataType type) {
    switch (type) {
        case TYPE_INT:     return "integer";
        case TYPE_STATE:   return "state";
        case TYPE_VEHICLE: return "vehicle";
        default:           return "unknown";
    }
}

void symbol_table_init(SymbolTable* st) {
    st->count = 0;
    st->capacity = 16;
    st->entries = (Symbol*)malloc(sizeof(Symbol) * st->capacity);
}

Symbol* symbol_table_lookup(SymbolTable* st, const char* name) {
    for (int i = 0; i < st->count; i++) {
        if (strcmp(st->entries[i].name, name) == 0) {
            return &st->entries[i];
        }
    }
    return NULL;
}

Symbol* symbol_table_insert(SymbolTable* st, const char* name, DataType type, const char* scope, int line, int col) {
    Symbol* existing = symbol_table_lookup(st, name);
    if (existing) {
        if (type != TYPE_UNKNOWN) {
            existing->type = type;
        }
        existing->line = line;
        existing->col = col;
        return existing;
    }

    if (st->count >= st->capacity) {
        st->capacity *= 2;
        st->entries = (Symbol*)realloc(st->entries, sizeof(Symbol) * st->capacity);
    }

    Symbol* sym = &st->entries[st->count++];
    strncpy(sym->name, name, MAX_IDENT_LEN - 1);
    sym->name[MAX_IDENT_LEN - 1] = '\0';
    sym->type = type;
    strncpy(sym->scope, scope ? scope : "global", sizeof(sym->scope) - 1);
    sym->scope[sizeof(sym->scope) - 1] = '\0';
    sym->is_initialized = false;
    sym->int_val = 0;
    sym->state_val = STATE_NONE;
    sym->line = line;
    sym->col = col;

    return sym;
}

bool symbol_table_set_int(SymbolTable* st, const char* name, int val) {
    Symbol* s = symbol_table_lookup(st, name);
    if (!s) return false;
    s->type = TYPE_INT;
    s->int_val = val;
    s->is_initialized = true;
    return true;
}

bool symbol_table_set_state(SymbolTable* st, const char* name, StateValue state) {
    Symbol* s = symbol_table_lookup(st, name);
    if (!s) return false;
    s->type = TYPE_STATE;
    s->state_val = state;
    s->is_initialized = true;
    return true;
}

static void walk_ast_for_symbols(SymbolTable* st, const ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case AST_PROGRAM:
            for (int i = 0; i < node->program.stmt_count; i++) {
                walk_ast_for_symbols(st, node->program.statements[i]);
            }
            break;

        case AST_VAR_DECL:
            symbol_table_insert(st, node->var_decl.name, TYPE_VEHICLE, "global", node->line, node->col);
            break;

        case AST_ASSIGN: {
            Symbol* s = symbol_table_insert(st, node->assign.name, TYPE_UNKNOWN, "global", node->line, node->col);
            if (node->assign.value) {
                if (node->assign.value->type == AST_LITERAL_INT) {
                    s->type = TYPE_INT;
                    s->int_val = node->assign.value->lit_int.value;
                    s->is_initialized = true;
                } else if (node->assign.value->type == AST_LITERAL_STATE) {
                    s->type = TYPE_STATE;
                    s->state_val = node->assign.value->lit_state.state;
                    s->is_initialized = true;
                }
            }
            break;
        }

        case AST_IF_STMT:
            walk_ast_for_symbols(st, node->if_stmt.condition);
            walk_ast_for_symbols(st, node->if_stmt.then_block);
            if (node->if_stmt.else_block) {
                walk_ast_for_symbols(st, node->if_stmt.else_block);
            }
            break;

        case AST_BINARY_OP:
            walk_ast_for_symbols(st, node->binary_op.left);
            walk_ast_for_symbols(st, node->binary_op.right);
            break;

        case AST_IDENTIFIER: {
            Symbol* s = symbol_table_lookup(st, node->ident.name);
            if (!s) {
                symbol_table_insert(st, node->ident.name, TYPE_UNKNOWN, "global", node->line, node->col);
            }
            break;
        }

        default:
            break;
    }
}

void symbol_table_build_from_ast(SymbolTable* st, const ASTNode* ast) {
    walk_ast_for_symbols(st, ast);
}

void symbol_table_print(const SymbolTable* st) {
    printf("+-----+--------------------+------------+------------+---------+\n");
    printf("| #   | Identifier         | Type       | Value      | Scope   |\n");
    printf("+-----+--------------------+------------+------------+---------+\n");

    for (int i = 0; i < st->count; i++) {
        const Symbol* s = &st->entries[i];
        char val_str[32] = "-";

        if (s->type == TYPE_INT && s->is_initialized) {
            snprintf(val_str, sizeof(val_str), "%d", s->int_val);
        } else if (s->type == TYPE_STATE && s->is_initialized) {
            snprintf(val_str, sizeof(val_str), "%s", state_to_str(s->state_val));
        }

        printf("| %-3d | %-18s | %-10s | %-10s | %-7s |\n",
               i + 1,
               s->name,
               data_type_to_str(s->type),
               val_str,
               s->scope);
    }

    printf("+-----+--------------------+------------+------------+---------+\n");
    printf("Total Symbols Registered: %d\n", st->count);
}

void symbol_table_free(SymbolTable* st) {
    if (st->entries) {
        free(st->entries);
        st->entries = NULL;
    }
    st->count = 0;
    st->capacity = 0;
}

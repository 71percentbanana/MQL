#include "symbol_table.h"

void symbol_table_init(SymbolTable* st) {
    st->capacity = 16;
    st->count = 0;
    st->entries = (Symbol*)malloc(sizeof(Symbol) * st->capacity);
}

Symbol* symbol_table_lookup(SymbolTable* st, const char* name) {
    if (!st || !name) return NULL;
    for (int i = 0; i < st->count; i++) {
        if (strcmp(st->entries[i].name, name) == 0) {
            return &st->entries[i];
        }
    }
    return NULL;
}

Symbol* symbol_table_insert(SymbolTable* st, const char* name, DataType type, const char* scope, int line, int col) {
    if (!st || !name) return NULL;

    Symbol* existing = symbol_table_lookup(st, name);
    if (existing) {
        // Already exists, return existing
        return existing;
    }

    if (st->count >= st->capacity) {
        st->capacity *= 2;
        st->entries = (Symbol*)realloc(st->entries, sizeof(Symbol) * st->capacity);
    }

    Symbol* sym = &st->entries[st->count++];
    memset(sym, 0, sizeof(Symbol));
    strncpy(sym->name, name, MAX_IDENT_LEN - 1);
    sym->type = type;
    strncpy(sym->scope, scope ? scope : "global", sizeof(sym->scope) - 1);
    sym->line = line;
    sym->col = col;
    sym->is_initialized = false;
    return sym;
}

bool symbol_table_set_int(SymbolTable* st, const char* name, int val) {
    Symbol* sym = symbol_table_lookup(st, name);
    if (!sym) return false;
    sym->type = TYPE_INT;
    sym->int_val = val;
    sym->is_initialized = true;
    return true;
}

bool symbol_table_set_state(SymbolTable* st, const char* name, StateValue state) {
    Symbol* sym = symbol_table_lookup(st, name);
    if (!sym) return false;
    sym->type = TYPE_STATE;
    sym->state_val = state;
    sym->is_initialized = true;
    return true;
}

void symbol_table_print(const SymbolTable* st) {
    printf("+----------------------+------------+---------------+----------+\n");
    printf("| Name                 | Type       | Value         | Scope    |\n");
    printf("+----------------------+------------+---------------+----------+\n");

    for (int i = 0; i < st->count; i++) {
        const Symbol* s = &st->entries[i];
        char val_buf[32] = "-";

        if (s->is_initialized) {
            if (s->type == TYPE_INT) {
                snprintf(val_buf, sizeof(val_buf), "%d", s->int_val);
            } else if (s->type == TYPE_STATE) {
                snprintf(val_buf, sizeof(val_buf), "%s", state_value_to_str(s->state_val));
            } else if (s->type == TYPE_VEHICLE) {
                snprintf(val_buf, sizeof(val_buf), "-");
            }
        }

        printf("| %-20s | %-10s | %-13s | %-8s |\n",
               s->name,
               data_type_to_str(s->type),
               val_buf,
               s->scope);
    }
    printf("+----------------------+------------+---------------+----------+\n");
}

void symbol_table_free(SymbolTable* st) {
    if (st && st->entries) {
        free(st->entries);
        st->entries = NULL;
        st->count = 0;
        st->capacity = 0;
    }
}

#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "common.h"

typedef struct {
    char name[MAX_IDENT_LEN];
    DataType type;
    char scope[32];

    bool is_initialized;
    int int_val;
    StateValue state_val;

    int line;
    int col;
} Symbol;

typedef struct {
    Symbol* entries;
    int count;
    int capacity;
} SymbolTable;

void symbol_table_init(SymbolTable* st);
Symbol* symbol_table_lookup(SymbolTable* st, const char* name);
Symbol* symbol_table_insert(SymbolTable* st, const char* name, DataType type, const char* scope, int line, int col);
bool symbol_table_set_int(SymbolTable* st, const char* name, int val);
bool symbol_table_set_state(SymbolTable* st, const char* name, StateValue state);
void symbol_table_print(const SymbolTable* st);
void symbol_table_free(SymbolTable* st);

#endif // SYMBOL_TABLE_H

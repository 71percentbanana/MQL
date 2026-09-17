#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "common.h"
#include "ir.h"
#include "symbol_table.h"

typedef struct {
    char name[MAX_IDENT_LEN];
    DataType type;
    int int_val;
    char str_val[MAX_IDENT_LEN];
} RuntimeVar;

typedef struct {
    RuntimeVar vars[256];
    int var_count;

    int violations_count;
    int actions_executed;
} Interpreter;

void interpreter_init(Interpreter* interp);
void interpreter_load_symbol_table(Interpreter* interp, const SymbolTable* st);
void interpreter_set_int(Interpreter* interp, const char* name, int val);
void interpreter_set_state(Interpreter* interp, const char* name, const char* state_str);
bool interpreter_execute(Interpreter* interp, const IRProgram* ir);

#endif // INTERPRETER_H

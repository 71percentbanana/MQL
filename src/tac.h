#ifndef TAC_H
#define TAC_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "sql_ast.h"

#define MAX_INSTRUCTIONS 32

typedef struct {
    char op[16];
    char arg1[64];
    char arg2[64];
    char result[32];
} Quadruple;

typedef struct {
    char code[128];
} TACInstruction;

typedef struct {
    Quadruple quads[MAX_INSTRUCTIONS];
    int quad_count;
    TACInstruction instructions[MAX_INSTRUCTIONS];
    int instruction_count;
    int temp_count;
} TACProgram;

void tac_generate(const ASTQuery* query, TACProgram* prog);
void tac_print(const TACProgram* prog);
void quads_print(const TACProgram* prog);

#endif // TAC_H

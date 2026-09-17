#ifndef IR_H
#define IR_H

#include "common.h"
#include "ast.h"

typedef enum {
    IR_LABEL,
    IR_ASSIGN,
    IR_BINARY_OP,
    IR_IF_FALSE_GOTO,
    IR_GOTO,
    IR_ACTION
} IROp;

typedef struct {
    IROp op;
    char op_name[16];   // e.g. ">", "==", "AND", "ASSIGN", "IF_FALSE", "GOTO", "LABEL"
    char arg1[MAX_IDENT_LEN];
    char arg2[MAX_IDENT_LEN];
    char result[MAX_IDENT_LEN];
} TACInstruction;

typedef struct {
    TACInstruction* code;
    int count;
    int capacity;
    int temp_count;
    int label_count;
} IRProgram;

void ir_init(IRProgram* ir);
void ir_generate(IRProgram* ir, const ASTNode* root);
void ir_print_tac(const IRProgram* ir);
void ir_print_quadruples(const IRProgram* ir);
void ir_free(IRProgram* ir);

#endif // IR_H

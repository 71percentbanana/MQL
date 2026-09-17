#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "common.h"
#include "ir.h"

typedef struct {
    int constant_folds;
    int dead_instructions_removed;
    int branches_pruned;
} OptimizationStats;

void optimize_ir(const IRProgram* input, IRProgram* output, OptimizationStats* stats);

#endif // OPTIMIZER_H

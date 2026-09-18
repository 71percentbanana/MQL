#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "sql_ast.h"
#include "tac.h"

typedef struct {
    int constant_fold_count;
    int predicate_pushdown_count;
    int projection_prune_count;
    bool has_dead_code_eliminated;
} OptimizationReport;

void optimizer_run(ASTQuery* query, TACProgram* tac, OptimizationReport* report);
void optimizer_print_report(const OptimizationReport* report);

#endif // OPTIMIZER_H

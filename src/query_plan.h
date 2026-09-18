#ifndef QUERY_PLAN_H
#define QUERY_PLAN_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "sql_ast.h"

typedef enum {
    PLAN_SCAN,
    PLAN_FILTER,
    PLAN_SORT,
    PLAN_LIMIT,
    PLAN_PROJECT
} PlanOpType;

typedef struct {
    PlanOpType type;
    char description[128];
    char cost_estimate[32];
} PlanStep;

typedef struct {
    PlanStep steps[8];
    int step_count;
    char collection[64];
    bool is_optimized;
} QueryPlan;

void plan_generate(const ASTQuery* query, QueryPlan* plan);
void plan_optimize(QueryPlan* plan);
void plan_print(const QueryPlan* plan);

#endif // QUERY_PLAN_H

#ifndef SQL_AST_H
#define SQL_AST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef enum {
    OP_EQ,
    OP_NEQ,
    OP_GT,
    OP_GTE,
    OP_LT,
    OP_LTE
} ComparisonOp;

typedef enum {
    LOGIC_AND,
    LOGIC_OR
} LogicalOp;

typedef enum {
    LIT_INT,
    LIT_FLOAT,
    LIT_STRING
} LiteralType;

typedef struct {
    LiteralType type;
    double num_val;
    char str_val[128];
} ASTLiteral;

typedef enum {
    FILTER_BINARY,
    FILTER_LOGICAL
} FilterNodeType;

typedef struct ASTFilter {
    FilterNodeType type;
    
    // For FILTER_BINARY (e.g. age >= 20)
    char field[64];
    ComparisonOp comp_op;
    ASTLiteral literal;

    // For FILTER_LOGICAL (e.g. left AND right)
    LogicalOp logical_op;
    struct ASTFilter* left;
    struct ASTFilter* right;
} ASTFilter;

typedef struct {
    bool is_wildcard;
    char fields[32][64];
    int field_count;
} ASTProjection;

typedef struct {
    bool has_order;
    char field[64];
    bool is_desc;
} ASTOrderBy;

typedef struct {
    bool has_limit;
    int limit_count;
} ASTLimit;

typedef struct {
    char collection[64];
    ASTProjection projection;
    ASTFilter* where_clause;
    ASTOrderBy order_by;
    ASTLimit limit;
} ASTQuery;

// Allocation helpers
ASTFilter* ast_create_binary_filter(const char* field, ComparisonOp op, ASTLiteral lit);
ASTFilter* ast_create_logical_filter(LogicalOp op, ASTFilter* left, ASTFilter* right);

// Visualization & Cleanup
const char* comp_op_str(ComparisonOp op);
void ast_print(const ASTQuery* query);
void ast_free(ASTQuery* query);

#endif // SQL_AST_H

#include "sql_ast.h"

ASTFilter* ast_create_binary_filter(const char* field, ComparisonOp op, ASTLiteral lit) {
    ASTFilter* node = (ASTFilter*)malloc(sizeof(ASTFilter));
    if (!node) return NULL;
    node->type = FILTER_BINARY;
    strncpy(node->field, field, sizeof(node->field) - 1);
    node->field[sizeof(node->field) - 1] = '\0';
    node->comp_op = op;
    node->literal = lit;
    node->logical_op = LOGIC_AND;
    node->left = NULL;
    node->right = NULL;
    return node;
}

ASTFilter* ast_create_logical_filter(LogicalOp op, ASTFilter* left, ASTFilter* right) {
    ASTFilter* node = (ASTFilter*)malloc(sizeof(ASTFilter));
    if (!node) return NULL;
    node->type = FILTER_LOGICAL;
    node->field[0] = '\0';
    node->logical_op = op;
    node->left = left;
    node->right = right;
    return node;
}

const char* comp_op_str(ComparisonOp op) {
    switch (op) {
        case OP_EQ:  return "==";
        case OP_NEQ: return "!=";
        case OP_GT:  return ">";
        case OP_GTE: return ">=";
        case OP_LT:  return "<";
        case OP_LTE: return "<=";
        default:     return "??";
    }
}

static void print_indent(int level) {
    for (int i = 0; i < level; i++) {
        printf("  ");
    }
}

static void ast_print_filter(const ASTFilter* filter, int indent) {
    if (!filter) return;

    if (filter->type == FILTER_BINARY) {
        print_indent(indent);
        if (filter->literal.type == LIT_STRING) {
            printf("BinaryCondition: [%s %s '%s']\n",
                   filter->field, comp_op_str(filter->comp_op), filter->literal.str_val);
        } else if (filter->literal.type == LIT_INT) {
            printf("BinaryCondition: [%s %s %d]\n",
                   filter->field, comp_op_str(filter->comp_op), (int)filter->literal.num_val);
        } else {
            printf("BinaryCondition: [%s %s %.2f]\n",
                   filter->field, comp_op_str(filter->comp_op), filter->literal.num_val);
        }
    } else if (filter->type == FILTER_LOGICAL) {
        print_indent(indent);
        printf("LogicalCondition: %s\n", filter->logical_op == LOGIC_AND ? "AND" : "OR");
        
        print_indent(indent + 1);
        printf("[Left]:\n");
        ast_print_filter(filter->left, indent + 2);
        
        print_indent(indent + 1);
        printf("[Right]:\n");
        ast_print_filter(filter->right, indent + 2);
    }
}

void ast_print(const ASTQuery* query) {
    printf("\nAST Structure (Hierarchical Syntax Tree):\n");
    printf("===========================================\n");
    printf("SelectQueryNode\n");

    // Projections
    printf("  [Projections]: ");
    if (query->projection.is_wildcard) {
        printf("* (All Columns)\n");
    } else {
        for (int i = 0; i < query->projection.field_count; i++) {
            printf("%s%s", query->projection.fields[i],
                   (i == query->projection.field_count - 1) ? "\n" : ", ");
        }
    }

    // Collection
    printf("  [From Collection]: %s\n", query->collection);

    // Where Clause
    if (query->where_clause) {
        printf("  [Where Filter]:\n");
        ast_print_filter(query->where_clause, 2);
    } else {
        printf("  [Where Filter]: <None>\n");
    }

    // Order By
    if (query->order_by.has_order) {
        printf("  [Order By]: %s (%s)\n",
               query->order_by.field,
               query->order_by.is_desc ? "DESCENDING" : "ASCENDING");
    } else {
        printf("  [Order By]: <None>\n");
    }

    // Limit
    if (query->limit.has_limit) {
        printf("  [Limit]: %d\n", query->limit.limit_count);
    } else {
        printf("  [Limit]: <None>\n");
    }
    printf("===========================================\n");
}

static void ast_free_filter(ASTFilter* filter) {
    if (!filter) return;
    if (filter->type == FILTER_LOGICAL) {
        ast_free_filter(filter->left);
        ast_free_filter(filter->right);
    }
    free(filter);
}

void ast_free(ASTQuery* query) {
    if (!query) return;
    if (query->where_clause) {
        ast_free_filter(query->where_clause);
        query->where_clause = NULL;
    }
}

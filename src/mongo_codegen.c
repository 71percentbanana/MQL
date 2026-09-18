#include "mongo_codegen.h"

static void emit_comparison_json(const ASTFilter* f, char* buf, size_t sz) {
    if (f->comp_op == OP_EQ) {
        if (f->literal.type == LIT_STRING) {
            snprintf(buf, sz, "\"%s\": \"%s\"", f->field, f->literal.str_val);
        } else if (f->literal.type == LIT_INT) {
            snprintf(buf, sz, "\"%s\": %d", f->field, (int)f->literal.num_val);
        } else {
            snprintf(buf, sz, "\"%s\": %.2f", f->field, f->literal.num_val);
        }
    } else {
        const char* mongo_op = "$eq";
        switch (f->comp_op) {
            case OP_NEQ: mongo_op = "$ne"; break;
            case OP_GT:  mongo_op = "$gt"; break;
            case OP_GTE: mongo_op = "$gte"; break;
            case OP_LT:  mongo_op = "$lt"; break;
            case OP_LTE: mongo_op = "$lte"; break;
            default:     mongo_op = "$eq"; break;
        }

        if (f->literal.type == LIT_STRING) {
            snprintf(buf, sz, "\"%s\": { \"%s\": \"%s\" }", f->field, mongo_op, f->literal.str_val);
        } else if (f->literal.type == LIT_INT) {
            snprintf(buf, sz, "\"%s\": { \"%s\": %d }", f->field, mongo_op, (int)f->literal.num_val);
        } else {
            snprintf(buf, sz, "\"%s\": { \"%s\": %.2f }", f->field, mongo_op, f->literal.num_val);
        }
    }
}

static void emit_filter_json(const ASTFilter* f, char* buf, size_t sz, int indent) {
    if (!f) return;

    if (f->type == FILTER_BINARY) {
        char cond[256];
        emit_comparison_json(f, cond, sizeof(cond));
        snprintf(buf, sz, "{ %s }", cond);
    } else if (f->type == FILTER_LOGICAL) {
        char left_buf[512];
        char right_buf[512];
        emit_filter_json(f->left, left_buf, sizeof(left_buf), indent + 1);
        emit_filter_json(f->right, right_buf, sizeof(right_buf), indent + 1);

        const char* log_op = (f->logical_op == LOGIC_AND) ? "$and" : "$or";
        snprintf(buf, sz,
                 "{\n"
                 "        \"%s\": [\n"
                 "          %s,\n"
                 "          %s\n"
                 "        ]\n"
                 "      }",
                 log_op, left_buf, right_buf);
    }
}

void mongo_codegen_pipeline(const ASTQuery* query, char* out_buf, size_t buf_sz) {
    out_buf[0] = '\0';

    char temp[4096];
    snprintf(temp, sizeof(temp), "db.%s.aggregate([\n", query->collection);
    strncat(out_buf, temp, buf_sz - strlen(out_buf) - 1);

    bool has_prev = false;

    // 1. $match stage
    if (query->where_clause) {
        char filter_str[1024];
        emit_filter_json(query->where_clause, filter_str, sizeof(filter_str), 1);
        snprintf(temp, sizeof(temp),
                 "  {\n"
                 "    \"$match\": %s\n"
                 "  }", filter_str);
        strncat(out_buf, temp, buf_sz - strlen(out_buf) - 1);
        has_prev = true;
    }

    // 2. $sort stage
    if (query->order_by.has_order) {
        if (has_prev) strncat(out_buf, ",\n", buf_sz - strlen(out_buf) - 1);
        snprintf(temp, sizeof(temp),
                 "  {\n"
                 "    \"$sort\": { \"%s\": %d }\n"
                 "  }",
                 query->order_by.field,
                 query->order_by.is_desc ? -1 : 1);
        strncat(out_buf, temp, buf_sz - strlen(out_buf) - 1);
        has_prev = true;
    }

    // 3. $limit stage
    if (query->limit.has_limit) {
        if (has_prev) strncat(out_buf, ",\n", buf_sz - strlen(out_buf) - 1);
        snprintf(temp, sizeof(temp),
                 "  {\n"
                 "    \"$limit\": %d\n"
                 "  }", query->limit.limit_count);
        strncat(out_buf, temp, buf_sz - strlen(out_buf) - 1);
        has_prev = true;
    }

    // 4. $project stage (if not wildcard)
    if (!query->projection.is_wildcard && query->projection.field_count > 0) {
        if (has_prev) strncat(out_buf, ",\n", buf_sz - strlen(out_buf) - 1);
        strncat(out_buf, "  {\n    \"$project\": {\n      \"_id\": 0", buf_sz - strlen(out_buf) - 1);
        for (int i = 0; i < query->projection.field_count; i++) {
            snprintf(temp, sizeof(temp), ",\n      \"%s\": 1", query->projection.fields[i]);
            strncat(out_buf, temp, buf_sz - strlen(out_buf) - 1);
        }
        strncat(out_buf, "\n    }\n  }", buf_sz - strlen(out_buf) - 1);
    }

    strncat(out_buf, "\n]);\n", buf_sz - strlen(out_buf) - 1);
}

void mongo_print_code(const ASTQuery* query) {
    char pipeline[8192];
    mongo_codegen_pipeline(query, pipeline, sizeof(pipeline));
    printf("\nTarget Code Generation: MongoDB Query Language (MQL):\n");
    printf("======================================================================\n");
    printf("%s", pipeline);
    printf("======================================================================\n");
}

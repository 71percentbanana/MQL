#include "tac.h"

static void add_tac_quad(TACProgram* p, const char* op, const char* arg1, const char* arg2, const char* res, const char* tac_code) {
    if (p->quad_count < MAX_INSTRUCTIONS) {
        Quadruple* q = &p->quads[p->quad_count++];
        strncpy(q->op, op, sizeof(q->op) - 1);
        q->op[sizeof(q->op) - 1] = '\0';

        strncpy(q->arg1, arg1, sizeof(q->arg1) - 1);
        q->arg1[sizeof(q->arg1) - 1] = '\0';

        strncpy(q->arg2, arg2, sizeof(q->arg2) - 1);
        q->arg2[sizeof(q->arg2) - 1] = '\0';

        strncpy(q->result, res, sizeof(q->result) - 1);
        q->result[sizeof(q->result) - 1] = '\0';
    }

    if (p->instruction_count < MAX_INSTRUCTIONS) {
        TACInstruction* t = &p->instructions[p->instruction_count++];
        strncpy(t->code, tac_code, sizeof(t->code) - 1);
        t->code[sizeof(t->code) - 1] = '\0';
    }
}

static void generate_filter_tac(TACProgram* p, const ASTFilter* f, char* out_res, size_t sz) {
    if (!f) {
        out_res[0] = '\0';
        return;
    }

    if (f->type == FILTER_BINARY) {
        char lit_str[64];
        if (f->literal.type == LIT_STRING) {
            snprintf(lit_str, sizeof(lit_str), "'%s'", f->literal.str_val);
        } else if (f->literal.type == LIT_INT) {
            snprintf(lit_str, sizeof(lit_str), "%d", (int)f->literal.num_val);
        } else {
            snprintf(lit_str, sizeof(lit_str), "%.2f", f->literal.num_val);
        }

        snprintf(out_res, sz, "t%d", ++p->temp_count);

        char tac_line[128];
        snprintf(tac_line, sizeof(tac_line), "%s = %s %s %s",
                 out_res, f->field, comp_op_str(f->comp_op), lit_str);

        const char* op_label = comp_op_str(f->comp_op);
        add_tac_quad(p, op_label, f->field, lit_str, out_res, tac_line);
    } else if (f->type == FILTER_LOGICAL) {
        char left_t[32];
        char right_t[32];
        generate_filter_tac(p, f->left, left_t, sizeof(left_t));
        generate_filter_tac(p, f->right, right_t, sizeof(right_t));

        snprintf(out_res, sz, "t%d", ++p->temp_count);

        const char* log_op = (f->logical_op == LOGIC_AND) ? "AND" : "OR";
        char tac_line[128];
        snprintf(tac_line, sizeof(tac_line), "%s = %s %s %s",
                 out_res, left_t, log_op, right_t);

        add_tac_quad(p, log_op, left_t, right_t, out_res, tac_line);
    }
}

void tac_generate(const ASTQuery* query, TACProgram* prog) {
    memset(prog, 0, sizeof(TACProgram));
    prog->temp_count = 0;

    // 1. SCAN
    char scan_t[32];
    snprintf(scan_t, sizeof(scan_t), "t%d", ++prog->temp_count);
    char tac_buf[128];
    snprintf(tac_buf, sizeof(tac_buf), "%s = SCAN(%s)", scan_t, query->collection);
    add_tac_quad(prog, "SCAN", query->collection, "-", scan_t, tac_buf);

    char current_t[32];
    strncpy(current_t, scan_t, sizeof(current_t));

    // 2. FILTER
    if (query->where_clause) {
        char cond_t[32];
        generate_filter_tac(prog, query->where_clause, cond_t, sizeof(cond_t));

        char filter_t[32];
        snprintf(filter_t, sizeof(filter_t), "t%d", ++prog->temp_count);
        snprintf(tac_buf, sizeof(tac_buf), "%s = FILTER(%s, %s)", filter_t, current_t, cond_t);
        add_tac_quad(prog, "FILTER", current_t, cond_t, filter_t, tac_buf);
        strncpy(current_t, filter_t, sizeof(current_t));
    }

    // 3. SORT
    if (query->order_by.has_order) {
        char sort_t[32];
        snprintf(sort_t, sizeof(sort_t), "t%d", ++prog->temp_count);
        char sort_arg[64];
        snprintf(sort_arg, sizeof(sort_arg), "%s (%s)",
                 query->order_by.field,
                 query->order_by.is_desc ? "DESC" : "ASC");

        snprintf(tac_buf, sizeof(tac_buf), "%s = SORT(%s, %s)", sort_t, current_t, sort_arg);
        add_tac_quad(prog, "SORT", current_t, sort_arg, sort_t, tac_buf);
        strncpy(current_t, sort_t, sizeof(current_t));
    }

    // 4. LIMIT
    if (query->limit.has_limit) {
        char limit_t[32];
        snprintf(limit_t, sizeof(limit_t), "t%d", ++prog->temp_count);
        char lim_arg[32];
        snprintf(lim_arg, sizeof(lim_arg), "%d", query->limit.limit_count);

        snprintf(tac_buf, sizeof(tac_buf), "%s = LIMIT(%s, %s)", limit_t, current_t, lim_arg);
        add_tac_quad(prog, "LIMIT", current_t, lim_arg, limit_t, tac_buf);
        strncpy(current_t, limit_t, sizeof(current_t));
    }

    // 5. PROJECT
    char proj_t[32];
    snprintf(proj_t, sizeof(proj_t), "t%d", ++prog->temp_count);
    char proj_arg[128] = "";
    if (query->projection.is_wildcard) {
        strcpy(proj_arg, "*");
    } else {
        for (int i = 0; i < query->projection.field_count; i++) {
            strncat(proj_arg, query->projection.fields[i], sizeof(proj_arg) - strlen(proj_arg) - 1);
            if (i < query->projection.field_count - 1) {
                strncat(proj_arg, ",", sizeof(proj_arg) - strlen(proj_arg) - 1);
            }
        }
    }
    snprintf(tac_buf, sizeof(tac_buf), "%s = PROJECT(%s, [%s])", proj_t, current_t, proj_arg);
    add_tac_quad(prog, "PROJECT", current_t, proj_arg, proj_t, tac_buf);
}

void tac_print(const TACProgram* prog) {
    printf("\nThree-Address Code (TAC) Instructions:\n");
    printf("======================================================================\n");
    for (int i = 0; i < prog->instruction_count; i++) {
        printf("  (%d)  %s\n", i + 1, prog->instructions[i].code);
    }
    printf("======================================================================\n");
}

void quads_print(const TACProgram* prog) {
    printf("\nQuadruple Representation Table (Op, Arg1, Arg2, Result):\n");
    printf("======================================================================\n");
    printf("+----+------------+----------------------+----------------------+--------+\n");
    printf("| #  | Op         | Arg1                 | Arg2                 | Result |\n");
    printf("+----+------------+----------------------+----------------------+--------+\n");
    for (int i = 0; i < prog->quad_count; i++) {
        printf("| %-2d | %-10s | %-20s | %-20s | %-6s |\n",
               i + 1,
               prog->quads[i].op,
               prog->quads[i].arg1,
               prog->quads[i].arg2,
               prog->quads[i].result);
    }
    printf("+----+------------+----------------------+----------------------+--------+\n");
    printf("Total Quadruples: %d | Temporaries Used: %d\n", prog->quad_count, prog->temp_count);
}

#include "interpreter.h"

void interpreter_init(Interpreter* interp) {
    interp->var_count = 0;
    interp->violations_count = 0;
    interp->actions_executed = 0;
}

static RuntimeVar* interp_find_or_create(Interpreter* interp, const char* name) {
    for (int i = 0; i < interp->var_count; i++) {
        if (strcmp(interp->vars[i].name, name) == 0) {
            return &interp->vars[i];
        }
    }
    if (interp->var_count < 256) {
        RuntimeVar* v = &interp->vars[interp->var_count++];
        memset(v, 0, sizeof(RuntimeVar));
        strncpy(v->name, name, MAX_IDENT_LEN - 1);
        return v;
    }
    return NULL;
}

void interpreter_load_symbol_table(Interpreter* interp, const SymbolTable* st) {
    if (!st) return;
    for (int i = 0; i < st->count; i++) {
        const Symbol* s = &st->entries[i];
        RuntimeVar* v = interp_find_or_create(interp, s->name);
        if (!v) continue;
        v->type = s->type;
        if (s->is_initialized) {
            if (s->type == TYPE_INT) {
                v->int_val = s->int_val;
            } else if (s->type == TYPE_STATE) {
                strncpy(v->str_val, state_value_to_str(s->state_val), sizeof(v->str_val) - 1);
            }
        }
    }
}

void interpreter_set_int(Interpreter* interp, const char* name, int val) {
    RuntimeVar* v = interp_find_or_create(interp, name);
    if (v) {
        v->type = TYPE_INT;
        v->int_val = val;
    }
}

void interpreter_set_state(Interpreter* interp, const char* name, const char* state_str) {
    RuntimeVar* v = interp_find_or_create(interp, name);
    if (v) {
        v->type = TYPE_STATE;
        strncpy(v->str_val, state_str, sizeof(v->str_val) - 1);
    }
}

static bool is_digits(const char* s) {
    if (!s || !*s) return false;
    for (int i = 0; s[i]; i++) {
        if (!isdigit((unsigned char)s[i])) return false;
    }
    return true;
}

static bool is_state_string(const char* s) {
    return (strcmp(s, "RED") == 0 ||
            strcmp(s, "GREEN") == 0 ||
            strcmp(s, "YELLOW") == 0 ||
            strcmp(s, "CROSSING") == 0 ||
            strcmp(s, "ROAD") == 0);
}

static void resolve_operand(Interpreter* interp, const char* op_str, bool* is_num, int* num_val, char* str_val) {
    if (is_digits(op_str)) {
        *is_num = true;
        *num_val = atoi(op_str);
        return;
    }

    if (is_state_string(op_str)) {
        *is_num = false;
        strncpy(str_val, op_str, MAX_IDENT_LEN - 1);
        str_val[MAX_IDENT_LEN - 1] = '\0';
        return;
    }

    for (int i = 0; i < interp->var_count; i++) {
        if (strcmp(interp->vars[i].name, op_str) == 0) {
            if (interp->vars[i].type == TYPE_INT || interp->vars[i].type == TYPE_BOOL) {
                *is_num = true;
                *num_val = interp->vars[i].int_val;
            } else {
                *is_num = false;
                strncpy(str_val, interp->vars[i].str_val, MAX_IDENT_LEN - 1);
                str_val[MAX_IDENT_LEN - 1] = '\0';
            }
            return;
        }
    }

    // Default fallback
    *is_num = true;
    *num_val = 0;
}

static int find_label(const IRProgram* ir, const char* label) {
    for (int i = 0; i < ir->count; i++) {
        if (ir->code[i].op == IR_LABEL && strcmp(ir->code[i].result, label) == 0) {
            return i;
        }
    }
    return -1;
}

bool interpreter_execute(Interpreter* interp, const IRProgram* ir) {
    int pc = 0;
    int max_cycles = 10000;
    int cycles = 0;

    while (pc < ir->count && cycles++ < max_cycles) {
        const TACInstruction* inst = &ir->code[pc];

        switch (inst->op) {
            case IR_LABEL:
                pc++;
                break;

            case IR_ASSIGN: {
                bool is_num = false;
                int n = 0;
                char s[MAX_IDENT_LEN] = "";
                resolve_operand(interp, inst->arg1, &is_num, &n, s);

                RuntimeVar* target = interp_find_or_create(interp, inst->result);
                if (target) {
                    if (is_num) {
                        target->type = TYPE_INT;
                        target->int_val = n;
                    } else {
                        target->type = TYPE_STATE;
                        strncpy(target->str_val, s, sizeof(target->str_val) - 1);
                    }
                }
                pc++;
                break;
            }

            case IR_BINARY_OP: {
                bool is_num1 = false, is_num2 = false;
                int n1 = 0, n2 = 0;
                char s1[MAX_IDENT_LEN] = "", s2[MAX_IDENT_LEN] = "";

                resolve_operand(interp, inst->arg1, &is_num1, &n1, s1);
                resolve_operand(interp, inst->arg2, &is_num2, &n2, s2);

                int result_val = 0;
                if (is_num1 && is_num2) {
                    if (strcmp(inst->op_name, ">") == 0) result_val = (n1 > n2);
                    else if (strcmp(inst->op_name, "<") == 0) result_val = (n1 < n2);
                    else if (strcmp(inst->op_name, ">=") == 0) result_val = (n1 >= n2);
                    else if (strcmp(inst->op_name, "<=") == 0) result_val = (n1 <= n2);
                    else if (strcmp(inst->op_name, "==") == 0) result_val = (n1 == n2);
                    else if (strcmp(inst->op_name, "!=") == 0) result_val = (n1 != n2);
                    else if (strcmp(inst->op_name, "AND") == 0) result_val = (n1 && n2);
                    else if (strcmp(inst->op_name, "OR") == 0) result_val = (n1 || n2);
                } else if (!is_num1 && !is_num2) {
                    if (strcmp(inst->op_name, "==") == 0) result_val = (strcmp(s1, s2) == 0);
                    else if (strcmp(inst->op_name, "!=") == 0) result_val = (strcmp(s1, s2) != 0);
                }

                RuntimeVar* target = interp_find_or_create(interp, inst->result);
                if (target) {
                    target->type = TYPE_BOOL;
                    target->int_val = result_val;
                }
                pc++;
                break;
            }

            case IR_IF_FALSE_GOTO: {
                bool is_num = false;
                int n = 0;
                char s[MAX_IDENT_LEN] = "";
                resolve_operand(interp, inst->arg1, &is_num, &n, s);

                if (n == 0) {
                    int target_pc = find_label(ir, inst->result);
                    if (target_pc < 0) {
                        fprintf(stderr, "Runtime Error: Jump target label '%s' not found!\n", inst->result);
                        return false;
                    }
                    pc = target_pc;
                } else {
                    pc++;
                }
                break;
            }

            case IR_GOTO: {
                int target_pc = find_label(ir, inst->result);
                if (target_pc < 0) {
                    fprintf(stderr, "Runtime Error: Goto target label '%s' not found!\n", inst->result);
                    return false;
                }
                pc = target_pc;
                break;
            }

            case IR_ACTION: {
                interp->actions_executed++;
                printf("\n=========================================\n");
                if (strcmp(inst->arg1, "ALLOW") == 0) {
                    printf("Traffic Check: COMPLIANT / PASS\n");
                    printf("Action Executed: %s\n", inst->arg1);
                } else {
                    interp->violations_count++;
                    printf("Traffic Violation Detected\n");
                    if (strcmp(inst->arg1, "ISSUE_FINE") == 0) {
                        printf("Type: Speed / Threshold Violation\n");
                    } else {
                        printf("Type: Traffic Signal / Crossing Violation\n");
                    }
                    printf("Action: %s\n", inst->arg1);
                }
                printf("=========================================\n");
                pc++;
                break;
            }

            default:
                pc++;
                break;
        }
    }

    if (cycles >= max_cycles) {
        fprintf(stderr, "Runtime Error: Execution step limit exceeded (possible loop)\n");
        return false;
    }

    return true;
}

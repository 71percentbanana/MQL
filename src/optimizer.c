#include "optimizer.h"

typedef struct {
    char name[MAX_IDENT_LEN];
    bool has_val;
    int int_val;
} ConstBinding;

static bool is_number_str(const char* s) {
    if (!s || !*s) return false;
    for (int i = 0; s[i]; i++) {
        if (!isdigit((unsigned char)s[i])) return false;
    }
    return true;
}

static bool get_known_value(ConstBinding* bindings, int count, const char* name, int* out_val) {
    if (is_number_str(name)) {
        *out_val = atoi(name);
        return true;
    }
    for (int i = 0; i < count; i++) {
        if (strcmp(bindings[i].name, name) == 0 && bindings[i].has_val) {
            *out_val = bindings[i].int_val;
            return true;
        }
    }
    return false;
}

static void set_known_value(ConstBinding* bindings, int* count, const char* name, int val) {
    for (int i = 0; i < *count; i++) {
        if (strcmp(bindings[i].name, name) == 0) {
            bindings[i].int_val = val;
            bindings[i].has_val = true;
            return;
        }
    }
    if (*count < 128) {
        strncpy(bindings[*count].name, name, MAX_IDENT_LEN - 1);
        bindings[*count].int_val = val;
        bindings[*count].has_val = true;
        (*count)++;
    }
}

void optimize_ir(const IRProgram* input, IRProgram* output, OptimizationStats* stats) {
    ir_init(output);
    if (stats) {
        stats->constant_folds = 0;
        stats->dead_instructions_removed = 0;
        stats->branches_pruned = 0;
    }

    ConstBinding bindings[128];
    int binding_count = 0;

    // Temporary array to hold first pass (constant folded)
    TACInstruction* pass1 = (TACInstruction*)malloc(sizeof(TACInstruction) * input->count);
    int pass1_count = 0;

    for (int i = 0; i < input->count; i++) {
        TACInstruction inst = input->code[i];

        if (inst.op == IR_BINARY_OP) {
            int v1, v2;
            bool ok1 = get_known_value(bindings, binding_count, inst.arg1, &v1);
            bool ok2 = get_known_value(bindings, binding_count, inst.arg2, &v2);

            if (ok1 && ok2) {
                int res = 0;
                if (strcmp(inst.op_name, ">") == 0) res = (v1 > v2);
                else if (strcmp(inst.op_name, "<") == 0) res = (v1 < v2);
                else if (strcmp(inst.op_name, ">=") == 0) res = (v1 >= v2);
                else if (strcmp(inst.op_name, "<=") == 0) res = (v1 <= v2);
                else if (strcmp(inst.op_name, "==") == 0) res = (v1 == v2);
                else if (strcmp(inst.op_name, "!=") == 0) res = (v1 != v2);
                else if (strcmp(inst.op_name, "AND") == 0) res = (v1 && v2);
                else if (strcmp(inst.op_name, "OR") == 0) res = (v1 || v2);

                // Constant folded to assignment: result = res
                inst.op = IR_ASSIGN;
                strcpy(inst.op_name, "ASSIGN");
                snprintf(inst.arg1, sizeof(inst.arg1), "%d", res);
                inst.arg2[0] = '\0';
                set_known_value(bindings, &binding_count, inst.result, res);

                if (stats) stats->constant_folds++;
            }
        } else if (inst.op == IR_ASSIGN) {
            int v;
            if (get_known_value(bindings, binding_count, inst.arg1, &v)) {
                set_known_value(bindings, &binding_count, inst.result, v);
            }
        }

        pass1[pass1_count++] = inst;
    }

    // Pass 2: Branch pruning & redundant jump elimination
    for (int i = 0; i < pass1_count; i++) {
        TACInstruction inst = pass1[i];

        // 1. If conditional jump on constant TRUE condition: IF_FALSE 1 GOTO L -> never jumps!
        if (inst.op == IR_IF_FALSE_GOTO) {
            int v;
            if (get_known_value(bindings, binding_count, inst.arg1, &v)) {
                if (v != 0) {
                    // Condition is always TRUE => jump is never taken
                    if (stats) stats->branches_pruned++;
                    continue; // Skip emitting IF_FALSE
                }
            }
        }

        // 2. Redundant GOTO L immediately preceding L:
        if (inst.op == IR_GOTO && (i + 1 < pass1_count)) {
            TACInstruction next = pass1[i + 1];
            if (next.op == IR_LABEL && strcmp(inst.result, next.result) == 0) {
                // Jumps directly to next instruction label -> redundant!
                if (stats) stats->dead_instructions_removed++;
                continue;
            }
        }

        // Copy instruction to output
        if (output->count >= output->capacity) {
            output->capacity *= 2;
            output->code = (TACInstruction*)realloc(output->code, sizeof(TACInstruction) * output->capacity);
        }
        output->code[output->count++] = inst;
    }

    output->temp_count = input->temp_count;
    output->label_count = input->label_count;
    free(pass1);
}

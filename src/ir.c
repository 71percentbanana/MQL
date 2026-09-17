#include "ir.h"

void ir_init(IRProgram* ir) {
    ir->capacity = 32;
    ir->count = 0;
    ir->temp_count = 0;
    ir->label_count = 0;
    ir->code = (TACInstruction*)malloc(sizeof(TACInstruction) * ir->capacity);
}

static void emit(IRProgram* ir, IROp op, const char* op_name, const char* a1, const char* a2, const char* res) {
    if (ir->count >= ir->capacity) {
        ir->capacity *= 2;
        ir->code = (TACInstruction*)realloc(ir->code, sizeof(TACInstruction) * ir->capacity);
    }
    TACInstruction* inst = &ir->code[ir->count++];
    inst->op = op;
    strncpy(inst->op_name, op_name ? op_name : "", sizeof(inst->op_name) - 1);
    strncpy(inst->arg1, a1 ? a1 : "", sizeof(inst->arg1) - 1);
    strncpy(inst->arg2, a2 ? a2 : "", sizeof(inst->arg2) - 1);
    strncpy(inst->result, res ? res : "", sizeof(inst->result) - 1);
}

static void gen_temp(IRProgram* ir, char* out_buf, size_t buf_size) {
    snprintf(out_buf, buf_size, "t%d", ++ir->temp_count);
}

static void gen_label(IRProgram* ir, char* out_buf, size_t buf_size) {
    snprintf(out_buf, buf_size, "L%d", ++ir->label_count);
}

static void gen_expr(IRProgram* ir, const ASTNode* node, char* out_place, size_t place_size) {
    if (!node) return;

    switch (node->type) {
        case AST_LITERAL_INT:
            snprintf(out_place, place_size, "%d", node->lit_int.value);
            break;

        case AST_LITERAL_STATE:
            snprintf(out_place, place_size, "%s", state_value_to_str(node->lit_state.state));
            break;

        case AST_IDENTIFIER:
            snprintf(out_place, place_size, "%s", node->ident.name);
            break;

        case AST_BINARY_OP: {
            char left_p[MAX_IDENT_LEN];
            char right_p[MAX_IDENT_LEN];
            gen_expr(ir, node->binary_op.left, left_p, sizeof(left_p));
            gen_expr(ir, node->binary_op.right, right_p, sizeof(right_p));

            gen_temp(ir, out_place, place_size);
            emit(ir, IR_BINARY_OP, operator_to_str(node->binary_op.op), left_p, right_p, out_place);
            break;
        }

        default:
            out_place[0] = '\0';
            break;
    }
}

static void gen_node(IRProgram* ir, const ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case AST_PROGRAM:
            for (int i = 0; i < node->program.stmt_count; i++) {
                gen_node(ir, node->program.statements[i]);
            }
            break;

        case AST_VAR_DECL:
            // Vehicles don't require TAC instruction, or can emit a comment/noop
            break;

        case AST_ASSIGN: {
            char val_place[MAX_IDENT_LEN];
            gen_expr(ir, node->assign.value, val_place, sizeof(val_place));
            emit(ir, IR_ASSIGN, "ASSIGN", val_place, "", node->assign.name);
            break;
        }

        case AST_ACTION:
            emit(ir, IR_ACTION, "ACTION", action_value_to_str(node->action.action), "", "");
            break;

        case AST_IF_STMT: {
            char cond_place[MAX_IDENT_LEN];
            gen_expr(ir, node->if_stmt.condition, cond_place, sizeof(cond_place));

            char l_else[16];
            gen_label(ir, l_else, sizeof(l_else));

            emit(ir, IR_IF_FALSE_GOTO, "IF_FALSE", cond_place, "", l_else);
            gen_node(ir, node->if_stmt.then_block);

            if (node->if_stmt.else_block) {
                char l_end[16];
                gen_label(ir, l_end, sizeof(l_end));
                emit(ir, IR_GOTO, "GOTO", "", "", l_end);
                emit(ir, IR_LABEL, "LABEL", "", "", l_else);
                gen_node(ir, node->if_stmt.else_block);
                emit(ir, IR_LABEL, "LABEL", "", "", l_end);
            } else {
                emit(ir, IR_LABEL, "LABEL", "", "", l_else);
            }
            break;
        }

        default:
            break;
    }
}

void ir_generate(IRProgram* ir, const ASTNode* root) {
    if (!root) return;
    gen_node(ir, root);
}

void ir_print_tac(const IRProgram* ir) {
    for (int i = 0; i < ir->count; i++) {
        const TACInstruction* inst = &ir->code[i];
        switch (inst->op) {
            case IR_LABEL:
                printf("%s:\n", inst->result);
                break;
            case IR_ASSIGN:
                printf("  %s = %s\n", inst->result, inst->arg1);
                break;
            case IR_BINARY_OP:
                printf("  %s = %s %s %s\n", inst->result, inst->arg1, inst->op_name, inst->arg2);
                break;
            case IR_IF_FALSE_GOTO:
                printf("  IF_FALSE %s GOTO %s\n", inst->arg1, inst->result);
                break;
            case IR_GOTO:
                printf("  GOTO %s\n", inst->result);
                break;
            case IR_ACTION:
                printf("  %s\n", inst->arg1);
                break;
        }
    }
}

void ir_print_quadruples(const IRProgram* ir) {
    printf("+----+---------------+---------------+---------------+---------------+\n");
    printf("| #  | Op            | Arg 1         | Arg 2         | Result        |\n");
    printf("+----+---------------+---------------+---------------+---------------+\n");

    for (int i = 0; i < ir->count; i++) {
        const TACInstruction* inst = &ir->code[i];
        printf("| %-2d | %-13s | %-13s | %-13s | %-13s |\n",
               i,
               inst->op_name,
               inst->arg1[0] ? inst->arg1 : "-",
               inst->arg2[0] ? inst->arg2 : "-",
               inst->result[0] ? inst->result : "-");
    }
    printf("+----+---------------+---------------+---------------+---------------+\n");
}

void ir_free(IRProgram* ir) {
    if (ir && ir->code) {
        free(ir->code);
        ir->code = NULL;
        ir->count = 0;
        ir->capacity = 0;
    }
}

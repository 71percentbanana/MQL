#include "common.h"

const char* token_type_to_str(TokenType type) {
    switch (type) {
        case TOKEN_EOF: return "EOF";
        case TOKEN_ERROR: return "ERROR";
        case TOKEN_KW_VEHICLE: return "KEYWORD(VEHICLE)";
        case TOKEN_KW_IF: return "KEYWORD(IF)";
        case TOKEN_KW_ELSE: return "KEYWORD(ELSE)";
        case TOKEN_KW_AND: return "KEYWORD(AND)";
        case TOKEN_KW_OR: return "KEYWORD(OR)";
        case TOKEN_ACT_ISSUE_FINE: return "ACTION(ISSUE_FINE)";
        case TOKEN_ACT_FLAG_VIOLATION: return "ACTION(FLAG_VIOLATION)";
        case TOKEN_ACT_ALLOW: return "ACTION(ALLOW)";
        case TOKEN_STATE_RED: return "STATE(RED)";
        case TOKEN_STATE_GREEN: return "STATE(GREEN)";
        case TOKEN_STATE_YELLOW: return "STATE(YELLOW)";
        case TOKEN_STATE_CROSSING: return "STATE(CROSSING)";
        case TOKEN_STATE_ROAD: return "STATE(ROAD)";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_ASSIGN: return "ASSIGN(=)";
        case TOKEN_OP_GT: return "OPERATOR(>)";
        case TOKEN_OP_LT: return "OPERATOR(<)";
        case TOKEN_OP_GTE: return "OPERATOR(>=)";
        case TOKEN_OP_LTE: return "OPERATOR(<=)";
        case TOKEN_OP_EQ: return "OPERATOR(==)";
        case TOKEN_OP_NEQ: return "OPERATOR(!=)";
        case TOKEN_LBRACE: return "DELIMITER({)";
        case TOKEN_RBRACE: return "DELIMITER(})";
        default: return "UNKNOWN";
    }
}

const char* data_type_to_str(DataType type) {
    switch (type) {
        case TYPE_INT: return "integer";
        case TYPE_STATE: return "state";
        case TYPE_VEHICLE: return "vehicle";
        case TYPE_ACTION: return "action";
        case TYPE_BOOL: return "boolean";
        default: return "unknown";
    }
}

const char* state_value_to_str(StateValue state) {
    switch (state) {
        case STATE_RED: return "RED";
        case STATE_GREEN: return "GREEN";
        case STATE_YELLOW: return "YELLOW";
        case STATE_CROSSING: return "CROSSING";
        case STATE_ROAD: return "ROAD";
        default: return "NONE";
    }
}

const char* action_value_to_str(ActionValue action) {
    switch (action) {
        case ACTION_ISSUE_FINE: return "ISSUE_FINE";
        case ACTION_FLAG_VIOLATION: return "FLAG_VIOLATION";
        case ACTION_ALLOW: return "ALLOW";
        default: return "NONE";
    }
}

const char* operator_to_str(OperatorType op) {
    switch (op) {
        case OP_GT: return ">";
        case OP_LT: return "<";
        case OP_GTE: return ">=";
        case OP_LTE: return "<=";
        case OP_EQ: return "==";
        case OP_NEQ: return "!=";
        case OP_AND: return "AND";
        case OP_OR: return "OR";
        default: return "NONE";
    }
}

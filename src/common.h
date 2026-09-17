#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_LEXEME_LEN 64
#define MAX_IDENT_LEN  64
#define MAX_CHILDREN   128

typedef enum {
    TOKEN_EOF,
    TOKEN_ERROR,

    // Keywords
    TOKEN_KW_VEHICLE,
    TOKEN_KW_IF,
    TOKEN_KW_ELSE,
    TOKEN_KW_AND,
    TOKEN_KW_OR,

    // Actions
    TOKEN_ACT_ISSUE_FINE,
    TOKEN_ACT_FLAG_VIOLATION,
    TOKEN_ACT_ALLOW,

    // Traffic States
    TOKEN_STATE_RED,
    TOKEN_STATE_GREEN,
    TOKEN_STATE_YELLOW,
    TOKEN_STATE_CROSSING,
    TOKEN_STATE_ROAD,

    // Identifiers and literals
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,

    // Operators
    TOKEN_ASSIGN,       // =
    TOKEN_OP_GT,        // >
    TOKEN_OP_LT,        // <
    TOKEN_OP_GTE,       // >=
    TOKEN_OP_LTE,       // <=
    TOKEN_OP_EQ,        // ==
    TOKEN_OP_NEQ,       // !=

    // Delimiters
    TOKEN_LBRACE,       // {
    TOKEN_RBRACE        // }
} TokenType;

typedef enum {
    TYPE_UNKNOWN,
    TYPE_INT,
    TYPE_STATE,
    TYPE_VEHICLE,
    TYPE_ACTION,
    TYPE_BOOL
} DataType;

typedef enum {
    STATE_NONE,
    STATE_RED,
    STATE_GREEN,
    STATE_YELLOW,
    STATE_CROSSING,
    STATE_ROAD
} StateValue;

typedef enum {
    ACTION_NONE,
    ACTION_ISSUE_FINE,
    ACTION_FLAG_VIOLATION,
    ACTION_ALLOW
} ActionValue;

typedef enum {
    OP_NONE,
    OP_GT,
    OP_LT,
    OP_GTE,
    OP_LTE,
    OP_EQ,
    OP_NEQ,
    OP_AND,
    OP_OR
} OperatorType;

typedef struct {
    TokenType type;
    char lexeme[MAX_LEXEME_LEN];
    int int_val;
    int line;
    int col;
} Token;

// Utility functions for string conversions
const char* token_type_to_str(TokenType type);
const char* data_type_to_str(DataType type);
const char* state_value_to_str(StateValue state);
const char* action_value_to_str(ActionValue action);
const char* operator_to_str(OperatorType op);

#endif // COMMON_H

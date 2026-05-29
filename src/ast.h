#pragma once

#include <stdint.h>

typedef enum {
    NODE_PROGRAM,
    NODE_FUNCTION,
    NODE_RETURN,
    NODE_INT_LIT,
    NODE_UNARY,
    NODE_BINARY,
} NodeKind;

typedef enum {
    UNARY_NEGATE,      // -
    UNARY_BITWISE_NOT, // ~
    UNARY_LOGICAL_NOT, // !
} UnaryOp;

typedef enum {
    BINARY_ADD,
    BINARY_SUBTRACT,
    BINARY_MULTIPLY,
    BINARY_DIVIDE,
    BINARY_REMAINDER,
} BinaryOp;

typedef struct Node {
    NodeKind kind;
    union {
        struct {
            struct Node *function;
        } program;
        struct {
            char *name; // name is strndup'd
            struct Node *body;
        } function;
        struct {
            struct Node *value;
        } ret;
        struct {
            int64_t value;
        } int_lit;
        struct {
            UnaryOp op;
            struct Node *operand;
        } unary;
        struct {
            BinaryOp op;
            struct Node *left_operand;
            struct Node *right_operand;
        } binary;
    } as;
} Node;

void print_ast(const Node *node);
void ast_free(Node *node);

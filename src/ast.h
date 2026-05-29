#pragma once

#include <stdint.h>

typedef enum {
    NODE_PROGRAM,
    NODE_FUNCTION,
    NODE_RETURN,
    NODE_INT_LIT,
    NODE_UNARY,
} NodeKind;

typedef enum {
    UNARY_NEGATE,      // -
    UNARY_BITWISE_NOT, // ~
    UNARY_LOGICAL_NOT, // !
} UnaryOp;

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
    } as;
} Node;

void print_ast(const Node *node);
void ast_free(Node *node);

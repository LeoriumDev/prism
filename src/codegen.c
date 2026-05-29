#include "codegen.h"
#include <assert.h>
#include <stdio.h>

static void emit(Node *node, FILE *f);

int codegen(Node *node, const char *path) {
    if (!node || !path)
        return -1;

    FILE *f = fopen(path, "w");
    if (!f)
        return -1;

    emit(node, f);

    fclose(f);
    return 0;
}

static void emit(Node *node, FILE *f) {
    switch (node->kind) {
    case NODE_PROGRAM:
        emit(node->as.program.function, f);
        break;
    case NODE_FUNCTION:
        fprintf(f, "    .text\n");
        fprintf(f, "    .globl %s\n", node->as.function.name);
        fprintf(f, "%s:\n", node->as.function.name);
        emit(node->as.function.body, f);
        break;
    case NODE_RETURN:
        emit(node->as.ret.value, f);
        fprintf(f, "    ret\n");
        break;
    case NODE_INT_LIT:
        fprintf(f, "    li a0, %lld\n", (long long)node->as.int_lit.value);
        break;
    case NODE_UNARY:
        emit(node->as.unary.operand, f);
        switch (node->as.unary.op) {
        case UNARY_NEGATE:
            fprintf(f, "    neg a0, a0\n");
            break;
        case UNARY_BITWISE_NOT:
            fprintf(f, "    not a0, a0\n");
            break;
        case UNARY_LOGICAL_NOT:
            fprintf(f, "    seqz a0, a0\n");
            break;
        }
        break;
    case NODE_BINARY:
        emit(node->as.binary.left_operand, f);
        fprintf(f, "    addi sp, sp, -16\n");
        fprintf(f, "    sd a0, 0(sp)\n");
        emit(node->as.binary.right_operand, f);
        fprintf(f, "    ld t0, 0(sp)\n");
        fprintf(f, "    addi sp, sp, 16\n");
        switch (node->as.binary.op) {
        case BINARY_ADD:
            fprintf(f, "    add a0, t0, a0\n");
            break;
        case BINARY_SUBTRACT:
            fprintf(f, "    sub a0, t0, a0\n");
            break;
        case BINARY_MULTIPLY:
            fprintf(f, "    mul a0, t0, a0\n");
            break;
        case BINARY_DIVIDE:
            fprintf(f, "    div a0, t0, a0\n");
            break;
        case BINARY_REMAINDER:
            fprintf(f, "    rem a0, t0, a0\n");
            break;
        }
        break;
    default:
        fprintf(stderr, "codegen: unhandled node kind %d\n", node->kind);
        assert(0);
    }
}

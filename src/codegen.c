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
    default:
        fprintf(stderr, "codegen: unhandled node kind %d\n", node->kind);
        assert(0);
    }
}
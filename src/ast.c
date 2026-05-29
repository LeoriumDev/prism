#include "ast.h"
#include <stdio.h>
#include <stdlib.h>

static void print_node(const Node *node, int depth) {
    for (int i = 1; i < depth; i++)
        printf("   ");
    if (depth > 0)
        printf("└─ ");

    switch (node->kind) {
    case NODE_PROGRAM:
        printf("Program\n");
        print_node(node->as.program.function, depth + 1);
        break;
    case NODE_FUNCTION:
        printf("Function name=\"%s\"\n", node->as.function.name);
        print_node(node->as.function.body, depth + 1);
        break;
    case NODE_RETURN:
        printf("Return\n");
        print_node(node->as.ret.value, depth + 1);
        break;
    case NODE_INT_LIT:
        printf("IntLit value=%lld\n", (long long)node->as.int_lit.value);
        break;
    case NODE_UNARY:
        switch (node->as.unary.op) {
        case UNARY_NEGATE:
            printf("Unary op=NEGATE\n");
            break;
        case UNARY_BITWISE_NOT:
            printf("Unary op=BITWISE_NOT\n");
            break;
        case UNARY_LOGICAL_NOT:
            printf("Unary op=LOGICAL_NOT\n");
            break;
        };
        print_node(node->as.unary.operand, depth + 1);
        break;
    }
}

void print_ast(const Node *node) {
    print_node(node, 0);
}

void ast_free(Node *node) {
    if (!node)
        return;

    switch (node->kind) {
    case NODE_PROGRAM:
        ast_free(node->as.program.function);
        free(node);
        break;
    case NODE_FUNCTION:
        free(node->as.function.name);
        ast_free(node->as.function.body);
        free(node);
        break;
    case NODE_RETURN:
        ast_free(node->as.ret.value);
        free(node);
        break;
    case NODE_INT_LIT:
        free(node);
        break;
    case NODE_UNARY:
        ast_free(node->as.unary.operand);
        free(node);
        break;
    }
}

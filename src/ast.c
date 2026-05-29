#include "ast.h"
#include <stdio.h>
#include <stdlib.h>

static const struct {
    const char *word;
    BinaryOp op;
} binary_ops[] = {
    {"ADD", BINARY_ADD},       {"SUBTRACT", BINARY_SUBTRACT},   {"MULTIPLY", BINARY_MULTIPLY},
    {"DIVIDE", BINARY_DIVIDE}, {"REMAINDER", BINARY_REMAINDER},
};

static void print_node(const Node *node, char *prefix, bool is_last, char *label) {
    printf("%s%s%s", prefix, is_last ? "└── " : "├── ", label);
    char child_prefix[256];
    const char *name;
    snprintf(child_prefix, sizeof(child_prefix), "%s%s", prefix, is_last ? "    " : "│   ");

    switch (node->kind) {
    case NODE_PROGRAM:
        printf("Program\n");
        print_node(node->as.program.function, child_prefix, true, "");
        break;
    case NODE_FUNCTION:
        printf("Function name=\"%s\"\n", node->as.function.name);
        print_node(node->as.function.body, child_prefix, true, "");
        break;
    case NODE_RETURN:
        printf("Return\n");
        print_node(node->as.ret.value, child_prefix, true, "");
        break;
    case NODE_INT_LIT:
        printf("IntLit value=%lld\n", (long long)node->as.int_lit.value);
        break;
    case NODE_UNARY:
        name = (node->as.unary.op == UNARY_NEGATE)        ? "NEGATE"
               : (node->as.unary.op == UNARY_BITWISE_NOT) ? "BITWISE_NOT"
                                                          : "LOGICAL_NOT";
        printf("Unary op=%s\n", name);
        print_node(node->as.unary.operand, child_prefix, true, "");
        break;
    case NODE_BINARY:
        name = binary_ops[node->as.binary.op].word;
        printf("Binary op=%s\n", name);
        print_node(node->as.binary.left_operand, child_prefix, false, "left: ");
        print_node(node->as.binary.right_operand, child_prefix, true, "right: ");
        break;
    }
}

void print_ast(const Node *node) {
    print_node(node, "", true, "");
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
    case NODE_BINARY:
        ast_free(node->as.binary.left_operand);
        ast_free(node->as.binary.right_operand);
        free(node);
        break;
    }
}

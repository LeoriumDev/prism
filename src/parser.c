#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* grammar:
 * program   := function
 * function  := "int" IDENT "(" "void" ")" "{" statement "}"
 * statement := "return" INT_LIT ";"
 */

static const char *token_display_names[TOKEN_COUNT] = {
    "int", "void", "return", "identifier", "integer literal", "(", ")", "{", "}", ";", "EOF",
};

static Token expect(Parser *parser, TokenType type) {
    if (parser->pos >= parser->tokens->count) {
        parser->hadError = true;
        return (Token){0};
    }
    if (parser->tokens->data[parser->pos].type == type) {
        return parser->tokens->data[parser->pos++];
    } else {
        fprintf(stderr, "[%zu:%zu] error: expected '%s', got '%s'\n",
                parser->tokens->data[parser->pos].pos.line,
                parser->tokens->data[parser->pos].pos.col, token_display_names[type],
                token_display_names[parser->tokens->data[parser->pos].type]);
        parser->hadError = true;
        return (Token){0};
    }
}

// statement := "return" INT_LIT ";"
static Node *parse_statement(Parser *parser) {
    expect(parser, KW_RETURN);
    Token int_lit_tok = expect(parser, INT_LIT);
    expect(parser, SEMICOLON);
    if (parser->hadError)
        return NULL;

    int64_t value = int_lit_tok.value.int_val;
    Node *ret_node = malloc(sizeof(*ret_node));
    Node *int_lit_node = malloc(sizeof(*int_lit_node));
    if (!ret_node || !int_lit_node)
        return NULL;

    ret_node->kind = NODE_RETURN;
    ret_node->as.ret.value = int_lit_node;
    int_lit_node->kind = NODE_INT_LIT;
    int_lit_node->as.int_lit.value = value;
    return ret_node;
}

// function := "int" IDENT "(" "void" ")" "{" statement "}"
static Node *parse_function(Parser *parser) {
    expect(parser, KW_INT);
    Token name_tok = expect(parser, IDENT);
    expect(parser, LPAREN);
    expect(parser, KW_VOID);
    expect(parser, RPAREN);
    expect(parser, LBRACE);
    Node *statement_node = parse_statement(parser);
    expect(parser, RBRACE);
    if (parser->hadError)
        return NULL;

    char *name = strdup(name_tok.value.str);
    if (!name)
        return NULL;

    Node *func_node = malloc(sizeof(*func_node));
    if (!func_node)
        return NULL;

    func_node->kind = NODE_FUNCTION;
    func_node->as.function.name = name;
    func_node->as.function.body = statement_node;
    return func_node;
}

// program := function
static Node *parse_program(Parser *parser) {
    Node *func_node = parse_function(parser);
    if (!func_node)
        return NULL;

    Node *prog_node = malloc(sizeof(*prog_node));
    if (!prog_node)
        return NULL;

    prog_node->kind = NODE_PROGRAM;
    prog_node->as.program.function = func_node;
    return prog_node;
}

Node *parse(TokenArray *tokens) {
    Parser parser = {
        .tokens = tokens,
        .hadError = false,
        .pos = 0,
    };
    return parse_program(&parser);
}

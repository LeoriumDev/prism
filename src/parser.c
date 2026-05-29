#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* grammar:
 * program    := function
 * function   := "int" IDENT "(" "void" ")" "{" statement "}"
 * statement  := "return" expression ";"
 * expression := term { ("+" | "-") term }
 * term       := factor { ("*" | "/" | "%") factor }
 * factor     := unary_op factor | INT_LIT
 * unary_op   := "-" | "~" | "!"
 */

static const char *token_display_names[TOKEN_COUNT] = {
    "invalid token", "'int'", "'void'", "'return'", "identifier", "integer literal",
    "'('",           "')'",   "'{'",    "'}'",      "';'",        "'+'",
    "'-'",           "'~'",   "'!'",    "'*'",      "'/'",        "'%'",
    "end of file",
};

static Token advance(Parser *parser) {
    if (parser->hadError || parser->pos >= parser->tokens->count) {
        parser->hadError = true;
        return (Token){.type = TOKEN_INVALID};
    }
    return parser->tokens->data[parser->pos++];
}

static Token peek(Parser *parser) {
    if (parser->hadError || parser->pos >= parser->tokens->count)
        return (Token){.type = TOKEN_INVALID};
    return parser->tokens->data[parser->pos];
}

static Token expect(Parser *parser, TokenType type) {
    if (parser->hadError || type == TOKEN_INVALID || parser->pos >= parser->tokens->count) {
        parser->hadError = true;
        return (Token){.type = TOKEN_INVALID};
    }
    Token tok = parser->tokens->data[parser->pos];
    if (tok.type == type) {
        parser->pos++;
        return tok;
    } else {
        fprintf(stderr, "[%zu:%zu] error: expected %s, got %s\n", tok.pos.line, tok.pos.col,
                token_display_names[type], token_display_names[tok.type]);
        parser->hadError = true;
        return (Token){.type = TOKEN_INVALID};
    }
}

// factor := unary_op factor | INT_LIT
static Node *parse_factor(Parser *parser) {
    if (parser->hadError)
        return NULL;

    Token tok = peek(parser);
    if (tok.type == MINUS || tok.type == TILDE || tok.type == BANG) {
        UnaryOp op;
        if (tok.type == MINUS) {
            expect(parser, MINUS);
            op = UNARY_NEGATE;
        } else if (tok.type == TILDE) {
            expect(parser, TILDE);
            op = UNARY_BITWISE_NOT;
        } else {
            expect(parser, BANG);
            op = UNARY_LOGICAL_NOT;
        }
        if (parser->hadError)
            return NULL;

        Node *factor_node = malloc(sizeof(*factor_node));
        if (!factor_node)
            return NULL;

        factor_node->kind = NODE_UNARY;
        factor_node->as.unary.op = op;
        factor_node->as.unary.operand = parse_factor(parser);
        if (!factor_node->as.unary.operand) {
            free(factor_node);
            return NULL;
        }
        return factor_node;
    } else if (tok.type == INT_LIT) {
        Token int_lit_tok = expect(parser, INT_LIT);
        if (parser->hadError)
            return NULL;

        int64_t value = int_lit_tok.value.int_val;
        Node *int_lit_node = malloc(sizeof(*int_lit_node));
        if (!int_lit_node)
            return NULL;

        int_lit_node->kind = NODE_INT_LIT;
        int_lit_node->as.int_lit.value = value;
        return int_lit_node;
    } else {
        parser->hadError = true;
        fprintf(stderr, "[%zu:%zu] error: expected expression, got %s\n", tok.pos.line, tok.pos.col,
                token_display_names[tok.type]);
        return NULL;
    }
}

// term := factor { ("*" | "/" | "%") factor }
static Node *parse_term(Parser *parser) {
    if (parser->hadError)
        return NULL;

    Node *left = parse_factor(parser);
    if (!left)
        return NULL;

    Token next = peek(parser);
    while (next.type == STAR || next.type == SLASH || next.type == PERCENT) {
        Token op_tok = advance(parser);
        Node *right = parse_factor(parser);
        if (!right) {
            ast_free(left);
            return NULL;
        }

        Node *binary_node = malloc(sizeof(*binary_node));
        if (!binary_node) {
            ast_free(left);
            ast_free(right);
            return NULL;
        }

        binary_node->kind = NODE_BINARY;
        binary_node->as.binary.op =
            (op_tok.type == STAR) ? BINARY_MULTIPLY
                                  : ((op_tok.type == SLASH) ? BINARY_DIVIDE : BINARY_REMAINDER);
        binary_node->as.binary.left_operand = left;
        binary_node->as.binary.right_operand = right;
        left = binary_node;
        next = peek(parser);
    }
    return left;
}

// expression := term { ("+" | "-") term }
static Node *parse_expression(Parser *parser) {
    if (parser->hadError)
        return NULL;

    Node *left = parse_term(parser);
    if (!left)
        return NULL;

    Token next = peek(parser);
    while (next.type == PLUS || next.type == MINUS) {
        Token op_tok = advance(parser);
        Node *right = parse_term(parser);
        if (!right) {
            ast_free(left);
            return NULL;
        }

        Node *binary_node = malloc(sizeof(*binary_node));
        if (!binary_node) {
            ast_free(left);
            ast_free(right);
            return NULL;
        }

        binary_node->kind = NODE_BINARY;
        binary_node->as.binary.op = (op_tok.type == PLUS) ? BINARY_ADD : BINARY_SUBTRACT;
        binary_node->as.binary.left_operand = left;
        binary_node->as.binary.right_operand = right;
        left = binary_node;
        next = peek(parser);
    }
    return left;
}

// statement  := "return" expression ";"
static Node *parse_statement(Parser *parser) {
    expect(parser, KW_RETURN);
    if (parser->hadError)
        return NULL;

    Node *expr_node = parse_expression(parser);
    if (!expr_node)
        return NULL;

    expect(parser, SEMICOLON);
    if (parser->hadError) {
        ast_free(expr_node);
        return NULL;
    }

    Node *ret_node = malloc(sizeof(*ret_node));
    if (!ret_node)
        return NULL;

    ret_node->kind = NODE_RETURN;
    ret_node->as.ret.value = expr_node;
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
    if (parser->hadError) {
        ast_free(statement_node);
        return NULL;
    }

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

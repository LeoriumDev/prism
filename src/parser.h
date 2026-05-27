#pragma once

#include "ast.h"
#include "lexer.h"
#include <stddef.h>

typedef struct {
    TokenArray *tokens;
    size_t pos;
    bool hadError;
} Parser;

Node *parse(TokenArray *tokens);

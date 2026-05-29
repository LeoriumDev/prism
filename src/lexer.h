#pragma once

#include <stddef.h>
#include <stdint.h>

#define INIT_TOKEN_ARRAY_CAP 8
#define TOKEN_ARRAY_GROWTH 2

typedef enum {
    TOKEN_INVALID = 0,

    // Keywords
    KW_INT,
    KW_VOID,
    KW_RETURN,

    // Identifier
    IDENT,

    // Literals
    INT_LIT,

    // Punctuation
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    SEMICOLON,

    // Operators
    PLUS,
    MINUS,
    TILDE,
    BANG,

    // EOF
    END_OF_FILE,

    // Token count
    TOKEN_COUNT,
} TokenType;

typedef union {
    int64_t int_val; // for INT_LIT
    // TokenValue owns a copy of the input string to avoid use-after-free.
    char *str; // for IDENT
} TokenValue;

typedef struct {
    size_t line;
    size_t col;
} TokenPos;

typedef struct {
    TokenType type;
    TokenValue value;
    TokenPos pos;
} Token;

typedef struct {
    Token *data;
    size_t count;
    size_t capacity;
} TokenArray;

// Tokenizes src into a heap-allocated TokenArray. Returns NULL on error.
TokenArray *tokenize(const char *src);

// Initializes the array
int token_array_init(TokenArray *arr);

// Pushes Token to the array
void token_array_push(TokenArray *arr, Token tok);

// Frees the array, all token strings it owns, and the struct itself.
void token_array_free(TokenArray *arr);

// Prints the entire TokenArray for debugging.
void token_array_print(const TokenArray *arr);

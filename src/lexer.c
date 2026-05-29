#include "lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *token_type_names[TOKEN_COUNT] = {
    "KW_INT", "KW_VOID",   "KW_RETURN", "IDENT", "INT_LIT", "LPAREN", "RPAREN",      "LBRACE",
    "RBRACE", "SEMICOLON", "PLUS",      "MINUS", "TILDE",   "BANG",   "END_OF_FILE",
};

static const struct {
    const char *word;
    TokenType type;
} keywords[] = {
    {"int", KW_INT},
    {"void", KW_VOID},
    {"return", KW_RETURN},
};

inline static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

inline static bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

TokenArray *tokenize(const char *src) {
    TokenArray *arr = malloc(sizeof(*arr));
    if (!arr)
        return NULL;

    if (token_array_init(arr)) {
        free(arr);
        return NULL;
    }

    size_t line = 1;
    size_t col = 1;
    const char *p = src;
    while (*p != '\0') {
        switch (*p) {
        // whitespace characters
        case ' ':
        case '\r': {
            col++;
            p++;
            break;
        }
        case '\t': {
            col += 4;
            p++;
            break;
        }
        case '\n': {
            line++;
            col = 1;
            p++;
            break;
        }

        // comment
        case '/': {
            if (*(p + 1) == '/') {
                while (*p != '\n' && *p != '\0') {
                    p++;
                    col++;
                }
            } else if (*(p + 1) == '*') {
                while (!(*p == '*' && *(p + 1) == '/') && *p != '\0') {
                    if (*p == '\n') {
                        line++;
                        col = 1;
                        p++;
                        continue;
                    }
                    p++;
                    col++;
                }
                if (*p != '\0') {
                    p += 2;
                    col += 2;
                }
            } else {
                token_array_free(arr);
                fprintf(stderr, "Not supported character: %c\n", *p);
                return NULL;
            }
            break;
        }

        // punctuation
        case '(': {
            Token tok = {
                .type = LPAREN,
                .value = {0},
                .pos = {.col = col++, .line = line},
            };
            token_array_push(arr, tok);
            p++;
            break;
        }
        case ')': {
            Token tok = {
                .type = RPAREN,
                .value = {0},
                .pos = {.col = col++, .line = line},
            };
            token_array_push(arr, tok);
            p++;
            break;
        }
        case '{': {
            Token tok = {
                .type = LBRACE,
                .value = {0},
                .pos = {.col = col++, .line = line},
            };
            token_array_push(arr, tok);
            p++;
            break;
        }
        case '}': {
            Token tok = {
                .type = RBRACE,
                .value = {0},
                .pos = {.col = col++, .line = line},
            };
            token_array_push(arr, tok);
            p++;
            break;
        }
        case ';': {
            Token tok = {
                .type = SEMICOLON,
                .value = {0},
                .pos = {.col = col++, .line = line},
            };
            token_array_push(arr, tok);
            p++;
            break;
        }

        // operators
        case '+': {
            Token tok = {
                .type = PLUS,
                .value = {0},
                .pos = {.col = col++, .line = line},
            };
            token_array_push(arr, tok);
            p++;
            break;
        }
        case '-': {
            Token tok = {
                .type = MINUS,
                .value = {0},
                .pos = {.col = col++, .line = line},
            };
            token_array_push(arr, tok);
            p++;
            break;
        }
        case '~': {
            Token tok = {
                .type = TILDE,
                .value = {0},
                .pos = {.col = col++, .line = line},
            };
            token_array_push(arr, tok);
            p++;
            break;
        }
        case '!': {
            Token tok = {
                .type = BANG,
                .value = {0},
                .pos = {.col = col++, .line = line},
            };
            token_array_push(arr, tok);
            p++;
            break;
        }
        default: {
            if (is_digit(*p)) { // interger literals
                int64_t value = 0;
                size_t start_col = col;
                while (is_digit(*p)) {
                    if (value > (INT64_MAX - (*p - '0')) / 10)
                        break;
                    value = value * 10 + (*p - '0');
                    p++;
                    col++;
                }
                Token tok = {
                    .type = INT_LIT,
                    .value = {.int_val = value},
                    .pos = {.col = start_col, .line = line},
                };
                token_array_push(arr, tok);
                break;
            } else if (is_alpha(*p) || *p == '_') { // identifiers
                const char *start_pos = p;
                size_t start_col = col;
                while (is_alpha(*p) || is_digit(*p) || *p == '_') {
                    p++;
                    col++;
                }

                size_t n_keywords = sizeof(keywords) / sizeof(keywords[0]);
                size_t len = p - start_pos;
                TokenType type = IDENT;
                for (size_t i = 0; i < n_keywords; i++) {
                    if (strlen(keywords[i].word) == len &&
                        strncmp(start_pos, keywords[i].word, len) == 0) {
                        type = keywords[i].type;
                        break;
                    }
                }

                Token tok = {
                    .type = type,
                    .value = {0},
                    .pos = {.col = start_col, .line = line},
                };
                if (type == IDENT) {
                    tok.value.str = strndup(start_pos, len);
                    if (!tok.value.str) {
                        token_array_free(arr);
                        return NULL;
                    }
                }
                token_array_push(arr, tok);
                break;
            } else {
                token_array_free(arr);
                fprintf(stderr, "Not supported character: %c\n", *p);
                return NULL;
            }
        }
        }
    }

    Token eof_token = {
        .type = END_OF_FILE,
        .value = {0},
        .pos = {.col = col, .line = line},
    };
    token_array_push(arr, eof_token);

    return arr;
}

int token_array_init(TokenArray *arr) {
    arr->capacity = INIT_TOKEN_ARRAY_CAP;
    arr->count = 0;
    arr->data = malloc(arr->capacity * sizeof(Token));
    if (!arr->data)
        return -1;
    return 0;
}

void token_array_push(TokenArray *arr, Token tok) {
    if (!arr)
        return;

    if (arr->count == arr->capacity) {
        arr->capacity *= TOKEN_ARRAY_GROWTH;
        Token *new_data = realloc(arr->data, arr->capacity * sizeof(Token));
        if (!new_data)
            return;
        arr->data = new_data;
    }

    arr->data[arr->count++] = tok;
}

void token_array_free(TokenArray *arr) {
    if (!arr)
        return;
    for (size_t i = 0; i < arr->count; i++) {
        if (arr->data[i].type == IDENT) {
            free(arr->data[i].value.str);
        }
    }
    free(arr->data);
    free(arr);
}

void token_array_print(const TokenArray *arr) {
    if (!arr)
        return;

    for (size_t i = 0; i < arr->count; i++) {
        Token tok = arr->data[i];

        char pos[32];
        snprintf(pos, sizeof(pos), "[%zu:%zu]", tok.pos.line, tok.pos.col);

        printf("%-8s ", pos);
        printf("%-12s", token_type_names[tok.type]);

        switch (tok.type) {
        case INT_LIT:
            printf(" %lld", (long long)tok.value.int_val);
            break;
        case IDENT:
            printf(" %s", tok.value.str);
            break;
        default:
            break;
        }

        printf("\n");
    }
}

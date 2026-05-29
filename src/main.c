#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen.h"
#include "lexer.h"
#include "parser.h"

typedef enum { EMIT_TOKENS, EMIT_AST, EMIT_ASM } EmitMode;

static char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        perror("fopen");
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (n < 0) {
        fclose(f);
        return NULL;
    }

    char *buf = malloc((size_t)n + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }

    size_t got = fread(buf, 1, (size_t)n, f);
    fclose(f);
    buf[got] = '\0';
    return buf;
}

static void print_usage(char **argv) {
    fprintf(stderr, "usage: %s [--emit=tokens|ast|asm] <input.c> [-o <output.S>]\n", argv[0]);
    fprintf(stderr, "  --emit=tokens   print tokens and stop\n");
    fprintf(stderr, "  --emit=ast      print AST and stop\n");
    fprintf(stderr, "  --emit=asm      (default) generate assembly to -o\n");
}

int main(int argc, char **argv) {
    // usage: prism [--emit=tokens|ast|asm] <input.c> [-o <output.S>]
    //   --emit=tokens   print tokens and stop
    //   --emit=ast      print AST and stop
    //   --emit=asm      (default) generate assembly to -o
    const char *in_path = NULL, *out_path = NULL;
    EmitMode mode = EMIT_ASM;
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--emit=", 7) == 0) {
            const char *value = argv[i] + 7;
            if (strcmp(value, "tokens") == 0) {
                mode = EMIT_TOKENS;
            } else if (strcmp(value, "ast") == 0) {
                mode = EMIT_AST;
            } else if (strcmp(value, "asm") == 0) {
                mode = EMIT_ASM;
            } else {
                print_usage(argv);
                return EXIT_FAILURE;
            }
        } else if (i + 1 < argc && argv[i][0] == '-' && argv[i][1] == 'o') {
            out_path = argv[++i];
        } else {
            in_path = argv[i];
        }
    }

    if (!in_path || (mode == EMIT_ASM && !out_path)) {
        print_usage(argv);
        return EXIT_FAILURE;
    }

    char *source = read_file(in_path);
    if (!source)
        return EXIT_FAILURE;

    TokenArray *toks = tokenize(source);
    if (!toks) {
        free(source);
        return EXIT_FAILURE;
    }

    if (mode == EMIT_TOKENS) {
        token_array_print(toks);
        free(source);
        token_array_free(toks);
        return EXIT_SUCCESS;
    }

    Node *ast = parse(toks);
    if (!ast) {
        free(source);
        token_array_free(toks);
        return EXIT_FAILURE;
    }

    if (mode == EMIT_AST) {
        print_ast(ast);
        free(source);
        token_array_free(toks);
        ast_free(ast);
        return EXIT_SUCCESS;
    }

    int status = codegen(ast, out_path);
    if (status == -1) {
        free(source);
        token_array_free(toks);
        ast_free(ast);
        return EXIT_FAILURE;
    }

    ast_free(ast);
    token_array_free(toks);
    free(source);
    return EXIT_SUCCESS;
}

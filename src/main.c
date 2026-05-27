#include <stdio.h>
#include <stdlib.h>

#include "codegen.h"
#include "lexer.h"
#include "parser.h"

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

int main(int argc, char **argv) {
    // usage: prism <input.c> -o <output.S>
    const char *in_path = NULL, *out_path = NULL;
    for (int i = 1; i < argc; i++) {
        if (i + 1 < argc && (argv[i][0] == '-' && argv[i][1] == 'o')) {
            out_path = argv[++i];
        } else {
            in_path = argv[i];
        }
    }
    if (!in_path || !out_path) {
        fprintf(stderr, "usage: %s <input.c> -o <output.S>\n", argv[0]);
        return 2;
    }

    char *source = read_file(in_path);
    if (!source)
        return 1;

    TokenArray *toks = tokenize(source);
    if (!toks) {
        free(source);
        return EXIT_FAILURE;
    }

    Node *ast = parse(toks);
    if (!ast) {
        free(source);
        token_array_free(toks);
        return EXIT_FAILURE;
    }

    print_ast(ast);
    ast_free(ast);
    token_array_free(toks);

    free(source);
    return 0;
}

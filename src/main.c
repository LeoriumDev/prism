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
    bool verbose = false;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-' && argv[i][1] == 'o' && i + 1 < argc) {
            out_path = argv[++i];
        } else if (argv[i][0] == '-' && argv[i][1] == 'v') {
            verbose = true;
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

    if (verbose)
        print_ast(ast);

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
    return 0;
}

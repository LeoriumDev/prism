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
    token_array_print(toks);
    token_array_free(toks);

    // ───── middle: the signatures of these three steps are YOUR design (§5–§7)
    // ───── expected behavior:
    //   1. lex(source)         → tokens (an array or a token stream)
    //   2. parse(tokens)       → AST (on failure: print error with line number,
    //   return non-zero)
    //   3. codegen(ast, out)   → write RISC-V assembly to out_path
    // any stage hitting invalid input: print "in_path:line: message", return
    // non-zero, never crash.
    //
    // TODO(you): wire these three steps against the function signatures you
    // design.
    (void)out_path;
    free(source);
    return 1;
    // ────────────────────────────────────────────────────────────────────────────
}
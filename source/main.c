#include <util.h>
#include <lexer.h>
#include <parser.h>

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

// read file into string
static char *read_str(char *path, size_t *len) {
    FILE *file = fopen(path, "r");
    if (!file)
        die_errno("failed to open file '%s'", path);

    // seek to the end of the file and ask for position to get file size
    fseek(file, 0, SEEK_END);
    *len = ftell(file);
    rewind(file);

    char *buf = malloc(*len);

    if (fread(buf, 1, *len, file) != *len)
        die_errno("failed to read file '%s'", path);

    fclose(file);

    return buf;
}

static void lookup_lc(char *src, size_t pos, size_t *l, size_t *c) {
    size_t line = 1;
    size_t col = 1;

    for (size_t i = 0; i <= pos; i++) {
        if (src[i] == '\n') line++;
        else                col++;
    }

    *l = line;
    *c = col;
}

static void print_atom_inner(atom_t a, size_t *ind) {
    for (size_t i = 0; i < *ind; i++)
        putc(' ', stdout);

    printf(" * ");

    switch (a.kind) {
        case ATOM_SYMBOL: printf("SYMBOL '%s'\n", a.str_val); break;
        case ATOM_STRING: printf("STRING '%s'\n", a.str_val); break;
        case ATOM_INT: printf("INT '%ld'\n", a.int_val); break;

        case ATOM_BATCH:
        case ATOM_INLINE:
        case ATOM_LIST:
        case ATOM_CMD:
            printf("%s:\n", atom_kind_str(a.kind));
            *ind += 2;
            for (size_t i = 0; i < buf_len(a.list_val); i++)
                print_atom_inner(a.list_val[i], ind);
            *ind -= 2;
            break;

        default:
            die("failed on %d", a.kind);
            break;
    }
}

static void print_atom(atom_t a) {
    size_t ind = 0;
    print_atom_inner(a, &ind);
}

int main(void) {
    char *path = "tests/test.jcl";
    size_t len;
    char *src = read_str(path, &len);

    printf("len = %zu\n", len);

    // initialise lexer with source string
    lexer_t lx;
    lexer_init(&lx, src, len);

    // parse input
    atom_t at;
    parse_err_t err;
    if (!parse(&lx, &at, &err)) {
        size_t line, col;
        lookup_lc(lx.src, err.pos, &line, &col);

        switch (err.kind) {
            case PERR_UNEXPECTED_EOF:
                die("%s@%zu:%zu: unexpected eof", path, line, col);
                break;
            case PERR_EXPECTED_TOK:
                die(
                    "%s@%zu:%zu: expected token '%s', found '%s'",
                    path,
                    line, col,
                    token_kind_str(err.expected), token_kind_str(err.found)
                );
                break;
            case PERR_EXPECTED_ATOM:
                die(
                    "%s@%zu:%zu: expected atom '%s', found '%s'",
                    path,
                    line, col,
                    atom_kind_str(err.expected), atom_kind_str(err.found)
                );
                break;
            case PERR_UNCLOSED:
                die(
                    "%s@%zu:%zu: unclosed atom '%s'",
                    path,
                    line, col,
                    atom_kind_str(err.unclosed)
                );
                break;
        }
    }

    print_atom(at);

    free(src);

    return 0;
}

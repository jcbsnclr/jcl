#pragma once

#include <lexer.h>

#include <stdbool.h>

typedef struct atom {
    size_t start, end;
    enum {
        ATOM_NULL,
        ATOM_SYMBOL,
        ATOM_STRING,
        ATOM_INT,

        ATOM_TL,
        ATOM_BATCH,
        ATOM_INLINE,
        ATOM_LIST,
        ATOM_CMD
    } kind;

    union {
        struct atom *list_val;
        char *str_val;
        int64_t int_val;
    };
} atom_t;

typedef struct {
    size_t pos;
    enum {
        PERR_UNEXPECTED_EOF,
        PERR_EXPECTED_TOK,
        PERR_EXPECTED_ATOM,
        PERR_UNCLOSED
    } kind;

    union {
        struct {
            int expected;
            int found;
        };
        int unclosed;
    };
} parse_err_t;

bool parse(lexer_t *lx, atom_t *at, parse_err_t *err);

char *atom_kind_str(int kind);

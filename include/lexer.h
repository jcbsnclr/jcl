#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// a token categorises a piece of source
// single-char tokens (e.g. '{', '.', etc.) are stored verbatim in kind
typedef struct {
    enum {
        TOK_WS = 256,
        TOK_COMMENT,
        
        TOK_SYMBOL,
        TOK_INT,
        TOK_STRING
    } kind;

    size_t start, end;
} token_t;

typedef struct {
    char *src;
    size_t len, pos;
} lexer_t;

// string name of token kind
char *token_kind_str(int kind);

// initialise lexer
void lexer_init(lexer_t *lx, char *src, size_t len);
// get next token from lexer
// the lexer should not fail under any circumstances; that is deferred to the parser
bool lexer_next(lexer_t *lx, token_t *tok);

#include <lexer.h>
#include <util.h>

#include <ctype.h>

// function to filter chars in stream 
typedef bool (*cond_t)(char c);

// while the current char matches condition cond, increment token end
static void take_while(lexer_t *lx, token_t *tok, cond_t cond) {
    while (lx->pos < lx->len && cond(lx->src[lx->pos]))
        tok->end = ++lx->pos;
}

static void take_while_str(lexer_t *lx, token_t *tok) {
    bool complete = false;

    while (!complete && lx->pos < lx->len) {
        char c = lx->src[lx->pos++];
        tok->end = lx->pos;

        // skip char after '\', to allow escaping strings
        // invalid escape sequences will be caught later
        if (c == '\\')
            tok->end = ++lx->pos;

        complete = c == '"';
    }
}

/*
 * rules for the lexer
 */

static bool is_symbol(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') ||
            c == '_';
}

static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

static bool is_space(char c) {
    return c == ' '  || c == '\f' || c == '\n' ||
           c == '\r' || c == '\t' || c == '\v';
}

static bool not_nl(char c) {
    return c != '\n';
}

/*
 * implementation
 */
#define K(k) [TOK_ ## k - 256] = #k

static char *kinds[] = {
    K(WS), K(COMMENT), K(SYMBOL), K(INT), K(STRING)
};

// function to turn a token kind into a string
// for single char tokens (e.g. '{'), they are written to tkbuf at tkpos
// this will work fine so long as you don't need more than 16 concurrent single-char token strings :P
static char tkbuf[32];
static size_t tkpos = 0;
char *token_kind_str(int kind) {
    if (kind < TOK_WS) {
        // if we've used the whole buffer, reset to 0
        if (tkpos == 32)
            tkpos = 0;

        size_t cur = tkpos;

        tkbuf[cur] = kind;
        tkbuf[cur + 1] = '\0';
        tkpos += 2;

        return tkbuf + cur;
    }

    return kinds[kind - TOK_WS];
}

void lexer_init(lexer_t *lx, char *src, size_t len) {
    lx->src = src;
    lx->len = len;
    lx->pos = 0;
}

bool lexer_next(lexer_t *lx, token_t *tok) {
    // return false if eof reached
    if (lx->pos >= lx->len)
        return false;

    char c = lx->src[lx->pos];
    tok->start = lx->pos;

    switch (c) {
        // whitespace
        case ' ':
        case '\f':
        case '\n':
        case '\r':
        case '\t':
        case '\v':
            tok->kind = TOK_WS;
            take_while(lx, tok, is_space);
            break;

        // comment
        case '#':
            tok->kind = TOK_COMMENT;
            take_while(lx, tok, not_nl);
            break;
            
        // symbol
        case 'a' ... 'z':
        case 'A' ... 'Z':
        case '_':
            tok->kind = TOK_SYMBOL;
            take_while(lx, tok, is_symbol);
            break;

        // integer
        case '0' ... '9':
            tok->kind = TOK_INT;
            take_while(lx, tok, is_digit);
            break;

        // string literal
        case '"':
            tok->kind = TOK_STRING;
            tok->end = ++lx->pos;
            take_while_str(lx, tok);
            break;

        // if uncaught by other cases, token is single char and stored verbatim
        default:
            tok->kind = c;
            tok->end = ++lx->pos;
            break;
    }

    return true;
}

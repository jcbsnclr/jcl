#include <parser.h>
#include <lexer.h>
#include <util.h>

#include <stdio.h>
#include <string.h>

#define TRY(e) \
    if (!(e)) return false

typedef struct {
    token_t orig;
    int kind;
    atom_t *body;
} pframe_t;

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
    }
}

static void pstack_print(pframe_t **st) {
    puts("pstack:");
    size_t ind = 4;

    for (size_t i = 0; i < buf_len(*st); i++) {
        printf("  frame '%s':\n", atom_kind_str((*st)[i].kind));
        
        for (size_t j = 0; j < buf_len((*st)[i].body); j++)
            print_atom_inner((*st)[i].body[j], &ind);
    }
}

static bool pstack_push(pframe_t **st, atom_t a, parse_err_t *err) {
    pframe_t *f = buf_top(*st);
    if (!f)
        die("pstack exhausted");

    switch (f->kind) {
        case ATOM_BATCH:
        case ATOM_INLINE:
            if (a.kind != ATOM_CMD) {
                err->pos = a.start;
                err->kind = PERR_EXPECTED_ATOM;
                err->expected = ATOM_CMD;
                err->found = a.kind;
                return false;
            }

            if (buf_len(a.list_val) != 0)
                buf_push(f->body, a);
            break;

        case ATOM_CMD:
            if (a.kind == ATOM_CMD) {
                err->pos = a.start;
                err->kind = PERR_EXPECTED_ATOM;
                err->expected = ATOM_SYMBOL;    // TODO: fix, add "any" symbol or other error type
                err->found = a.kind;
                return false;
            }
            // fall through
        case ATOM_LIST:
            buf_push(f->body, a);
            break;

        default:
            die("pstack_push defaulted on '%s' with '%s' (%zu->%zu)", atom_kind_str(f->kind), atom_kind_str(a.kind), a.start, a.end);
            break;
    }

    return true;
}

static void pstack_open(pframe_t **st, int kind, token_t orig) {
    pframe_t f;

    f.orig = orig;
    f.kind = kind;
    f.body = NULL;

    buf_push(*st, f);
}

static bool pstack_close(pframe_t **st, int kind, token_t orig, parse_err_t *err) {
    pframe_t f;
    if (!buf_pop(*st, f))
        die("pstack exhausted");

    /*
    bool dclose = f.kind == ATOM_CMD && (kind == ATOM_BATCH || kind == ATOM_INLINE);
    */
    
    if (/*!dclose &&*/ f.kind != kind && f.kind) {
        err->pos = orig.start;
        err->kind = PERR_EXPECTED_TOK;
        err->expected = f.orig.kind;
        err->found = orig.kind;
        return false;
    }

    atom_t a;
    a.kind = f.kind;
    a.start = f.orig.start;
    a.end = orig.end;
    a.list_val = f.body;

    TRY(pstack_push(st, a, err));

    /*
    if (dclose)
        pstack_close(st, f.kind, orig, err);
    */
    
    return true;
}

static bool pstack_finish(pframe_t **st, lexer_t *lx, atom_t *at, parse_err_t *err) {
    pframe_t f;
    if (!buf_pop(*st, f))
        die("pstack exhausted");

    if (buf_len(*st) != 0) {
        err->pos = f.orig.start; 
        err->kind = PERR_UNCLOSED;
        err->unclosed = f.kind;
        return false;
    }

    if (f.kind != ATOM_BATCH)
       die("lol!");

    at->kind = ATOM_BATCH;
    at->start = f.orig.start;
    at->end = lx->len;
    at->list_val = f.body;

    return true;
}

static bool token_to_atom(lexer_t *lx, token_t tok, atom_t *at, parse_err_t *err) {
    (void)err;

    switch (tok.kind) {
        case TOK_INT:
            at->kind = ATOM_INT;
            at->start = tok.start;
            at->end = tok.end;
            at->int_val = 0;
            for (size_t i = tok.start; i < tok.end; i++) {
                at->int_val *= 10;
                at->int_val += lx->src[i] - '0';
            }
            break;

        case TOK_SYMBOL:
            at->kind = ATOM_SYMBOL;
            at->start = tok.start;
            at->end = tok.end;
            at->str_val = strndup(lx->src + tok.start, tok.end - tok.start);
            break;

        default:
            die("token_to_atom: unimplemented token '%s'", token_kind_str(tok.kind));
            break;
    }
    
    return true;
}

bool parse(lexer_t *lx, atom_t *at, parse_err_t *err) {
    token_t start;
    start.start = lx->pos;
    start.end = lx->pos + 1;

    pframe_t *st = NULL;
    pstack_open(&st, ATOM_BATCH, start);
    pstack_open(&st, ATOM_CMD, start);

    token_t tok;
    atom_t a;
    while (lexer_next(lx, &tok)) {
        switch (tok.kind) {
            case TOK_WS:
            case TOK_COMMENT:
                continue;

            case TOK_SYMBOL:
            case TOK_INT:
            case TOK_STRING:
                TRY(token_to_atom(lx, tok, &a, err));
                TRY(pstack_push(&st, a, err));
                break;

            case '(': pstack_open(&st, ATOM_LIST, tok); break;
            case '{': pstack_open(&st, ATOM_BATCH, tok); pstack_open(&st, ATOM_CMD, tok); break;
            case '[': pstack_open(&st, ATOM_INLINE, tok); pstack_open(&st, ATOM_CMD, tok); break;

            case ')': TRY(pstack_close(&st, ATOM_CMD, tok, err)); TRY(pstack_close(&st, ATOM_LIST, tok, err)); break;
            case '}': TRY(pstack_close(&st, ATOM_CMD, tok, err)); TRY(pstack_close(&st, ATOM_BATCH, tok, err)); break;
            case ']': TRY(pstack_close(&st, ATOM_CMD, tok, err)); TRY(pstack_close(&st, ATOM_INLINE, tok, err)); break;

            case ';':
                TRY(pstack_close(&st, ATOM_CMD, tok, err));
                pstack_open(&st, ATOM_CMD, tok);
                break;

            default: die("default");
        }        
        //pstack_print(&st);
        (void)pstack_print;
    }

    TRY(pstack_close(&st, ATOM_CMD, tok, err));
    TRY(pstack_finish(&st, lx, at, err));

    return true;
}

#define A(a) [ATOM_ ## a] = #a

static char *atom_kinds[] = {
    A(SYMBOL), A(STRING), A(INT),
    A(BATCH), A(INLINE), A(LIST), A(CMD)
};

char *atom_kind_str(int kind) {
    return atom_kinds[kind];
}

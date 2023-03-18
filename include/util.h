#pragma once

#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * stretchy buffer (à la stb)
 * 
 * stretchy buffers let us have a "generic" dynamic array container
 * to create a new buf, create a null ptr and push to it:
 *
 *   int *buf = NULL;
 *   buf_push(buf, 123);
 *
 * this will allocate 8 * int worth of storage by default, and will double
 * with each re-alloc
 */

typedef struct {
    size_t len, cap;
} buf_hdr_t;

// get a pointer to the buffer's header
#define buf_hdr(buf) \
    ((buf) ? (buf_hdr_t *)(buf) - 1 : NULL)

#define buf_len(buf) \
    ((buf) ? buf_hdr(buf)->len : 0)
#define buf_cap(buf) \
    ((buf) ? buf_hdr(buf)->cap : 0)
#define buf_full(buf) \
    ((buf) ? buf_len(buf) >= buf_cap(buf) : true)

// stack ops for buf
#define buf_push(buf, val) (\
    buf_full(buf) ? (buf) = buf_grow(buf, sizeof(*buf)) : 0, \
    (buf)[buf_hdr(buf)->len++] = val)
// pop returns true on success
#define buf_pop(buf, into) (\
    buf_len(buf) == 0 ? false : (into = (buf)[--buf_hdr(buf)->len], true))

#define buf_top(buf) \
    buf_len(buf) != 0 ? &(buf)[buf_len(buf) - 1] : NULL

// grows the buffer. by default it will double it's size each time to try and avoid future allocs
void *buf_grow(void *buf, size_t elem);
// deallocate buffer
void buf_free(void *buf);

/*
 * utils for reporting errors
 */

// report error message and exit with failure
void die(char *fmt, ...);
// report error message and errno string
void die_errno(char *fmt, ...);

#define die_assert(cond, ...) \
    if (!(cond)) \
        die("assertion '" #cond "' failed: " __VA_ARGS__)

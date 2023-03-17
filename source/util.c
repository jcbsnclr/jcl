#include <util.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

void *buf_grow(void *buf, size_t elem) {
    size_t len = buf_len(buf);
    // if buffer does not yet exist, give it default capacity of 8, else double old cap
    size_t new_cap = buf ? buf_cap(buf) * 2 : 8;

    buf_hdr_t *new_buf = malloc(elem * new_cap + sizeof(buf_hdr_t));
    void *data = (void *)(new_buf + 1);
    
    // copy over old data to new buffer
    if (buf)
        memcpy(data, buf, len * elem);

    // set up header
    new_buf->len = len;
    new_buf->cap = new_cap;

    buf_free(buf);

    return data;
}

void buf_free(void *buf) {
    if (buf)
        free(buf_hdr(buf));
}

void die(char *fmt, ...) {
    fprintf(stderr, "error: ");

    va_list args;
    va_start(args, fmt);

    vfprintf(stderr, fmt, args);
    fputc('\n', stderr);

    exit(EXIT_FAILURE);
}

void die_errno(char *fmt, ...) {
    int err = errno;

    fprintf(stderr, "error: ");

    va_list args;
    va_start(args, fmt);
    
    vfprintf(stderr, fmt, args);
    fprintf(stderr, ": %s", strerror(err));
    fputc('\n', stderr);

    exit(EXIT_FAILURE);
}

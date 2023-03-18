#include <rt.h>
#include <util.h>

#include <stdio.h>
#include <string.h>

void rt_run(atom_t *at) {
    die_assert(at->kind == ATOM_BATCH || at->kind == ATOM_INLINE, "rt_run expects batch");

    for (size_t i = 0; i < buf_len(at->list_val); i++) {
        atom_t cmd = at->list_val[i];
        die_assert(cmd.kind == ATOM_CMD,                "batch must consist of commands");
        die_assert(cmd.list_val[0].kind == ATOM_SYMBOL, "command name must be symbol"); 

        char *name = cmd.list_val[0].str_val;
        if (strcmp(name, "echo") == 0) {
            die_assert(buf_len(cmd.list_val) >= 2, "echo expects at least 1 argument");

            for (size_t j = 1; j < buf_len(cmd.list_val); j++) {
                atom_t val = cmd.list_val[j];

                switch (val.kind) {
                    case ATOM_SYMBOL: 
                    case ATOM_STRING: printf("%s", val.str_val); break; 
                    case ATOM_INT:    printf("%ld", val.int_val); break;

                    default: die("echo: unimplemented atom '%s'", atom_kind_str(val.kind)); break;
                }
            }            

            printf("\n");
        } else {
            die("unknown command: %s", name);
        }
    }
}

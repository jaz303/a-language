#include "menace/global.h"
#include "menace/ast.h"
#include "menace/intern.h"

#include <stdlib.h>

int is_globally_initialised = 0;

int global_init() {
    if (!is_globally_initialised) {
        ast_global_init();
        is_globally_initialised = 1;
    }
    return 1;
}

int context_init(context_t *ctx) {
    if (!is_globally_initialised) {
        if (!global_init()) {
            return 0;
        }
    }
    
    ctx->gc_head = NULL;
    
    ctx->code = NULL;
    ctx->code_pos = 0;
    ctx->code_capacity = 0;
    
    intern_init(ctx);
    ast_init(ctx);
    
    return 1;
}

const char *token_get_name(token_t token) {
    return token_names[token];
}

UINT roundup2(UINT v) {
    /* http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2 */
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    
    return v;
}

void fatal_error(const char *msg) {
    printf("[error] %s\n", msg);
    exit(1);
}

void memory_error() {
    printf("[error] memory error :(\n");
    exit(1);
}

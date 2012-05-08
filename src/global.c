#include "menace/global.h"
#include "menace/ast.h"

#include <stdlib.h>

int context_init(context_t *ctx) {
    ast_init_context(ctx);
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

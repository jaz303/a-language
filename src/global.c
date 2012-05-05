#include "menace/global.h"

#include <stdlib.h>

const char *token_get_name(token_t token) {
    return token_names[token];
}

void fatal_error(const char *msg) {
    printf("[error] %s\n");
    exit(1);
}

void memory_error() {
    printf("[error] memory error :(\n");
    exit(1);
}

#include "menace/intern.h"
#include "menace/hash.h"

#include <stdlib.h>
#include <string.h>

#define INITIAL_ARRAY_SIZE 64

void intern_init(context_t *ctx) {
    
    ctx->intern.array_size = INITIAL_ARRAY_SIZE;
    ctx->intern.count = 0;
    
    intern_table_init(&ctx->intern.s2i);
    
    ctx->intern.i2s = malloc(sizeof(char*) * ctx->intern.array_size);
    if (ctx->intern.i2s == NULL) {
        memory_error();
    }
    
}

INTERN string_to_intern(context_t *ctx, const char *str) {
    INTERN out = intern_table_get(&ctx->intern.s2i, str);
    
    if (out == 0) {
        out = ++ctx->intern.count;
        
        if (out == ctx->intern.array_size) {
            size_t new_array_size = ctx->intern.array_size * 1.5;
            ctx->intern.i2s = realloc(ctx->intern.i2s, sizeof(char*) * new_array_size);
            if (ctx->intern.i2s == NULL) {
                memory_error();
            }
        }
        
        char *copy = malloc(strlen(str) + 1);
        if (copy == NULL) {
            memory_error();
        }
        strcpy(copy, str);
        
        intern_table_put(&ctx->intern.s2i, copy, out);
        ctx->intern.i2s[out] = copy;
    }
    
    return out;
}

const char* intern_to_string(context_t *ctx, INTERN i) {
    if (i == 0 || i > ctx->intern.count) {
        return NULL;
    } else {
        return ctx->intern.i2s[i];
    }
}

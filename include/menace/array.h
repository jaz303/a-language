#ifndef ARRAY_H
#define ARRAY_H

#include "menace/global.h"


void            array_init(context_t *ctx, obj_array_t *ary);
void            array_init_with_capacity(context_t *ctx, obj_array_t *ary, UINT capacity);

void            array_cleanup(context_t *ctx, obj_array_t *ary);
    
UINT            array_get_length(context_t *ctx, obj_array_t *ary);
VALUE           array_get_index(context_t *ctx, obj_array_t *ary, UINT ix);
void            array_set_index(context_t *ctx, obj_array_t *ary, UINT ix, VALUE v);

void            array_push(context_t *ctx, obj_array_t *ary, VALUE v);
VALUE           array_pop(context_t *ctx, obj_array_t *ary);
void            array_reverse(context_t *ctx, obj_array_t *ary);

#endif
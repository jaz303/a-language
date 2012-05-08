#ifndef ARRAY_H
#define ARRAY_H

#include "menace/global.h"

obj_array_t*    array_create(context_t *ctx);
obj_array_t*    array_create_with_capacity(context_t *ctx, UINT capacity);

void            array_destroy(context_t *ctx, obj_array_t *ary);

UINT            array_get_size(context_t *ctx, obj_array_t *ary);
VALUE           array_get_index(context_t *ctx, obj_array_t *ary, UINT ix);
void            array_set_index(context_t *ctx, obj_array_t *ary, UINT ix, VALUE v);

void            array_push(context_t *ctx, obj_array_t *ary, VALUE v);
VALUE           array_pop(context_t *ctx, obj_array_t *ary);
void            array_reverse(context_t *ctx, obj_array_t *ary);

#endif
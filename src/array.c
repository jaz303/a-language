#include "menace/array.h"

#include <stdlib.h>

// TODO: need to use GC memory allocation when it's ready
// TODO: shrink array
// TODO: tweakable growth factor?

static void grow(context_t *ctx, obj_array_t *ary) {
    UINT new_cap = ary->capacity * 2;
    ary->values = realloc(ary->values, sizeof(VALUE) * new_cap);
    if (!ary->values) memory_error();
    ary->capacity = new_cap;
}

obj_array_t* array_create(context_t *ctx) {
    return array_create_with_capacity(ctx, 8);
}

obj_array_t* array_create_with_capacity(context_t *ctx, UINT capacity) {
    // TODO: replace with gc_alloc()
    obj_array_t *ary = malloc(sizeof(obj_array_t));
    if (!ary) memory_error();
    
    ary->obj.type = ARRAY_T;
    ary->length = 0;
    ary->capacity = roundup2(capacity);
    
    ary->values = malloc(sizeof(VALUE) * ary->capacity);
    if (!ary->values) memory_error();
    
    return ary;
}

void array_destroy(context_t *ctx, obj_array_t *ary) {
    // TODO: replace with gc_free()
    free(ary->values);
    free(ary);
}

UINT array_get_length(context_t *ctx, obj_array_t *ary) {
    return ary->length;
}

VALUE array_get_index(context_t *ctx, obj_array_t *ary, UINT ix) {
    if (ix < 0) ix = ary->length + ix;
    if (ix < 0 || ix >= ary->length) {
        return kNull;
    } else {
        return ary->values[ix];
    }
}

void array_set_index(context_t *ctx, obj_array_t *ary, UINT ix, VALUE v) {
    if (ix < 0) ix = ary->length + ix;
    if (ix < 0 || ix >= ary->length) {
        // TODO: exception
        fatal_error("attempting to set illegal array index");
    } else {
        ary->values[ix] = v;
    }
}

void array_push(context_t *ctx, obj_array_t *ary, VALUE v) {
    if (ary->length == ary->capacity) grow(ctx, ary);
    ary->values[ary->length++] = v;
}

VALUE array_pop(context_t *ctx, obj_array_t *ary) {
    if (ary->length == 0) {
        return kNull;
    } else {
        return ary->values[--(ary->length)];
    }
}

void array_reverse(context_t *ctx, obj_array_t *ary) {
    for (UINT i = 0, m = ary->length / 2; i < m; i++) {
        VALUE tmp = ary->values[ary->length - i];
        ary->values[ary->length - i] = ary->values[i];
        ary->values[i] = tmp;
    }
}
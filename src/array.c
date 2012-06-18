#include "menace/array.h"
#include "menace/gc.h"

#include <stdlib.h>

// TODO: shrink array
// TODO: tweakable growth factor?

#define DEFAULT_CAPACITY 8

static void grow(context_t *ctx, obj_array_t *ary) {
    UINT new_cap = ary->capacity * 2;
    ary->values = mnc_gc_realloc(ctx, ary->values, sizeof(VALUE) * new_cap);
    if (!ary->values) memory_error();
    ary->capacity = new_cap;
}

void array_init(context_t *ctx, obj_array_t *ary) {
    return array_init_with_capacity(ctx, ary, DEFAULT_CAPACITY);
}

void array_init_with_capacity(context_t *ctx, obj_array_t *ary, UINT capacity) {
    ary->length = 0;
    ary->capacity = roundup2(capacity);
    ary->values = mnc_gc_alloc(ctx, sizeof(VALUE) * ary->capacity);
    if (!ary->values) memory_error();
}

void array_cleanup(context_t *ctx, obj_array_t *ary) {
    mnc_gc_free(ctx, ary->values);
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
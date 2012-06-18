#include "menace/gc.h"
#include "menace/array.h"
#include "menace/hash.h"

#include <stdlib.h>

void* mnc_gc_alloc(context_t *ctx, size_t sz) {
    void *bytes = malloc(sz);
    if (!bytes) {
        mnc_gc_run(ctx);
        bytes = malloc(sz);
    }
    return bytes;
}

void* mnc_gc_calloc(context_t *ctx, size_t num, size_t sz) {
    void *bytes = calloc(num, sz);
    if (!bytes) {
        mnc_gc_run(ctx);
        bytes = calloc(num, sz);
    }
    return bytes;
}

void* mnc_gc_realloc(context_t *ctx, void *ptr, size_t sz) {
    void *bytes = realloc(ptr, sz);
    if (!bytes) {
        mnc_gc_run(ctx);
        bytes = realloc(ptr, sz);
    }
    return bytes;
}

void mnc_gc_free(context_t *ctx, void *ptr) {
    free(ptr);
}

void mnc_gc_run(context_t *ctx) {
    // TODO: iterate over all global symbols and the stack, marking everything
    mnc_gc_sweep(ctx);
}

void mnc_gc_mark(context_t *ctx, VALUE val) {
    obj_t *obj = AS_OBJECT(val);
    obj->gc.next = PTR_MASK(obj->gc.next, |, 0x01);
    if (obj->meta->gc_mark) {
        obj->meta->gc_mark(ctx, val);
    }
}

void mnc_gc_sweep(context_t *ctx) {
    obj_t *curr = ctx->gc_head;
    obj_t *prev = NULL;
    while (curr) {
        if (PTR_MASK(curr->gc.next, &, 0x03) == 0) {
            obj_t *victim = curr;
            if (victim->meta->gc_free) {
                victim->meta->gc_free(ctx, victim);
            }
            if (prev) {
                UINT prev_mask = (UINT) PTR_MASK(prev->gc.next, &, 0x03);
                prev->gc.next = PTR_MASK(curr->gc.next, |, prev_mask);
            } else {
                ctx->gc_head = curr->gc.next;
            }
            curr = curr->gc.next;
            free(victim);
        } else {
            curr->gc.next = PTR_MASK(curr->gc.next, &, ~0x01);
            prev = curr;
            curr = PTR_MASK(curr->gc.next, &, ~0x03);
        }
    }
}

static void* alloc_meta(context_t *ctx, size_t sz, meta_t *meta) {
    obj_t *obj = mnc_gc_alloc(ctx, sz);
    if (obj) {
        obj->meta = meta;
        obj->gc.next = ctx->gc_head;
        ctx->gc_head = obj;
    }
    return obj;
}

obj_float_t* mnc_gc_alloc_float(context_t *ctx) {
    return alloc_meta(ctx, sizeof(obj_float_t), &Float);
}

obj_string_t* mnc_gc_alloc_string(context_t *ctx) {
    return alloc_meta(ctx, sizeof(obj_string_t), &String);
}

obj_array_t* mnc_gc_alloc_array(context_t *ctx) {
    return alloc_meta(ctx, sizeof(obj_array_t), &Array);
}

obj_dict_t* mnc_gc_alloc_dict(context_t *ctx) {
    return alloc_meta(ctx, sizeof(obj_dict_t), &Dict);
}

obj_function_t* mnc_gc_alloc_function(context_t *ctx) {
    return alloc_meta(ctx, sizeof(obj_function_t), &Function);
}

obj_native_function_t* mnc_gc_alloc_native_function(context_t *ctx) {
    return alloc_meta(ctx, sizeof(obj_native_function_t), &NativeFunction);
}

void mnc_gc_pin(context_t *ctx, VALUE val) {
    if (IS_OBJECT(val)) {
        obj_t *obj = (obj_t*)val;
        obj->gc.next = PTR_MASK(obj->gc.next, |, 0x02);
    } else {
        // TODO: print error
    }
}

void mnc_gc_unpin(context_t *ctx, VALUE val) {
    if (IS_OBJECT(val)) {
        obj_t *obj = (obj_t*)val;
        obj->gc.next = PTR_MASK(obj->gc.next, &, ~0x02);
    } else {
        // TODO: print error
    }
}

/*
 * Markers
 */

void mnc_gc_mark_array(context_t *ctx, VALUE ary) {
    MNC_ARRAY_ITERATE(ary, i, val)
        if (IS_OBJECT(val)) mnc_gc_mark(ctx, val);
    MNC_ARRAY_ITERATE_END
}

void mnc_gc_mark_dict(context_t *ctx, VALUE dict) {
    MNC_DICT_ITERATE(dict, i, key, val)
        if (IS_OBJECT(key)) mnc_gc_mark(ctx, key);
        if (IS_OBJECT(val)) mnc_gc_mark(ctx, val);
    MNC_DICT_ITERATE_END
}

/*
 * Free-ers
 */

void mnc_gc_free_array(context_t *ctx, VALUE ary) {
    array_cleanup(ctx, (obj_array_t*)ary);
}

void mnc_gc_free_dict(context_t *ctx, VALUE dict) {
    dict_cleanup(&((obj_dict_t*)dict)->dict);
}

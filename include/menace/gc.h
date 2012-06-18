#ifndef MNC_GC_H
#define MNC_GC_H

#include "menace/global.h"

/*
 * These GC allocation functions will automatically run GC in the event
 * that allocation fails, then retry. If the retry fails, NULL will be
 * returned.
 *
 * NOTE: memory allocated by these functions is NOT "owned" by the GC;
 * you are still responsible for freeing it (with mnc_gc_free()) as
 * necessary. They merely exist as convenience for auto-running GC in
 * low-memory conditions.
 */
void*                   mnc_gc_alloc(context_t *ctx, size_t sz);
void*                   mnc_gc_calloc(context_t *ctx, size_t num, size_t sz);
void*                   mnc_gc_realloc(context_t *ctx, void *ptr, size_t sz);
void                    mnc_gc_free(context_t *ctx, void *ptr);

/*
 * Run the garbage collector
 */
void                    mnc_gc_run(context_t *ctx);
void                    mnc_gc_sweep(context_t *ctx);

/*
 * Mark the passed object by tagging its pointer and calling its registered
 * mark function (if any). It is the callers responsibility not to pass
 * any non-object types (e.g. primitives) to this function.
 */
void                    mnc_gc_mark(context_t *ctx, VALUE val);

/*
 * Object allocation functions; these do the folowing:
 *
 * 1. allocate memory for the object's struct
 * 2. set up the GC header
 * 3. set the object's meta ptr
 */

obj_float_t*            mnc_gc_alloc_float(context_t *ctx);
obj_string_t*           mnc_gc_alloc_string(context_t *ctx);
obj_array_t*            mnc_gc_alloc_array(context_t *ctx);
obj_dict_t*             mnc_gc_alloc_dict(context_t *ctx);
obj_function_t*         mnc_gc_alloc_function(context_t *ctx);
obj_native_function_t*  mnc_gc_alloc_native_function(context_t *ctx);

/*
 * Notify GC that object is referenced from C-land and should not be freed
 */
void                    mnc_gc_pin(context_t *ctx, VALUE val);
void                    mnc_gc_unpin(context_t *ctx, VALUE val);

void                    mnc_gc_mark_array(context_t *ctx, VALUE ary);
void                    mnc_gc_mark_dict(context_t *ctx, VALUE dict);

void                    mnc_gc_free_array(context_t *ctx, VALUE ary);
void                    mnc_gc_free_dict(context_t *ctx, VALUE dict);

#endif
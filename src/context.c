#include "menace/context.h"
#include "menace/intern.h"
#include "menace/gc.h"

#include <stdlib.h>

obj_native_function_t* ctx_register_native_function(context_t *ctx, const char *name, native_fn_f fn, void *userdata) {
    
    obj_native_function_t *obj = mnc_gc_alloc_native_function(ctx);
    if (!obj) {
        return 0;
    }
    
    obj->name       = string_to_intern(ctx, name);
    obj->fn         = fn;
    obj->userdata   = userdata;
    
    return obj;
    
}
#include "menace/context.h"
#include "menace/intern.h"

#include <stdlib.h>

obj_native_function_t* ctx_register_native_function(context_t *ctx, const char *name, native_fn_f fn, void *userdata) {
    
    obj_native_function_t *obj = malloc(sizeof(obj_native_function_t));
    if (!fn) {
        return 0;
    }
    
    // TODO: allocate from gc
    
    obj->obj.type   = NATIVE_FUNCTION_T;
    obj->name       = string_to_intern(ctx, name);
    obj->fn         = fn;
    obj->userdata   = userdata;
    
    return obj;
    
}
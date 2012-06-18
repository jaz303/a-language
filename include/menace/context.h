#ifndef CONTEXT_H
#define CONTEXT_H

#include "menace/global.h"

obj_native_function_t* ctx_register_native_function(context_t *ctx, const char *name, native_fn_f fn, void *userdata);

#endif
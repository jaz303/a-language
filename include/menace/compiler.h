#ifndef MNC_COMPILER_H
#define MNC_COMPILER_H

#include "menace/global.h"
#include "menace/ast.h"

obj_function_t*     mnc_compile_function(context_t *ctx, ast_function_t *ast);

#endif
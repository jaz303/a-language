#ifndef MENACE_AST_H
#define MENACE_AST_H

#include "menace/global.h"

void ast_init_context(context_t *ctx);
void* ast_alloc(context_t *ctx, size_t sz);
void* ast_alloc_with_type(context_t *ctx, size_t sz, ast_node_type_t type);
void ast_cleanup(context_t *ctx);

#endif

#include "menace/ast.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define AST_PAGE_KB 8

void ast_init_context(context_t *ctx) {
    // TODO: mojo
}

void* ast_alloc(context_t *ctx, size_t sz) {
    // TODO: ast node to be allocated from context pool
    void *ast_node = malloc(sz);
    if (!ast_node) memory_error();
    return ast_node;
}

void* ast_alloc_with_type(context_t *ctx, size_t sz, ast_node_type_t type) {
    ast_node_t *node = ast_alloc(ctx, sz);
    node->type = type;
    return node;
}

void ast_cleanup(context_t *ctx) {
    // TODO: clean up all allocated nodes
}


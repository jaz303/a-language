#ifndef MENACE_AST_H
#define MENACE_AST_H

#include "menace/global.h"

void            ast_init_context(context_t *ctx);
void            ast_cleanup_context(context_t *ctx);

int             ast_len(context_t *ctx, ast_id_t cell);
ast_id_t        ast_cell_next(context_t *ctx, ast_id_t cell);
ast_value_t*    ast_cell_value(context_t *ctx, ast_id_t cell);

ast_id_t        ast_get_free_cell(context_t *ctx);
ast_id_t        ast_get_free_value(context_t *ctx);
ast_id_t        ast_create_node(context_t *ctx, int node_type);
ast_id_t        ast_append_node(context_t *ctx, ast_id_t head, ast_id_t value);

void            ast_destroy(context_t *ctx, ast_id_t cell_or_value);

#endif
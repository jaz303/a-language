#include "menace/ast.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define INITIAL_AST_CELLS       1024
#define INITIAL_AST_VALUES      1024

static void grow_cell_pool(ast_pool_t *p) {
    ast_id_t new_size = p->num_cells * 2;
    p->cells = realloc(p->cells, new_size * sizeof(ast_cell_t));
    p->free_cells = realloc(p->free_cells, new_size * sizeof(ast_id_t));
    if (!p->cells || !p->free_cells) {
        memory_error();
    }
    for (int i = p->num_cells; i < new_size; i++) {
        p->free_cells[i] = i;
    }
    p->num_cells = new_size;
}

static void grow_value_pool(ast_pool_t *p) {
    ast_id_t new_size = p->num_values * 2;
    p->values = realloc(p->values, new_size * sizeof(ast_value_t));
    p->free_values = realloc(p->free_values, new_size * sizeof(ast_id_t));
    if (!p->values || !p->free_values) {
        memory_error();
    }
    for (int i = p->num_values; i < new_size; i++) {
        p->free_values[i] = i;
    }
    p->num_values = new_size;
}

void ast_init_context(context_t *ctx) {
    ast_pool_t *p = &ctx->ast_pool;
    
    p->num_cells = INITIAL_AST_CELLS;
    p->cells = malloc(p->num_cells * sizeof(ast_cell_t));
    p->free_cells = malloc(p->num_cells * sizeof(ast_id_t));
    p->next_free_cell_ix = 1;
    
    p->num_values = INITIAL_AST_VALUES;
    p->values = malloc(p->num_values * sizeof(ast_value_t));
    p->free_values = malloc(p->num_values * sizeof(ast_id_t));
    p->next_free_value_ix = 1;
    
    if (!p->cells || !p->free_cells || !p->values || !p->free_values) {
        memory_error();
    }
    
    for (int i = 0; i < INITIAL_AST_CELLS; i++) {
        p->free_cells[i] = i;
    }
    
    for (int i = 0; i < INITIAL_AST_VALUES; i++) {
        p->free_values[i] = i;
    }
}

void ast_cleanup_context(context_t *ctx) {
    free(ctx->ast_pool.cells);
    free(ctx->ast_pool.free_cells);
    free(ctx->ast_pool.values);
    free(ctx->ast_pool.free_values);
}

int ast_len(context_t *ctx, ast_id_t cell) {
    int len = 0;
    while (AST_INDEX(cell)) {
        len++;
        cell = ctx->ast_pool.cells[AST_INDEX(cell)].cdr;
    }
    return len;
}

ast_id_t ast_cell_next(context_t *ctx, ast_id_t cell) {
    assert(AST_IS_CELL(cell));
    return ctx->ast_pool.cells[AST_INDEX(cell)].cdr;
}

ast_value_t* ast_cell_value(context_t *ctx, ast_id_t cell) {
    assert(AST_IS_CELL(cell));
    ast_id_t value_ix = ctx->ast_pool.cells[AST_INDEX(cell)].car;
    assert(AST_IS_VALUE(value_ix));
    return &(ctx->ast_pool.values[AST_INDEX(value_ix)]);
}

ast_id_t ast_cell_cell(context_t *ctx, ast_id_t cell) {
    assert(AST_IS_CELL(cell));
    ast_id_t cell_ix = ctx->ast_pool.cells[AST_INDEX(cell)].car;
    assert(AST_IS_CELL(cell_ix));
    return cell_ix;
}

ast_id_t ast_get_free_cell(context_t *ctx) {
    if (ctx->ast_pool.next_free_cell_ix == ctx->ast_pool.num_cells) {
        grow_cell_pool(&ctx->ast_pool);
    }
    ast_id_t cell_ix = ctx->ast_pool.next_free_cell_ix++;
    ctx->ast_pool.cells[cell_ix].car = 0;
    ctx->ast_pool.cells[cell_ix].cdr = 0;
    return AST_MAKE_CELL(cell_ix);
}

ast_id_t ast_get_free_value(context_t *ctx) {
    if (ctx->ast_pool.next_free_value_ix == ctx->ast_pool.num_values) {
        grow_value_pool(&ctx->ast_pool);
    }
    ast_id_t cell_ix = ctx->ast_pool.next_free_value_ix++;
    return AST_MAKE_VALUE(cell_ix);
}

ast_id_t ast_create_node(context_t *ctx, int node_type) {
    ast_id_t type_ix = ast_get_free_value(ctx);
    ctx->ast_pool.values[AST_INDEX(type_ix)].node_type = node_type;
    
    ast_id_t cell_ix = ast_get_free_cell(ctx);
    ctx->ast_pool.cells[AST_INDEX(cell_ix)].car = type_ix;
    
    return cell_ix;
}

ast_id_t ast_append_node(context_t *ctx, ast_id_t head, ast_id_t value) {
    assert(AST_IS_CELL(head));
    
    ast_id_t cons_ix = ast_get_free_cell(ctx);
    ctx->ast_pool.cells[AST_INDEX(cons_ix)].car = value;
    
    ctx->ast_pool.cells[AST_INDEX(head)].cdr = cons_ix;

    return cons_ix;
}

void ast_destroy(context_t *ctx, ast_id_t cell_or_value) {
    if (AST_IS_CELL(cell_or_value)) {
        ast_id_t curr = cell_or_value;
        while (AST_INDEX(curr)) {
            ast_id_t ix = AST_INDEX(curr);
            ast_destroy(ctx, ctx->ast_pool.cells[ix].car);
            ast_id_t next = ctx->ast_pool.cells[ix].cdr;
            ctx->ast_pool.next_free_cell_ix--;
            ctx->ast_pool.free_cells[ctx->ast_pool.next_free_cell_ix] = ix;
            curr = next;
        }
    } else {
        ctx->ast_pool.next_free_value_ix--;
        ctx->ast_pool.free_values[ctx->ast_pool.next_free_value_ix] = AST_INDEX(cell_or_value);
    }
}

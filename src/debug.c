#include "menace/debug.h"
#include "menace/ast.h"
#include <assert.h>

#define NEXT(node)          ast_cell_next(ctx, node)
#define CELL_CELL(node)     ast_cell_cell(ctx, node)
#define CELL_VAL(node)      ast_cell_value(ctx, node)
#define PRINT_CDR(node) \
    while ((node = NEXT(node))) { \
        fputs(" ", stream); \
        do_debug_ast_print(ctx, CELL_CELL(node), stream); \
    }

static void do_debug_ast_print(context_t *ctx, ast_id_t node, FILE *stream) {
    assert(AST_IS_CELL(node));
    
    ast_value_t *val = CELL_VAL(node);
    if (val->node_type <= AST_LITERAL_MAX) {
        switch (val->node_type) {
            case AST_LITERAL_NULL:
            {
                fputs("null", stream);
                break;
            }
            case AST_LITERAL_INT:
            {
                ast_value_t *v = CELL_VAL(NEXT(node));
                fprintf(stream, "%d", (int)v->val_int);
                break;
            }
            case AST_LITERAL_BOOL:
            {
                ast_value_t *v = CELL_VAL(NEXT(node));
                fprintf(stream, "%s", v->val_int ? "true" : "false");
                break;
            }
            case AST_LITERAL_STRING:
                fputs("STRING", stream);
                break;
            case AST_LITERAL_COLOR:
                fprintf(stream, "rgba(%d,%d,%d,%d)", val->val_color[AST_RED],
                                                     val->val_color[AST_GREEN],
                                                     val->val_color[AST_BLUE],
                                                     val->val_color[AST_ALPHA]);
                break;
            case AST_LITERAL_SYMBOL:
                fputs(":SYMBOL", stream);
                break;
            default:
                fputs("???", stream);
                break;
        }
    } else {
        fputc('(', stream);
        
        switch (val->node_type) {
            case AST_STATEMENTS:
                fputs("statements", stream);
                PRINT_CDR(node);
                break;
            case AST_WHILE:
                fputs("while", stream);
                PRINT_CDR(node);
                break;
            case AST_IF:
                fputs("if", stream);
                PRINT_CDR(node);
                break;
            case AST_PASS:
                fputs("pass", stream);
                break;
            case AST_PARAMETER_LIST:
                fputs("parameter-list", stream);
                break;
            case AST_BINARY_OP:
            case AST_UNARY_OP:
                node = NEXT(node);
                fputs(token_get_name(CELL_VAL(node)->val_int), stream);
                PRINT_CDR(node);
                break;
            default:
                fputs("???", stream);
                break;
        }
        
        fputc(')', stream);
    }
}

void debug_ast_print(context_t *ctx, ast_id_t node, FILE *stream) {
    do_debug_ast_print(ctx, node, stream);
    fputc('\n', stream);
}

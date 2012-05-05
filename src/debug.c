#include "menace/debug.h"

static void do_debug_ast_print(ast_node_t *ast, FILE *stream) {
    // fputc('(', stream);
    // switch (ast->type) {
    //     
    //     case AST_LITERAL_INT:
    //         fprintf(stream, "int %d", ast->params[0].intval);
    //         break;
    //     case AST_LITERAL_BOOL:
    //         fprintf(stream, "bool %s", ast->params[0].intval ? "true" : "false");
    //         break;
    //     case AST_LITERAL_STRING:
    //         fprintf(stream, "string \"%s\"", ast->params[0].strval);
    //         break;
    //     case AST_LITERAL_COLOR:
    //         fprintf(stream, "color %d,%d,%d,%d", ast->params[0].colorval[AST_RED],
    //                                              ast->params[0].colorval[AST_GREEN],
    //                                              ast->params[0].colorval[AST_BLUE],
    //                                              ast->params[0].colorval[AST_ALPHA]);
    //         break;
    //     case AST_SYMBOL:
    //         fprintf(stream, "symbol :%s", ast->params[0].strval);
    //         break;
    //     
    //     default:
    //         fprintf(stderr, "%s: error, unknown AST node type: %d\n", __PRETTY_FUNCTION__, ast->type);
    //         break;
    // 
    // }
    // fputc(')', stream);
}

void debug_ast_print(ast_node_t *ast, FILE *stream) {
    do_debug_ast_print(ast, stream);
    fputc('\n', stream);
}

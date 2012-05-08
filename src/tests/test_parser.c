#include <stdio.h>
#include <stdlib.h>

#include "menace/global.h"
#include "menace/ast.h"
#include "menace/parser.h"
#include "menace/debug.h"

int main(int argc, char *argv[]) {
    
    context_t context;
    context_init(&context);
    
    scanner_t *scanner  = scanner_create_for_file(stdin);
    parser_t *parser    = parser_create(&context, scanner);
    ast_id_t root       = parser_parse(parser);
    
    printf("success: %s\n", root ? "yes" : "no");
    
    // 
    // 
    // parser_t parser;
    // parser_init(&parser, &context, scanner);
    // 
    // ast_id_t root = parser_parse(&parser);
    // 
    // if (root == 0) {
    //     printf("parse error: %s\n", parser.error);
    // } else {
    //     debug_ast_print(&context, root, stdout);
    // }
    
    return 0;
}
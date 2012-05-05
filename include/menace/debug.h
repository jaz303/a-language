#ifndef MENACE_DEBUG_H
#define MENACE_DEBUG_H

#include <stdio.h>

#include "menace/ast.h"

void debug_ast_print(ast_node_t *ast, FILE *stream);

#endif
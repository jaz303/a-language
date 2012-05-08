#ifndef MENACE_DEBUG_H
#define MENACE_DEBUG_H

#include "menace/global.h"
#include <stdio.h>

void debug_ast_print(context_t *ctx, ast_id_t node, FILE *stream);

#endif
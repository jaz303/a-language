#ifndef MENACE_DEBUG_H
#define MENACE_DEBUG_H

#include "menace/global.h"
#include <stdio.h>

void pretty_print(context_t *ctx, ast_statements_t *stmts, FILE *stream);

#endif
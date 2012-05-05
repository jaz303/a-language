#ifndef MENACE_PARSER_H
#define MENACE_PARSER_H

#include "menace/scanner.h"
#include "menace/ast.h"

int         parser_init(parser_t *p, context_t *c, scanner_t *s);
ast_id_t    parser_parse(parser_t *p);

#endif
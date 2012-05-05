#ifndef MENACE_SCANNER_H
#define MENACE_SCANNER_H

#include <stdio.h>

#include "menace/global.h"

scanner_t*  scanner_create_for_file(FILE *file);
void        scanner_destroy(scanner_t *s);
token_t     scanner_get_next_token(scanner_t *s, const char **text, int *len);

#endif
#ifndef INTERN_H
#define INTERN_H

#include "menace/global.h"

void        intern_init(context_t *ctx);

INTERN      string_to_intern(context_t *ctx, const char *str);
const char* intern_to_string(context_t *ctx, INTERN i);

#endif
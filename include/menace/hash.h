#ifndef HASH_H
#define HASH_H

#include "menace/global.h"

hash_int_t          hash_djb2(const char* key);
hash_int_t          hash_sdbm(const char* key);

intern_table_t*     intern_table_create(context_t *ctx);
void                intern_table_destroy(context_t *ctx, intern_table_t *hsh);
INTERN              intern_table_get(intern_table_t *hsh, const char *str);
void                intern_table_put(intern_table_t *hsh, const char *str, INTERN val);
hash_int_t          intern_table_get_size(intern_table_t *hsh);

symbol_table_t*     symbol_table_create(context_t *ctx);
void                symbol_table_destroy(symbol_table_t *hsh);
VALUE               symbol_table_get(symbol_table_t *hsh, INTERN sym);
void                symbol_table_put(symbol_table_t *hsh, INTERN sym, VALUE val);
hash_int_t          symbol_table_get_size(symbol_table_t *hsh);

dict_t*             dict_create(context_t *ctx);
void                dict_destroy(dict_t *hsh);
VALUE               dict_get(dict_t *hsh, VALUE key);
void                dict_put(dict_t *hsh, VALUE key, VALUE val);
hash_int_t          dict_get_size(dict_t *hsh);

#endif

#ifndef HASH_H
#define HASH_H

#include "menace/global.h"

#define GH_DEBUG_PRINT(hsh) \
            printf("b=%lu occ=%lu sz=%lu ub=%lu\n", \
                        (unsigned long)(hsh)->n_buckets, \
                        (unsigned long)(hsh)->n_occupied, \
                        (unsigned long)(hsh)->size, \
                        (unsigned long)(hsh)->upper_bound)

hash_int_t          hash_djb2(const char* key);
hash_int_t          hash_sdbm(const char* key);

intern_table_t*     intern_table_create(context_t *ctx);
void                intern_table_init(intern_table_t *hsh);
void                intern_table_destroy(context_t *ctx, intern_table_t *hsh);
INTERN              intern_table_get(intern_table_t *hsh, const char *str);
void                intern_table_put(intern_table_t *hsh, const char *str, INTERN val);
hash_int_t          intern_table_size(intern_table_t *hsh);

symbol_table_t*     symbol_table_create(context_t *ctx);
void                symbol_table_init(symbol_table_t *hsh);
void                symbol_table_destroy(symbol_table_t *hsh);
VALUE               symbol_table_get(symbol_table_t *hsh, INTERN sym);
void                symbol_table_put(symbol_table_t *hsh, INTERN sym, VALUE val);
VALUE               symbol_table_delete(symbol_table_t *hsh, INTERN sym);
hash_int_t          symbol_table_size(symbol_table_t *hsh);

dict_t*             dict_create(context_t *ctx);
void                dict_init(dict_t *hsh);
void                dict_destroy(dict_t *hsh);
VALUE               dict_get(dict_t *hsh, VALUE key);
void                dict_put(dict_t *hsh, VALUE key, VALUE val);
VALUE               dict_delete(dict_t *hsh, VALUE key);
hash_int_t          dict_size(dict_t *hsh);

#endif

#ifndef HASH_H
#define HASH_H

#include "menace/global.h"

#define MNC_DICT_ITERATE(hsh, itervar, keyvar, valvar) \
    for (hash_int_t itervar = 0; itervar < ((hash_t*)hsh)->n_buckets; ++itervar) { \
        if (GH_BUCKET_STATE(((hash_t*)hsh)->flags, itervar) != GH_BUCKET_FULL) continue; \
        VALUE keyvar = ((hash_t*)hsh)->buckets[itervar].key.value; \
        VALUE valvar = ((hash_t*)hsh)->buckets[itervar].value.value;
        
#define MNC_DICT_ITERATE_END \
    }
    
/* 
 * This stuff is only exposed for use by the MNC_DICT_ITERATE macro.
 * Please regard it as private!
 */

#define GH_BUCKET_EMPTY                     0
#define GH_BUCKET_FULL                      1
#define GH_BUCKET_DELETED                   2
    
// TODO: benchmark usefulness of all this bit-fiddling
#define GH_BUCKET_STATE(flags, ix)          ((flags[ix>>2]>>((ix&3)<<1))&3)
#define GH_SET_BUCKET_STATE(flags, ix, s)   (flags[ix>>2]=(flags[ix>>2]&(~(3<<((ix&3)<<1))))|(s<<((ix&3)<<1)))

/* End Private */

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

void                dict_init(dict_t *hsh);
void                dict_cleanup(dict_t *hsh);
VALUE               dict_get(dict_t *hsh, VALUE key);
void                dict_put(dict_t *hsh, VALUE key, VALUE val);
VALUE               dict_delete(dict_t *hsh, VALUE key);
hash_int_t          dict_size(dict_t *hsh);

#endif

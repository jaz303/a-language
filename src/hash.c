#include "menace/hash.h"

#include <string.h>
#include <stdlib.h>

#define GH_BUCKET_EMPTY                     0
#define GH_BUCKET_FULL                      1
#define GH_BUCKET_DELETED                   2
    
// TODO: benchmark usefulness of all this bit-fiddling
#define GH_BUCKET_STATE(flags, ix)          ((flags[ix>>2]>>((ix&3)<<1))&3)
#define GH_SET_BUCKET_STATE(flags, ix, s)   (flags[ix>>2]=(flags[ix>>2]&(~(3<<((ix&3)<<1))))|(s<<((ix&3)<<1)))

#define HASH_ALLOC(hsh,sz)                  malloc(sz)
#define HASH_REALLOC(hsh,ptr,sz)            realloc(ptr,sz)
#define HASH_FREE(hsh,ptr)                  free(ptr)
#define HASH_MAX_LOAD                       0.7
#define HASH_CAST(__var__)                  hash_t *h = (hash_t*)__var__
#define HASH_STRING(str)                    hash_djb2(str)

static hash_int_t bucket_sizes[] = {
    0,          3,          11,         23,         53,         97,
    193,        389,        769,        1543,       3079,       6151,
    12289,      24593,      49157,      98317,      196613,     393241,
    786433,     1572869,    3145739,    6291469,    12582917,   25165843,
    50331653,   100663319,  201326611,  402653189,  805306457,  1610612741
};

static const hash_int_t n_bucket_sizes = (sizeof(bucket_sizes)/sizeof(hash_int_t));

/* debug */
#ifdef GEN_HASH_DEBUG
	#include <assert.h>
	#define __gh_debug 1
#else
	#define __gh_debug 0
#endif

static int hash_resize(hash_t *h, hash_int_t new_buckets) {
    unsigned char *new_flags = NULL;
    hash_int_t pix = n_bucket_sizes - 1;
    while (bucket_sizes[pix] > new_buckets) pix--;
    new_buckets = bucket_sizes[pix + 1];
    
    /* this is in attractivechaos's original but not sure why; \
     * calling resize is based on hash occupancy, not size, so \
     * why use size here? i'll leave this code here as a \
     * reminder to muse over it every now and again... */ \
    /* if (h->size >= (new_buckets * HASH_MAX_LOAD + 0.5)) return 1; */
    
    hash_int_t new_flags_size = sizeof(unsigned char) * ((new_buckets >> 2) + 1);
    new_flags = HASH_ALLOC(h, new_flags_size);
    if (!new_flags) memory_error();
    memset(new_flags, 0, new_flags_size);
    
    if (new_buckets > h->n_buckets) {
        h->buckets = HASH_REALLOC(h, h->buckets, new_buckets * sizeof(hash_node_t));
        if (!h->buckets) memory_error();
    }
    
    for (int j = 0; j < h->n_buckets; j++) {
        if (GH_BUCKET_STATE(h->flags, j) == GH_BUCKET_FULL) {
            hash_key_t key = h->buckets[j].key;
            hash_value_t val = h->buckets[j].value;
            GH_SET_BUCKET_STATE(h->flags, j, GH_BUCKET_DELETED);
            while (1) {
                hash_int_t hc;
                switch (h->type) {
                    case HASH_SYMBOL_TABLE:     hc = (hash_int_t)key.symbol;    break;
                    case HASH_INTERN_TABLE:     hc = HASH_STRING(key.string);   break;
                    case HASH_DICT:             hc = 0; /* TODO: hash value */  break;
                }
                hash_int_t hb   = hc % new_buckets;
                hash_int_t inc  = 1 + hc % (new_buckets - 1);
                while (GH_BUCKET_STATE(new_flags, hb) != GH_BUCKET_EMPTY) {
                    hb += inc;
                    if (hb >= new_buckets) hb -= new_buckets;
                }
                GH_SET_BUCKET_STATE(new_flags, hb, GH_BUCKET_FULL);
                if (hb < h->n_buckets && GH_BUCKET_STATE(h->flags, hb) == GH_BUCKET_FULL) {
                    hash_key_t tmp_key = h->buckets[hb].key;
                    hash_value_t tmp_val = h->buckets[hb].value;
                    h->buckets[hb].key = key;
                    h->buckets[hb].value = val;
                    key = tmp_key;
                    val = tmp_val;
                    GH_SET_BUCKET_STATE(h->flags, hb, GH_BUCKET_DELETED);
                } else {
                    h->buckets[hb].key = key;
                    h->buckets[hb].value = val;
                    break;
                }
            }
        }
    }
    
    if (new_buckets < h->n_buckets) {
        h->buckets = HASH_REALLOC(h, h->buckets, new_buckets * sizeof(hash_node_t));
        if (!h->buckets) memory_error();
    }
    
    HASH_FREE(h, h->flags);
    h->flags = new_flags;
    h->n_buckets = new_buckets;
    h->n_occupied = h->size;
    h->upper_bound = h->n_buckets * HASH_MAX_LOAD + 0.5;
    
    return 1;
}

static void hash_init(hash_t *h, hash_type_t type) {
    memset(h, 0, sizeof(hash_t));
    h->type = type;
}

hash_int_t find_slot_by_str(hash_t *hsh, const char *str) {
    if (hsh->n_buckets) {
        hash_int_t hc   = HASH_STRING(str);
        hash_int_t hb   = hc % hsh->n_buckets;
        hash_int_t inc  = 1 + hc % (hsh->n_buckets - 1);
        hash_int_t last = hb;
        while (1) {
            char state = GH_BUCKET_STATE(hsh->flags, hb);
            if (state == GH_BUCKET_EMPTY) {
                break;
            } else if (state == GH_BUCKET_FULL && strcmp(hsh->buckets[hb].key.string, str) == 0) {
                return hb;
            }
            hb += inc;
            if (hb >= hsh->n_buckets) hb -= hsh->n_buckets;
            if (hb == last) break;
        }
    }
    return hsh->n_buckets;
}

hash_int_t find_slot_by_value(hash_t *hsh, VALUE val) {
    // TODO: need to impl value_hash() and value_is_equal()
    return 0;
}

hash_int_t find_slot_by_symbol(hash_t *hsh, INTERN symbol) {
        if (hsh->n_buckets) {
        hash_int_t hc   = (hash_int_t) symbol;
        hash_int_t hb   = hc % hsh->n_buckets;
        hash_int_t inc  = 1 + hc % (hsh->n_buckets - 1);
        hash_int_t last = hb;
        while (1) {
            char state = GH_BUCKET_STATE(hsh->flags, hb);
            if (state == GH_BUCKET_EMPTY) {
                break;
            } else if (state == GH_BUCKET_FULL && (hsh->buckets[hb].key.symbol == symbol)) {
                return hb;
            }
            hb += inc;
            if (hb >= hsh->n_buckets) hb -= hsh->n_buckets;
            if (hb == last) break;
        }
    }
    return hsh->n_buckets;
}

static hash_int_t hash_put(hash_t *hsh, hash_int_t hc, hash_key_t k, hash_value_t v) {
    
    if (hsh->n_occupied >= hsh->upper_bound) {
        hash_resize(hsh, hsh->n_buckets + ((hsh->n_buckets > (hsh->size * 2)) ? -1 : 1));
    }
    
    hash_int_t hb   = hc % hsh->n_buckets;
    hash_int_t inc  = 1 + hc % (hsh->n_buckets - 1);
    hash_int_t tgt  = hsh->n_buckets;
    hash_int_t old  = hsh->n_buckets;
    
    while (1) {
        char state = GH_BUCKET_STATE(hsh->flags, hb);
        if (state == GH_BUCKET_EMPTY) {
            if (tgt == hsh->n_buckets) tgt = hb;
            break; /* search is over; this key can't exist anywhere else */
        } else if (state == GH_BUCKET_DELETED) {
            if (tgt == hsh->n_buckets) tgt = hb;
        } else {
            int keq;
            switch (hsh->type) {
                case HASH_SYMBOL_TABLE: keq = (k.symbol == hsh->buckets[hb].key.symbol);            break;
                case HASH_INTERN_TABLE: keq = (strcmp(k.string, hsh->buckets[hb].key.string) == 0); break;
                case HASH_DICT:         keq = 0; /* TODO: value eq */                               break;
            }
            if (keq) {
                old = hb;
                if (tgt == hsh->n_buckets) tgt = hb;
                break;
            }
        }
        hb += inc;
        if (hb >= hsh->n_buckets) hb -= hsh->n_buckets;
    }
    
    if (old != hsh->n_buckets) { /* replace */
        hsh->buckets[tgt].value = v;
        if (old != tgt) {
            hsh->buckets[tgt].key = hsh->buckets[old].key;
            GH_SET_BUCKET_STATE(hsh->flags, old, GH_BUCKET_DELETED);
            GH_SET_BUCKET_STATE(hsh->flags, tgt, GH_BUCKET_FULL);
        }
    } else { /* insert */
        hsh->size++;
        if (GH_BUCKET_STATE(hsh->flags, tgt) == GH_BUCKET_EMPTY) hsh->n_occupied++;
        hsh->buckets[tgt].key = k;
        hsh->buckets[tgt].value = v;
        GH_SET_BUCKET_STATE(hsh->flags, tgt, GH_BUCKET_FULL);
    }
    
    return tgt;

}

/*
 * Public interface
 */
 
/* hash functions from http://www.cse.yorku.ca/~oz/hash.html */

hash_int_t hash_djb2(const char* key) {
    const unsigned char *str = (const unsigned char *)key;
    hash_int_t hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash;
}
  
hash_int_t hash_sdbm(const char* key) {
    const unsigned char *str = (const unsigned char *)key;
    hash_int_t hash = 0;
    int c;
    while ((c = *str++))
        hash = c + (hash << 6) + (hash << 16) - hash;
    return hash;
}

intern_table_t* intern_table_create(context_t *ctx) {
    hash_t *hsh = malloc(sizeof(hash_t));
    if (hsh) hash_init(hsh, HASH_INTERN_TABLE);
    return (intern_table_t*)hsh;
}

void intern_table_init(intern_table_t *hsh) {
    hash_init((hash_t*)hsh, HASH_INTERN_TABLE);
}

void intern_table_destroy(context_t *ctx, intern_table_t *hsh) {
    HASH_CAST(hsh);
    // iterate over all buckets, if state is full , free
    (void)h;
}

INTERN intern_table_get(intern_table_t *hsh, const char *str) {
    HASH_CAST(hsh);
    hash_int_t slot = find_slot_by_str(h, str);
    if (slot == h->n_buckets) {
        return 0; /* zero is never used as an intern value */
    } else {
        return h->buckets[slot].value.symbol;
    }
}

void intern_table_put(intern_table_t *hsh, const char *str, INTERN val) {
    HASH_CAST(hsh);
    hash_key_t key;
    key.string = str;
    hash_value_t value;
    value.symbol = val;
    hash_put(h, HASH_STRING(str), key, value);
}

int intern_table_delete(intern_table_t *hsh, const char *str) {
    HASH_CAST(hsh);
    hash_int_t slot = find_slot_by_str(h, str);
    if (slot == h->n_buckets) {
        return 0;
    } else {
        GH_SET_BUCKET_STATE(h->flags, slot, GH_BUCKET_DELETED);
        h->size--;
        return 1;
    }
}

hash_int_t intern_table_size(intern_table_t *hsh) {
    HASH_CAST(hsh);
    return h->size;
}

symbol_table_t* symbol_table_create(context_t *ctx) {
    hash_t *hsh = malloc(sizeof(hash_t));
    if (hsh) hash_init(hsh, HASH_SYMBOL_TABLE);
    return (symbol_table_t*)hsh;
}

void symbol_table_init(symbol_table_t *hsh) {
    hash_init((hash_t*)hsh, HASH_SYMBOL_TABLE);
}

void symbol_table_destroy(symbol_table_t *hsh) {
    HASH_CAST(hsh);
    (void)h;
}

VALUE symbol_table_get(symbol_table_t *hsh, INTERN sym) {
    HASH_CAST(hsh);
    hash_int_t slot = find_slot_by_symbol(h, sym);
    if (slot == h->n_buckets) {
        return NULL;
    } else {
        return h->buckets[slot].value.value;
    }
}

void symbol_table_put(symbol_table_t *hsh, INTERN sym, VALUE val) {
    HASH_CAST(hsh);
    hash_key_t key;
    key.symbol = sym;
    hash_value_t value;
    value.value = val;
    hash_put(h, sym, key, value);
}

VALUE symbol_table_delete(symbol_table_t *hsh, INTERN sym) {
    HASH_CAST(hsh);
    hash_int_t slot = find_slot_by_symbol(h, sym);
    if (slot == h->n_buckets) {
        return 0;
    } else {
        VALUE v = h->buckets[slot].value.value;
        GH_SET_BUCKET_STATE(h->flags, slot, GH_BUCKET_DELETED);
        h->size--;
        return v;
    }
}

hash_int_t symbol_table_size(symbol_table_t *hsh) {
    HASH_CAST(hsh);
    return h->size;
}

dict_t* dict_create(context_t *ctx) {
    hash_t *hsh = malloc(sizeof(hash_t));
    if (hsh) hash_init(hsh, HASH_DICT);
    return (symbol_table_t*)hsh;
}

void dict_init(dict_t *hsh) {
    hash_init((hash_t*)hsh, HASH_DICT);
}

void dict_destroy(dict_t *hsh) {
    HASH_CAST(hsh);
    (void)h;
}

int dict_contains(dict_t *hsh, VALUE key) {
    HASH_CAST(hsh);
    return find_slot_by_value(h, key) != h->n_buckets;
}

VALUE dict_get(dict_t *hsh, VALUE key) {
    HASH_CAST(hsh);
    hash_int_t slot = find_slot_by_value(h, key);
    if (slot == h->n_buckets) {
        return NULL;
    } else {
        return h->buckets[slot].value.value;
    }
}

void dict_put(dict_t *hsh, VALUE key, VALUE val) {
    HASH_CAST(hsh);
    (void)h;
    // TODO: hash value
}

VALUE dict_delete(dict_t *hsh, VALUE key) {
    HASH_CAST(hsh);
    hash_int_t slot = find_slot_by_value(h, key);
    if (slot == h->n_buckets) {
        return 0;
    } else {
        VALUE v = h->buckets[slot].value.value;
        GH_SET_BUCKET_STATE(h->flags, slot, GH_BUCKET_DELETED);
        h->size--;
        return v;
    }
}

hash_int_t dict_size(dict_t *hsh) {
    HASH_CAST(hsh);
    return h->size;
}

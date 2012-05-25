#include "menace/global.h"
#include "menace/hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <assert.h>

#define MIN(a,b) (((a)<(b))?(a):(b))

typedef struct hash_test {
    INTERN  key;
    VALUE   value;
    int     in_table;
} hash_test_t;

#define COUNT   480000
#define PASSES  200

hash_test_t table[COUNT];

void random_string(char *target, int suffix) {
    int len = (rand() % 8) + 1;
    while (len--) *(target++) = (rand() % 26) + 'a';
    sprintf(target, "%d", suffix);
}

void check(symbol_table_t *hsh) {
    unsigned long sz = 0;
    int i;
    for (i = 0; i < COUNT; i++) {
        if (table[i].in_table) sz++;
        VALUE value = symbol_table_get(hsh, table[i].key);
        if ((value > 0) != table[i].in_table) {
            printf("error (in_table=%d, ix=%d k=%ld)\n", table[i].in_table, i, (long) table[i].key);
        } else if (value && value != table[i].value) {
            printf("cmp error\n");
        }
    }
    if (sz != hsh->size) {
        printf("size error (exp=%lu,rep=%lu)\n", sz, (unsigned long)hsh->size);
    }
}

int main(int argc, char *argv[]) {
    
    context_t *ctx = malloc(sizeof(context_t));
    context_init(ctx);
    
    srand(time(NULL));
    
    int i;
    for (i = 0; i < COUNT; i++) {
        do {
            table[i].key = rand();
        } while (table[i].key == 0);
        table[i].value = MK_INTVAL(rand() % 0x7fffffff);
        table[i].in_table = 0;
        if (((i+1)%50000) == 0) {
            printf("%d random pairs generated\n", i+1);
        }
    }
    
    symbol_table_t *hsh = symbol_table_create(ctx);

    unsigned long ops = 0;

    int j;
    for (j = 0; j < PASSES; j++) {
        int ins = (j % 4 == 0) ? 0 : 1;
        int range = (rand() % 30000) + 1;
        ops += range;
        int min = rand() % COUNT;
        int max = MIN(COUNT, min + range);
        printf("Pass %d/%d %s (%d) %d-%d\n", j+1, PASSES, ins ? "insert" : "delete", range, min, max);
        GH_DEBUG_PRINT(hsh);
        if (ins) {
            for (i = min; i < max; i++) {
                hash_int_t sz = hsh->size;
                symbol_table_put(hsh, table[i].key, table[i].value);
                if (!table[i].in_table) {
                    assert(sz + 1 == hsh->size);
                } else if (sz != hsh->size) {
                    printf("expected=%lu actual=%lu\n", (unsigned long)sz, (unsigned long)hsh->size);
                    assert(0);
                }
                table[i].in_table = 1;
            }
        } else {
            for (i = min; i < max; i++) {
                hash_int_t sz = hsh->size;
                if (symbol_table_delete(hsh, table[i].key)) {
                    assert(table[i].in_table);
                    assert(sz - 1 == hsh->size);
                    table[i].in_table = 0;
                } else if (table[i].in_table) {
                    printf("delete error\n");
                    exit(1);
                }
            }
        }
        check(hsh);
    }
    
    printf("ops: %lu\n", ops);
    
    return 0;
    
}
#include "menace/builtin_types.h"
#include "menace/gc.h"

meta_t Float = {
    .gc_mark = NULL,
    .gc_free = NULL
};

meta_t String = {
    .gc_mark = NULL,
    .gc_free = NULL
};

meta_t Array = {
    .gc_mark = mnc_gc_mark_array,
    .gc_free = mnc_gc_free_array
};

meta_t Dict = {
    .gc_mark = mnc_gc_mark_dict,
    .gc_free = mnc_gc_free_dict
};

meta_t Function = {
    .gc_mark = NULL,
    .gc_free = NULL
};

meta_t NativeFunction = {
    .gc_mark = NULL,
    .gc_free = NULL
};
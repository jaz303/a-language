#ifndef MENACE_GLOBAL_H
#define MENACE_GLOBAL_H

#include <stdio.h>
#include <stdint.h>

#ifdef _LP64
	typedef void*       VALUE;
	typedef int64_t     INT;
	typedef uint64_t    INTERN;
#else
	typedef void*       VALUE;
	typedef int32_t     INT;
	typedef uint32_t    INTERN;
#endif

#define INT_BIT             0x01
#define SPECIAL_BIT         0x02
#define TAG_MASK            (INT_BIT | SPECIAL_BIT)

#define VALUE_IS_PTR(v)     (((INT)v & TAG_MASK) == 0)
#define MK_PTR(p)           ((VALUE)p)
#define PTR(v)              ((void*)v)

#define INT_SHIFT           1
#define VALUE_IS_INT(v)     (((INT)v & INT_BIT) == INT_BIT)
#define MK_INTVAL(i)        ((VALUE)(((INT)(i) << INT_SHIFT) | INT_BIT))
#define INTVAL(v)           (((INT)v) >> INT_SHIFT)

#define SPECIAL_SHIFT       2
#define SPECIAL_NIL         0x00
#define SPECIAL_FALSE       0x01
#define SPECIAL_TRUE        0x03
#define SPECIAL_SYMBOL      0x04

#define kNil                ((VALUE)(SPECIAL_BIT | (SPECIAL_NIL << SPECIAL_SHIFT)))
#define kFalse              ((VALUE)(SPECIAL_BIT | (SPECIAL_FALSE << SPECIAL_SHIFT)))
#define kTrue               ((VALUE)(SPECIAL_BIT | (SPECIAL_TRUE << SPECIAL_SHIFT)))

/*
 * Object types
 */
 
typedef enum {
    OBJ_STRING      = 1,
    OBJ_FN,
    OBJ_NATIVE_FN,
    OBJ_OPAQUE,
    OBJ_FLOAT,
    OBJ_DATE,
    OBJ_MONEY,
    OBJ_COLOR,
    OBJ_ARRAY,
    OBJ_DICT,
    OBJ_OBJECT,
    OBJ_REGEXP
} obj_type_t;


/*
 * Primitive types - gnarly
 */

typedef int32_t INTEGER;
typedef uint32_t COLOR;

/*
 * Array-backed memory pool
 */

typedef struct {
    size_t          element_size;
    int32_t         size;
    char            *data;
    int32_t         *free_list;
    int32_t         free_pos;
} array_pool_t;

/*
 * Token types
 */

#define TOKEN(symbol, string) symbol
enum {
    T_IGNORE = 0, /* zero is used by the scanner as a sentinel */
    #include "menace/tokens.x"
};
#undef TOKEN

typedef unsigned char token_t;
extern const char const *token_names[];

/*
 * AST structures and macros
 */

typedef uint32_t ast_id_t;

#define AST_IS_CELL(ix)     ((ix & 0x80000000) == 0x80000000)
#define AST_IS_VALUE(ix)    ((ix & 0x80000000) == 0)
#define AST_MAKE_CELL(ix)   (ix | 0x80000000)
#define AST_MAKE_VALUE(ix)  (ix)
#define AST_INDEX(ix)       (ix & 0x7fffffff)

typedef struct {
    ast_id_t car;   /* either a cell or a value */
    ast_id_t cdr;   /* either a cell or 0/nil */
} ast_cell_t;

enum {
    AST_ALPHA   = 0,
    AST_RED     = 1,
    AST_GREEN   = 2,
    AST_BLUE    = 3
};

typedef union {
    enum {
        AST_LITERAL_NULL,
        AST_LITERAL_INT,
        AST_LITERAL_BOOL,
        AST_LITERAL_STRING,
        AST_LITERAL_COLOR,
        AST_SYMBOL,
        AST_STATEMENTS,
        AST_WHILE,
        AST_IF,
        AST_PASS,
        AST_PARAMETER_LIST,
    }               node_type;
    INT             val_int;
    unsigned char   val_color[4];
    char            *val_string;
} ast_value_t;


typedef struct {
    ast_id_t        num_cells;
    ast_cell_t      *cells;
    ast_id_t        *free_cells;
    ast_id_t        next_free_cell_ix;
    ast_id_t        num_values;
    ast_value_t     *values;
    ast_id_t        *free_values;
    ast_id_t        next_free_value_ix;
} ast_pool_t;

/* 
 * Main context object
 */

typedef struct {
    ast_pool_t      ast_pool;
} context_t;

/*
 * Scanner; defined as void because it's declared in scanner.c and can
 * only be created dynamically.
 */

typedef void scanner_t;

/*
 * Parser object
 */

typedef struct {
    context_t       *context;
    scanner_t       *scanner;
    token_t         current_token;
    const char      *token_text;
    int             token_len;
    char            *error;
} parser_t;

/* 
 * Utility Functions
 */

const char *token_get_name(token_t);

/* terminates */
void fatal_error(const char *msg) __attribute__ ((noreturn));

/* terminates */
void memory_error() __attribute__ ((noreturn));

#endif
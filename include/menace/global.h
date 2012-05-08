#ifndef MENACE_GLOBAL_H
#define MENACE_GLOBAL_H

#include <stdio.h>
#include <stdint.h>

/* define this to pack floats/colors/dates into VALUE types (64-bit only) */
/* (TODO!!!) */
// #define USE_MOAR_PACKING

/*
 * Object types
 */
 
typedef enum {
    INTEGER_T           = 1,
    BOOLEAN_T,
    SYMBOL_T,
    NULL_T,
    
    FLOAT_T,
    
    STRING_T,
    FUNCTION_T,
    NATIVE_FUNCTION_T,
    OPAQUE_T,
    DATE_T,
    MONEY_T,
    COLOR_T,
    ARRAY_T,
    DICT_T,
    OBJECT_T,
    REGEXP_T
} obj_type_t;

#ifdef _LP64
	typedef void*       VALUE;
	typedef int64_t     INT;
    typedef uint64_t    UINT;
	typedef uint64_t    INTERN;
#else
	typedef void*       VALUE;
	typedef int32_t     INT;
    typedef uint32_t    UINT;
	typedef uint32_t    INTERN;
#endif

/*
 * VM instruction type
 */
 
typedef union {
    INT     o;      /* opcode */
    INT     i;      /* integer operand */
    UINT    u;      /* unsigned integer operand */
    VALUE   v;      /* value operand */
} inst_t;

/*
 * 000000 - ptr
 * 000001 - int
 * 000010 - null
 * 000110 - false
 * 001110 - true
 * 001010 - symbol
 */

#define VALUE_IS_PTR(v)             (((INT)v & 0x03) == 0)
#define MK_PTR(p)                   ((VALUE)p)
#define PTR(v)                      ((void*)v)

#define VALUE_IS_INT(v)             (((INT)v & 0x01) == 0x01)
#define MK_INTVAL(i)                ((VALUE)((((INT)(i)) << 1) | 0x01))
#define INTVAL(v)                   (((INT)v) >> 1)

#define kNull                       (0x02)

#define VALUE_IS_BOOL(v)            ((((INT)v) & 0x06) == 0x06)
#define kFalse                      (0x06)
#define kTrue                       (0x0C)

#define VALUE_IS_SYMBOL(v)          ((((INTERN)v) & 0x0A) == 0x0A)
#define MK_SYMBOL(i)                ((VALUE)((((INTERN)(i)) << 4) & 0x0A))
#define SYMBOLVAL(v)                (((INTERN)v) >> 4)

#define AS_OBJECT(v)                ((obj_t*)v)
#define IS_OBJECT(v)                (VALUE_IS_PTR(v))

#define OBJ_IS_FLOAT(v)             (AS_OBJECT(v)->type == FLOAT_T)
#define OBJ_IS_COLOR(v)             (AS_OBJECT(v)->type == COLOR_T)
#define OBJ_IS_STRING(v)            (AS_OBJECT(v)->type == STRING_T)
#define OBJ_IS_FUNCTION(v)          (AS_OBJECT(v)->type == FUNCTION_T)
#define OBJ_IS_NATIVE_FUNCTION(v)   (AS_OBJECT(v)->type == NATIVE_FUNCTION_T)
#define OBJ_IS_OPAQUE(v)            (AS_OBJECT(v)->type == OPAQUE_T)
#define OBJ_IS_DATE(v)              (AS_OBJECT(v)->type == DATE_T)
#define OBJ_IS_MONEY(v)             (AS_OBJECT(v)->type == MONEY_T)
#define OBJ_IS_ARRAY(v)             (AS_OBJECT(v)->type == ARRAY_T)
#define OBJ_IS_DICT(v)              (AS_OBJECT(v)->type == DICT_T)
#define OBJ_IS_OBJECT(v)            (AS_OBJECT(v)->type == OBJECT_T)
#define OBJ_IS_REGEXP(v)            (AS_OBJECT(v)->type == REGEXP_T)

#define VALUE_IS_FLOAT(v)           (IS_OBJECT(v) && AS_OBJECT(v)->type == FLOAT_T)
#define VALUE_IS_COLOR(v)           (IS_OBJECT(v) && AS_OBJECT(v)->type == COLOR_T)
#define VALUE_IS_STRING(v)          (IS_OBJECT(v) && AS_OBJECT(v)->type == STRING_T)
#define VALUE_IS_FUNCTION(v)        (IS_OBJECT(v) && AS_OBJECT(v)->type == FUNCTION_T)
#define VALUE_IS_NATIVE_FUNCTION(v) (IS_OBJECT(v) && AS_OBJECT(v)->type == NATIVE_FUNCTION_T)
#define VALUE_IS_OPAQUE(v)          (IS_OBJECT(v) && AS_OBJECT(v)->type == OPAQUE_T)
#define VALUE_IS_DATE(v)            (IS_OBJECT(v) && AS_OBJECT(v)->type == DATE_T)
#define VALUE_IS_MONEY(v)           (IS_OBJECT(v) && AS_OBJECT(v)->type == MONEY_T)
#define VALUE_IS_ARRAY(v)           (IS_OBJECT(v) && AS_OBJECT(v)->type == ARRAY_T)
#define VALUE_IS_DICT(v)            (IS_OBJECT(v) && AS_OBJECT(v)->type == DICT_T)
#define VALUE_IS_OBJECT(v)          (IS_OBJECT(v) && AS_OBJECT(v)->type == OBJECT_T)
#define VALUE_IS_REGEXP(v)          (IS_OBJECT(v) && AS_OBJECT(v)->type == REGEXP_T)

#define VALUE_IS_TRUTHY(v)          (((INT)(v)) != kNull && ((INT)(v)) != kFalse)

/*
 * Object types
 */
 
typedef struct gc_header {
    char _;
} gc_header_t;

typedef struct obj {
    gc_header_t     gc;
    obj_type_t      type;
} obj_t;

typedef struct obj_array {
    obj_t       obj;                /* header */
    VALUE       *values;            /* array contents */
    UINT        length;             /* # of elements in array */
    UINT        capacity;           /* size of backing store */
} obj_array_t;

typedef struct obj_function {
    obj_t       obj;                /* header */
    UINT        frame_size;         /* how much space must we reserve for args/locals? */
    inst_t      *code;              /* opcodes */
    /* source code? */
    /* AST? */
} obj_function_t;

/*
 * Primitive types - gnarly
 */

typedef int32_t INTEGER;
typedef uint32_t COLOR;

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

#define AST_IS_CELL(ix)         ((ix & 0x80000000) == 0x80000000)
#define AST_IS_VALUE(ix)        ((ix & 0x80000000) == 0)
#define AST_MAKE_CELL(ix)       (ix | 0x80000000)
#define AST_MAKE_VALUE(ix)      (ix)
#define AST_INDEX(ix)           (ix & 0x7fffffff)
#define AST_GET_VALUE(ctx, v)   (ctx->ast_pool->values[AST_INDEX(v)])

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
        AST_LITERAL_IDENT,
        AST_LITERAL_SYMBOL,
        AST_STATEMENTS,
        AST_WHILE,
        AST_IF,
        AST_PASS,
        AST_PARAMETER_LIST,
        AST_BINARY_OP,
        AST_UNARY_OP,
    }               node_type;
    INT             val_int;
    unsigned char   val_color[4];
    char            *val_string;
} ast_value_t;

#define AST_LITERAL_MAX AST_LITERAL_SYMBOL

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
typedef void parser_t;

/*
 * VM Opcodes & State
 */

#define OPCODE(code) OP_##code,
typedef enum {
#include "menace/opcodes.x"
} opcode_t;
#undef OPCODE

#define OPCODE_MAX OP_HALT

typedef struct {
    
} vm_t;

/* 
 * Utility Functions
 */
 
int context_init(context_t *ctx);
const char *token_get_name(token_t);
UINT roundup2(UINT v);

/* terminates */
void fatal_error(const char *msg) __attribute__ ((noreturn));

/* terminates */
void memory_error() __attribute__ ((noreturn));

/*
 *
 */
 
#define CASSERT(ix, expn) typedef char __C_ASSERT_##ix##__[(expn)?1:-1] 

CASSERT(1, sizeof(VALUE) == sizeof(INT));
CASSERT(2, sizeof(VALUE) == sizeof(UINT));
CASSERT(3, sizeof(VALUE) == sizeof(INTERN));
CASSERT(4, sizeof(VALUE) == sizeof(inst_t));

#endif
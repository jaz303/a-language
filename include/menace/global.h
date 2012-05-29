#ifndef MENACE_GLOBAL_H
#define MENACE_GLOBAL_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/*
 * Forward declarations
 */

typedef struct context context_t;

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

typedef float REAL;

/*
 * Type tagging:
 *
 * 000000 - ptr
 * 000001 - int     (31/63 MSBs == integer value)
 * 000010 - null    (== 0x02)
 * 000110 - false   (== 0x06)
 * 001110 - true    (== 0x0C) (boolean can be identified with `foo & 0x06`)
 * 001010 - symbol  (28/60 MSBs == symbol value)
 */
 
#define MK_PTR(p)                   ((VALUE)p)
#define PTR(v)                      ((void*)v)

#define MK_INTVAL(i)                ((VALUE)((((INT)(i)) << 1) | 0x01))
#define INTVAL(v)                   (((INT)v) >> 1)

#define MK_SYMBOL(i)                ((VALUE)((((INTERN)(i)) << 4) | 0x0A))
#define SYMBOLVAL(v)                (((INTERN)v) >> 4)

#define MK_FLOATVAL(ctx, v)         ((void)v, NULL)
#define FLOATVAL(v)                 (((obj_float_t*)(v))->value)

#define kNull                       ((VALUE)0x02)
#define kFalse                      ((VALUE)0x06)
#define kTrue                       ((VALUE)0x0C)

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

#define VALUE_IS_PTR(v)             (((INT)v & 0x03) == 0)
#define VALUE_IS_INT(v)             (((INT)v & 0x01) == 0x01)
#define VALUE_IS_BOOL(v)            ((((INT)v) & 0x06) == 0x06)
#define VALUE_IS_SYMBOL(v)          ((((INT)v) & 0x0A) == 0x0A)
#define VALUE_IS_FLOAT(v)           (IS_OBJECT(v) && OBJ_IS_FLOAT(v))
#define VALUE_IS_COLOR(v)           (IS_OBJECT(v) && OBJ_IS_COLOR(v))
#define VALUE_IS_STRING(v)          (IS_OBJECT(v) && OBJ_IS_STRING(v))
#define VALUE_IS_FUNCTION(v)        (IS_OBJECT(v) && OBJ_IS_FUNCTION(v))
#define VALUE_IS_NATIVE_FUNCTION(v) (IS_OBJECT(v) && OBJ_IS_NATIVE_FUNCTION(v))
#define VALUE_IS_OPAQUE(v)          (IS_OBJECT(v) && OBJ_IS_OPAQUE(v))
#define VALUE_IS_DATE(v)            (IS_OBJECT(v) && OBJ_IS_DATE(v))
#define VALUE_IS_MONEY(v)           (IS_OBJECT(v) && OBJ_IS_MONEY(v))
#define VALUE_IS_ARRAY(v)           (IS_OBJECT(v) && OBJ_IS_ARRAY(v))
#define VALUE_IS_DICT(v)            (IS_OBJECT(v) && OBJ_IS_DICT(v))
#define VALUE_IS_OBJECT(v)          (IS_OBJECT(v) && OBJ_IS_OBJECT(v))
#define VALUE_IS_REGEXP(v)          (IS_OBJECT(v) && OBJ_IS_REGEXP(v))

#define VALUE_IS_TRUTHY(v)          ((v != kFalse) && (v != kNull))

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
 * Object types
 */
 
typedef struct gc_header {
    char _;
} gc_header_t;

typedef struct obj {
    gc_header_t     gc;
    obj_type_t      type;
} obj_t;

typedef struct obj_float {
    obj_t       obj;
    REAL        value;
} obj_float_t;

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
 * Hash table
 */
 
#ifdef _LP64
    typedef uint64_t    hash_int_t;
#else
    typedef uint32_t    hash_int_t;
#endif

typedef hash_int_t hash_iter_t;
 
typedef union {
    VALUE           value;
    INTERN          symbol;
    const char*     string;
} hash_key_t;

typedef union {
    VALUE           value;
    INTERN          symbol;
} hash_value_t;

typedef struct hash_node hash_node_t;
struct hash_node {
    hash_key_t      key;
    hash_value_t    value;
};

typedef enum {
    HASH_SYMBOL_TABLE,                      /* INTERN   => VALUE    */
    HASH_INTERN_TABLE,                      /* cstring  => INTERN   */
    HASH_DICT                               /* VALUE    => VALUE    */
} hash_type_t;

typedef struct {
    hash_type_t         type;               /* type of hash table */
    hash_int_t          n_buckets;          /* # of buckets allocated */
    hash_int_t          n_occupied;         /* # of occupied buckets (i.e. full or deleted) */
    hash_int_t          upper_bound;        /* threshold of occupied buckets at which we will resize */
    hash_int_t          size;               /* # of K/V pairs in the hash (i.e. full buckets) */
    unsigned char       *flags;             /* auxiliary packed flag array for tracking bucket states */
    hash_node_t         *buckets;           /* the buckets */
    void                *userdata;          /* not used at present */
} hash_t;

typedef hash_t symbol_table_t;
typedef hash_t intern_table_t;
typedef hash_t dict_t;

/*
 * Intern wrapper struct
 */
 
typedef struct {
    INTERN              count;              /* number of interned strings */
    size_t              array_size;         /* allocated size of backing array */
    intern_table_t      s2i;                /* cstring => INTERN */
    const char          **i2s;              /* INTERN => cstring */
} intern_t;

/*
 * Token types
 */

#define TOKEN(symbol, string) symbol
enum {
    T_IGNORE = 0, /* zero is used by the scanner as a sentinel */
    #include "menace/tokens.x"
};
#undef TOKEN

/* semantic token names */
#define T_B_AND     T_AMPERSAND
#define T_B_OR      T_PIPE
#define T_B_XOR     T_HAT
#define T_B_NOT     T_TILDE
#define T_L_AND     T_DBL_AMPERSAND
#define T_L_OR      T_DBL_PIPE
#define T_L_NOT     T_BANG

typedef unsigned char token_t;
extern const char const *token_names[];

/*
 * AST structures and macros
 */

enum {
    AST_ALPHA   = 0,
    AST_RED     = 1,
    AST_GREEN   = 2,
    AST_BLUE    = 3
};

typedef struct {
} ast_pool_t;

typedef enum {
    AST_LITERAL,
    AST_IDENT,
    AST_ARRAY,
    AST_DICT,
    AST_WHILE,
    AST_ASSIGN,
    AST_IF,
    AST_PASS,
    AST_RETURN,
    AST_FUNCTION,
    AST_UNARY_EXP,
    AST_BINARY_EXP,
    AST_SELECTOR,
    AST_INVOKE,
    AST_INDEX
} ast_node_type_t;

#define AST_LITERAL_MAX AST_LITERAL_SYMBOL

/* Forward declarations */

typedef struct ast_node                     ast_node_t;
typedef struct ast_statements               ast_statements_t;
typedef struct ast_expressions              ast_expressions_t;
typedef struct ast_conditions               ast_conditions_t;
typedef struct ast_parameters               ast_parameters_t;
typedef struct ast_array_members            ast_array_members_t;
typedef struct ast_dict_members             ast_dict_members_t;
typedef struct ast_literal                  ast_literal_t;
typedef struct ast_ident                    ast_ident_t;
typedef struct ast_literal_collection       ast_literal_collection_t;
typedef struct ast_unary_exp                ast_unary_exp_t;
typedef struct ast_binary_exp               ast_binary_exp_t;
typedef struct ast_while                    ast_while_t;
typedef struct ast_if                       ast_if_t;
typedef struct ast_pass                     ast_pass_t;
typedef struct ast_assign                   ast_assign_t;
typedef struct ast_return                   ast_return_t;
typedef struct ast_function                 ast_function_t;
typedef struct ast_selector                 ast_selector_t;
typedef struct ast_invoke                   ast_invoke_t;
typedef struct ast_index                    ast_index_t;

/* AST support types */

struct ast_node {
    ast_node_type_t         type;
};

struct ast_statements {
    ast_node_t              *statement;
    ast_statements_t        *next;
};

struct ast_expressions {
    ast_node_t              *exp;
    ast_expressions_t       *next;
};

struct ast_conditions {
    ast_node_t              *exp;
    ast_statements_t        *body;
    ast_conditions_t        *next;
};

struct ast_parameters {
    INTERN                  name;
    ast_parameters_t        *next;
};

struct ast_array_members {
    ast_node_t              *value;
    ast_array_members_t     *next;
};

struct ast_dict_members {
    ast_node_t              *key;
    ast_node_t              *value;
    ast_dict_members_t      *next;
};

/* AST expression types */

struct ast_literal {
    ast_node_type_t         type;
    VALUE                   value;
};

struct ast_ident {
    ast_node_type_t         type;
    INTERN                  name;
};

struct ast_literal_collection {
    ast_node_type_t         type;
    void                    *head;
};

struct ast_unary_exp {
    ast_node_type_t         type;
    token_t                 operator;
    ast_node_t              *exp;
};

struct ast_binary_exp {
    ast_node_type_t         type;
    token_t                 operator;
    ast_node_t              *lexp;
    ast_node_t              *rexp;
};

/* AST statement types */

struct ast_while {
    ast_node_type_t         type;
    ast_node_t              *condition;
    ast_statements_t        *body;
};

struct ast_if {
    ast_node_type_t         type;
    ast_conditions_t        *conditions;
};

struct ast_pass {
    ast_node_type_t         type;
};

struct ast_assign {
    ast_node_type_t         type;
    ast_node_t              *target;
    ast_node_t              *value;
};

struct ast_return {
    ast_node_type_t         type;
    ast_node_t              *exp;
};

/* named function definition */
struct ast_function {
    ast_node_type_t         type;
    INTERN                  name;
    int                     arity;
    ast_parameters_t        *parameters;
    ast_statements_t        *body;
};

/* property access:
 * foo.bar
 */
struct ast_selector {
    ast_node_type_t         type;
    ast_node_t              *receiver;
    INTERN                  name;
};

/* method/function invoke:
 * foo.bar(a, b, c)         : receiver == foo, name = bar
 * (a).bar(a, b, c)         : receiver == (a), name = bar
 * bar()                    : receiver == NULL, name = bar
 */
struct ast_invoke {
    ast_node_type_t         type;
    ast_node_t              *receiver;
    INTERN                  name;
    ast_expressions_t       *arguments;
};

/* array indexing:
 * bar[0]
 * foo[1,2,3]
 */
struct ast_index {
    ast_node_type_t         type;
    ast_node_t              *receiver;
    ast_expressions_t       *arguments;
};

/* 
 * Main context object
 */

struct context {
    ast_pool_t      ast_pool;
    intern_t        intern;
};

/*
 * Scanner; defined as void because it's declared in scanner.c and can
 * only be created dynamically.
 */

typedef void scanner_t;

typedef struct {
    context_t   *context;
    scanner_t   *scanner;
    char        *error;
    token_t     current_token;
    char        *current_text;
    int         current_len;
} parser_t;

/*
 * VM Opcodes & State
 */

#define OPCODE(code) OP_##code,
typedef enum {
#include "menace/opcodes.x"
} opcode_t;
#undef OPCODE

#define OPCODE_MAX OP_HALT

/*
 * VM State
 */

typedef struct {
    
} vm_t;

/* 
 * Utility Functions
 */

int             global_init();
int             context_init(context_t *ctx);
const char*     token_get_name(token_t);
UINT            roundup2(UINT v);
void            fatal_error(const char *msg) __attribute__ ((noreturn));    /* terminates */
void            memory_error() __attribute__ ((noreturn));                  /* terminates */

/*
 * Sanity check; all fundamental types should be the same size
 */
 
#define CASSERT(ix, expn) typedef char __C_ASSERT_##ix##__[(expn)?1:-1] 

CASSERT(1, sizeof(VALUE) == sizeof(INT));
CASSERT(2, sizeof(VALUE) == sizeof(UINT));
CASSERT(3, sizeof(VALUE) == sizeof(INTERN));
CASSERT(4, sizeof(VALUE) == sizeof(inst_t));
CASSERT(5, sizeof(VALUE) == sizeof(hash_key_t));
CASSERT(6, sizeof(VALUE) == sizeof(hash_value_t));

#endif

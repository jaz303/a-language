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

#define PTR_MASK(ptr, op, mask) ((void*)(((UINT)ptr) op (mask)))

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

#define OBJ_IS_FLOAT(v)             (AS_OBJECT(v)->meta == &Float)
#define OBJ_IS_STRING(v)            (AS_OBJECT(v)->meta == &String)
#define OBJ_IS_FUNCTION(v)          (AS_OBJECT(v)->meta == &Function)
#define OBJ_IS_NATIVE_FUNCTION(v)   (AS_OBJECT(v)->meta == &NativeFunction)
#define OBJ_IS_ARRAY(v)             (AS_OBJECT(v)->meta == &Array)
#define OBJ_IS_DICT(v)              (AS_OBJECT(v)->meta == &Dict)

#define VALUE_IS_PTR(v)             (((INT)v & 0x03) == 0)
#define VALUE_IS_INT(v)             (((INT)v & 0x01) == 0x01)
#define VALUE_IS_BOOL(v)            ((((INT)v) & 0x06) == 0x06)
#define VALUE_IS_SYMBOL(v)          ((((INT)v) & 0x0A) == 0x0A)
#define VALUE_IS_FLOAT(v)           (IS_OBJECT(v) && OBJ_IS_FLOAT(v))
#define VALUE_IS_STRING(v)          (IS_OBJECT(v) && OBJ_IS_STRING(v))
#define VALUE_IS_FUNCTION(v)        (IS_OBJECT(v) && OBJ_IS_FUNCTION(v))
#define VALUE_IS_NATIVE_FUNCTION(v) (IS_OBJECT(v) && OBJ_IS_NATIVE_FUNCTION(v))
#define VALUE_IS_ARRAY(v)           (IS_OBJECT(v) && OBJ_IS_ARRAY(v))
#define VALUE_IS_DICT(v)            (IS_OBJECT(v) && OBJ_IS_DICT(v))

#define MTEST(v)                    ((v != kFalse) && (v != kNull))

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
} hash_t;

typedef hash_t symbol_table_t;
typedef hash_t intern_table_t;
typedef hash_t dict_t;

/*
 * VM instruction type
 */
 
typedef union {
    INT     o;      /* opcode */
    INT     i;      /* integer operand */
    UINT    u;      /* unsigned integer operand */
    VALUE   v;      /* value operand */
    INTERN  n;      /* name operand */
} inst_t;

/*
 * Object types
 */

typedef struct {
    void (*gc_mark)(context_t *ctx, VALUE val);
    void (*gc_free)(context_t *ctx, VALUE val);
} meta_t;

typedef struct obj obj_t;
 
typedef struct gc_header {
    obj_t           *next;
} gc_header_t;

struct obj {
    gc_header_t     gc;
    meta_t          *meta;
};

extern meta_t Float;
extern meta_t String;
extern meta_t Array;
extern meta_t Dict;
extern meta_t Function;
extern meta_t NativeFunction;

typedef struct obj_float {
    obj_t       obj;
    REAL        value;
} obj_float_t;

typedef struct obj_string {
    obj_t       obj;
    INT         len;
    char        *str;
} obj_string_t;

typedef struct obj_array {
    obj_t       obj;                /* header */
    VALUE       *values;            /* array contents */
    UINT        length;             /* # of elements in array */
    UINT        capacity;           /* size of backing store */
} obj_array_t;

typedef struct obj_dict {
    obj_t       obj;                /* header */
    dict_t      dict;               /* hash table */
} obj_dict_t;

typedef struct obj_function {
    obj_t           obj;            /* header */
    UINT            frame_size;     /* number of VALUE-sized slots required for args/locals */
    inst_t          *code;          /* opcodes */
    int             arity;          /* number of args */
    symbol_table_t  symbols;        /* map symbols to frame slots - to support runtime eval */
    /* source code? */
    /* AST? */
    /* documentation */
} obj_function_t;

typedef VALUE (*native_fn_f)(context_t *ctx,
                             VALUE receiver,
                             VALUE *args,
                             int nargs,
                             VALUE *exception,
                             void *userdata);

typedef struct obj_native_function {
    obj_t           obj;
    INTERN          name;
    native_fn_f     fn;
    void            *userdata;
} obj_native_function_t;

/*
 * Primitive types - gnarly
 */
 
typedef uint32_t COLOR;

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
    AST_STRING,
    AST_IDENT,
    AST_ARRAY,
    AST_DICT,
    AST_WHILE,
    AST_FOR,
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
typedef struct ast_string                   ast_string_t;
typedef struct ast_ident                    ast_ident_t;
typedef struct ast_literal_collection       ast_literal_collection_t;
typedef struct ast_unary_exp                ast_unary_exp_t;
typedef struct ast_binary_exp               ast_binary_exp_t;
typedef struct ast_while                    ast_while_t;
typedef struct ast_for                      ast_for_t;
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

struct ast_string {
    ast_node_type_t         type;
    INTERN                  string;
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

struct ast_for {
    ast_node_type_t         type;
    INTERN                  key_var;
    INTERN                  value_var;
    ast_node_t              *exp;
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
    obj_t           *gc_head;
    ast_pool_t      ast_pool;
    intern_t        intern;
    
    inst_t          *code;              /* array of instructions currently being compiled */
    int             code_pos;           /* position of next instruction to be inserted in code array */
    int             code_capacity;      /* max # of instructions in code array */
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

#include "menace/compiler.h"
#include "menace/global.h"
#include "menace/gc.h"
#include "menace/hash.h"

#include <assert.h>
#include <string.h>

/*
 * TODO: handle allocation errors during grow_code()
 *       best thing to do is longjmp to the function compiler, I think...
 * TODO: context should have a generalised way of recording an error, preferably
 *       with a known-sized buffer and support for sprintf(). This will be used
 *       in the compiler for reporting an unknown node type, amongst other things.
 */

typedef struct cmpstate {
    context_t           *ctx;
    obj_function_t      *fn;
    symbol_table_t      *symbols;
    UINT                next_slot;
} cmpstate_t;

#define INITIAL_CODE_SIZE   256
#define CODE_GROWTH_FACTOR  1.5
#define CTX                 (cmpstate->ctx)

/* compilation helpers */

#define COMPILE_FN(name, ast_type) static int compile_##name(cmpstate_t *cmpstate, ast_type *node)
#define COMPILE(name, ast_type, node) compile_##name(cmpstate, (ast_type*)node)

COMPILE_FN(statement,   ast_node_t);
COMPILE_FN(while,       ast_while_t);
COMPILE_FN(for,         ast_for_t);
COMPILE_FN(assign,      ast_assign_t);
COMPILE_FN(if,          ast_if_t);
COMPILE_FN(pass,        ast_pass_t);
COMPILE_FN(return,      ast_return_t);

/* code emission helpers */

#define EMIT_OP(o)      emit_op(CTX, o)
#define EMIT_INT(i)     emit_int(CTX, i)
#define EMIT_UINT(u)    emit_uint(CTX, u)
#define EMIT_VALUE(v)   emit_value(CTX, v)

static int grow_code(context_t *ctx) {
    if (ctx->code_capacity == 0) {
        ctx->code_capacity = INITIAL_CODE_SIZE;
        ctx->code = mnc_gc_alloc(ctx, sizeof(inst_t) * ctx->code_capacity);
    } else {
        ctx->code_capacity *= CODE_GROWTH_FACTOR;
        ctx->code = mnc_gc_realloc(ctx, ctx->code, sizeof(inst_t) * ctx->code_capacity);
        
    }
    return ctx->code != NULL;
}

static void emit_op(context_t *ctx, INT o) {
    if (ctx->code_pos == ctx->code_capacity) grow_code(ctx);
    ctx->code[ctx->code_pos++].o = o;
}

static void emit_int(context_t *ctx, INT i) {
    if (ctx->code_pos == ctx->code_capacity) grow_code(ctx);
    ctx->code[ctx->code_pos++].i = i;
}

static void emit_uint(context_t *ctx, UINT u) {
    if (ctx->code_pos == ctx->code_capacity) grow_code(ctx);
    ctx->code[ctx->code_pos++].u = u;
}

static void emit_value(context_t *ctx, VALUE v) {
    if (ctx->code_pos == ctx->code_capacity) grow_code(ctx);
    ctx->code[ctx->code_pos++].v = v;
}

/* frame helpers */

#define FRAME_SLOT(sym) frame_slot_for_ident(cmpstate, sym)
static UINT frame_slot_for_ident(cmpstate_t *state, INTERN symbol) {
    if (symbol_table_contains(state->symbols, symbol)) {
        return (UINT)symbol_table_get(state->symbols, symbol);
    } else {
        UINT slot = (state->fn->frame_size++);
        symbol_table_put(state->symbols, symbol, (VALUE)slot);
        return slot;
    }
}

obj_function_t* mnc_compile_function(context_t *ctx, ast_function_t *ast) {
    
    obj_function_t *fn = mnc_gc_alloc_function(ctx);
    fn->frame_size  = 0;
    fn->code        = NULL;
    fn->arity       = 0;
    symbol_table_init(&fn->symbols);
    
    cmpstate_t _cmpstate;
    _cmpstate.fn        = fn;
    _cmpstate.symbols   = &fn->symbols;
    cmpstate_t *cmpstate = &_cmpstate;
    
    /* set up a frame slot for each parameter */
    ast_parameters_t *params = ast->parameters;
    while (params) {
        FRAME_SLOT(params->name);
        fn->arity++;
        params = params->next;
    }
    
    /* mark the start of this function's code */
    size_t code_start = ctx->code_pos;
    
    /* compile function body */
    ast_statements_t *stmts = ast->body;
    assert(stmts);
    while (stmts) {
        compile_statement(cmpstate, stmts->statement);
        stmts = stmts->next;
    }
    
    /*
     * function is guaranteed to be non-empty, and every statement
     * is guaranteed to leave its result on the stack. so just emit
     * a final return (which will pop the stack) to return the result
     * of the last evaluated expression.
     */
    EMIT_OP(OP_RETURN);
    
    /* copy chunk of bytecode used by this function */
    size_t code_size = sizeof(inst_t) * (ctx->code_pos - code_start);
    fn->code = mnc_gc_alloc(ctx, code_size);
    if (!fn->code) memory_error();
    memcpy(fn->code, ctx->code + code_start, code_size);
    
    /* reset global code pos so it can be reused */
    ctx->code_pos = code_start;
    
    return fn;
    
}

COMPILE_FN(statement, ast_node_t) {
    switch (node->type) {
        case AST_WHILE:     return COMPILE(while, ast_while_t, node);
        case AST_FOR:       return COMPILE(for, ast_for_t, node);
        case AST_ASSIGN:    return COMPILE(assign, ast_assign_t, node);
        case AST_IF:        return COMPILE(if, ast_if_t, node);
        case AST_PASS:      return COMPILE(pass, ast_pass_t, node);
        case AST_RETURN:    return COMPILE(return, ast_return_t, node);
        default:            return 0;
    }
}

COMPILE_FN(while, ast_while_t) {
    return 0;
}

COMPILE_FN(for, ast_for_t) {
    return 0;
}

COMPILE_FN(assign, ast_assign_t) {
    return 0;
}

COMPILE_FN(if, ast_if_t) {
    return 0;
}

COMPILE_FN(pass, ast_pass_t) {
    return 0;
}

COMPILE_FN(return, ast_return_t) {
    return 0;
}


// COMPILE_FN(null) {
//     EMIT(OP_PUSH_NULL);
//     return 1;
// }
// 
// COMPILE_FN(int) {
//     int foo = 10;
//     EMIT(OP_PUSH);
//     EMIT(MK_INTVAL(foo));
//     return 2;
// }
// 
// COMPILE_FN(bool) {
//     int foo = 10;
//     EMIT(foo ? OP_PUSH_TRUE : OP_PUSH_FALSE);
//     return 1;
// }
// 
// COMPILE_FN(string) {
//     // insert string into string table
//     // plant make string opcode
// }
// 
// COMPILE_FN(color) {
//     // push r,g,b,a onto stack
//     // emit make_color
// }
// 
// COMPILE_FN(ident) {
//     // push 
// }
// 
// COMPILE_FN(symbol) {
//     
// }
// 
// COMPILE_FN(statements) {
//     int size = 0;
//     // iterate over each statement and compile
//     return size;
// }
// 
// COMPILE_FN(while) {
//     // get current pos
//     // walk expression
//     // EMIT(JMP_IF_FALSE)
//     // EMIT(0)
//     // mark current pos
//     // walk body
//     // EMIT(JMP)
//     // EMIT(delta)
//     // update EMIT(0) to point to current location
// }
// 
// COMPILE_FN(if_cons) {
//     
// }
// 
// COMPILE_FN(if) {
//     // compile expression
//     // emit jump-if-false 0
//     // size1 = compile statements
//     // emit jmp 0
//     // size2 = compile if-cons
//     // modify jump-if-false to = size1 + 2 + size2
//     // modify jmp 0 to = size2
// }
// 
// COMPILE_FN(pass) {
//     return 0;
// }
// 
// COMPILE_FN(binary_op) {
//     int size = 0;
//     int op;
//     
//     ast_id_t l, r;
//     
//     // compile left expression
//     // compile right expression
//     
//     switch (op) {
//         case T_OP_PLUS:
//             EMIT(OP_ADD);
//             break;
//         case T_OP_MINUS:
//             EMIT(OP_SUB);
//             break;
//         case T_OP_TIMES:
//             EMIT(OP_MUL);
//             break;
//         case T_OP_DIV:
//             EMIT(OP_DIV);
//             break;
//         case T_OP_POW:
//             /* TODO */
//             break;
//         case T_OP_EQ:
//             EMIT(OP_EQ);
//             break;
//         case T_OP_NEQ:
//             EMIT(OP_NEQ);
//             break;
//         case T_OP_LT:
//             EMIT(OP_LT);
//             break;
//         case T_OP_LTE:
//             EMIT(OP_LTE);
//             break;
//         case T_OP_GT:
//             EMIT(OP_GT);
//             break;
//         case T_OP_GTE:
//             EMIT(OP_GTE);
//             break;
//         default:
//             /* ERROR */
//     }
//     
//     return size;
// }
// 
// COMPILE_FN(unary_op) {
//     
// }
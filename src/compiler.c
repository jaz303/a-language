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
#define OFFSET              (CTX->code_pos)

/* compilation helpers */

#define COMPILE_FN(name, ast_type) static int compile_##name(cmpstate_t *cmpstate, ast_type *node)
#define COMPILE(name, ast_type, node) compile_##name(cmpstate, (ast_type*)node)

COMPILE_FN(statements,  ast_statements_t);
COMPILE_FN(statement,   ast_node_t);
COMPILE_FN(while,       ast_while_t);
COMPILE_FN(for,         ast_for_t);
COMPILE_FN(assign,      ast_assign_t);
COMPILE_FN(conditions,  ast_conditions_t);
COMPILE_FN(if,          ast_if_t);
COMPILE_FN(pass,        ast_pass_t);
COMPILE_FN(return,      ast_return_t);
COMPILE_FN(expression,  ast_node_t);

/* code emission helpers */

#define EMIT_OP(o)      emit_op(CTX, o)
#define EMIT_INT(i)     emit_int(CTX, i)
#define EMIT_UINT(u)    emit_uint(CTX, u)
#define EMIT_VALUE(v)   emit_value(CTX, v)
#define EMIT_INTERN(n)  emit_intern(CTX, n)

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

static void emit_intern(context_t *ctx, INTERN n) {
    if (ctx->code_pos == ctx->code_capacity) grow_code(ctx);
    ctx->code[ctx->code_pos++].n = n;
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
    int code_start = ctx->code_pos;
    
    /* compile function body */
    assert(ast->body);
    COMPILE(statements, ast_statements_t, ast->body);
    
    /*
     * function is guaranteed to be non-empty, and every statement
     * is guaranteed to leave its result on the stack. so just emit
     * a final return (which will pop the stack) to return the result
     * of the last evaluated expression.
     */
    EMIT_OP(OP_RETURN);
    
    /* copy chunk of bytecode used by this function */
    int code_size = sizeof(inst_t) * (ctx->code_pos - code_start);
    fn->code = mnc_gc_alloc(ctx, code_size);
    if (!fn->code) memory_error();
    memcpy(fn->code, ctx->code + code_start, code_size);
    
    /* reset global code pos so it can be reused */
    ctx->code_pos = code_start;
    
    return fn;
    
}

COMPILE_FN(statements, ast_statements_t) {
    while (node) {
        COMPILE(statement, ast_node_t, node->statement);
        node = node->next;
    }
    return 1;
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
    // TODO: probably riddled with off-by-one errors
    
    INT loop_start = OFFSET;
    
    COMPILE(expression, ast_node_t, node->condition);
    EMIT_OP(OP_JMP_IF_FALSE);
    
    INT jump_pos = OFFSET;
    EMIT_INT(0);
    
    COMPILE(statements, ast_statements_t, node->body);
    
    EMIT_OP(OP_JMP);
    EMIT_INT(loop_start - OFFSET);
    
    INT loop_after = OFFSET;
    CTX->code[jump_pos].i = loop_after - jump_pos;
    
    return 1;
}

COMPILE_FN(for, ast_for_t) {
    return 0;
}

COMPILE_FN(assign, ast_assign_t) {
    return 0;
}

COMPILE_FN(conditions, ast_conditions_t) {
    if (node == NULL) {
        return 0;
    } else if (node->exp) {
        
        INT cond_start,     /* start offset of this condition */
            skip_jump,      /* offset at which we must write the length to skip to hit next condition */
            body_start,     /* offset of condition body */
            end_jump,       /* offset at which we must write the length to skip to exit the if-statement */
            cond_len,       /* total length of this condition - expression + test + body */
            rest_len;       /* total length of successive conditions */
        
        cond_start = OFFSET;
        
        /* compile expression and test, leaving placeholder for jump offset */
        COMPILE(expression, ast_node_t, node->exp);
        EMIT_OP(OP_JMP_IF_FALSE);
        skip_jump = OFFSET;
        EMIT_INT(0);
        
        /* compile the body */
        body_start = OFFSET;
        COMPILE(statements, ast_statements_t, node->body);
        
        /* write correct jump value to land at next condition */
        CTX->code[skip_jump].i = OFFSET - body_start;
        
        /* write jump to exit if-statement, leaving placeholder for jump offset */
        if (node->next) {
            EMIT_OP(OP_JMP);
            end_jump = OFFSET;
            EMIT_INT(0);
        }
        
        /* calculate length of this condition and successive conditions */
        cond_len = OFFSET - cond_start;
        rest_len = COMPILE(conditions, ast_conditions_t, node->next);
        
        /* write correct jump value to exit the if-statement */
        if (node->next) {
            CTX->code[end_jump].i = rest_len;
        }
        
        /* return total length of this + successive conditions */
        return cond_len + rest_len;
    
    } else {
        INT start = OFFSET;
        COMPILE(statements, ast_statements_t, node->body);
        return OFFSET - start;
    }
}

COMPILE_FN(if, ast_if_t) {
    ast_conditions_t *cond = node->conditions;
    assert(cond);
    return compile_conditions(cmpstate, cond);
}

COMPILE_FN(pass, ast_pass_t) {
    return 1;
}

COMPILE_FN(return, ast_return_t) {
    if (node->exp) {
        COMPILE(expression, ast_node_t, node->exp);
    } else {
        EMIT_OP(OP_PUSH_NULL);
    }
    EMIT_OP(OP_RETURN);
    return 1;
}

COMPILE_FN(expression, ast_node_t) {
    switch (node->type) {
        case AST_LITERAL:
        {
            ast_literal_t *literal = (void*)node;
            if (literal->value == kNull) {
                EMIT_OP(OP_PUSH_NULL);
            } else if (literal->value == kTrue) {
                EMIT_OP(OP_PUSH_TRUE);
            } else if (literal->value == kFalse) {
                EMIT_OP(OP_PUSH_FALSE);
            } else {
                EMIT_OP(OP_PUSH);
                EMIT_VALUE(literal->value);
            }
            break;
        }
        case AST_STRING:
        {
            break;
        }
        case AST_IDENT:
        {
            ast_ident_t *id = (void*)node;
            EMIT_OP(OP_PUSH_LOCAL);
            EMIT_UINT(FRAME_SLOT(id->name));
            break;
        }
        case AST_ARRAY:
        {
            ast_literal_collection_t *ary = (void*)node;
            int nelems = 0;
            ast_array_members_t *curr = (ast_array_members_t*)ary->head;
            while (curr) {
                COMPILE(expression, ast_node_t, curr->value);
                nelems++;
            }
            EMIT_OP(OP_MAKE_ARRAY);
            EMIT_UINT(nelems);
            break;
        }
        case AST_DICT:
        {
            ast_literal_collection_t *dict = (void*)dict;
            int nelems = 0;
            ast_dict_members_t *curr = (ast_dict_members_t*)dict->head;
            while (curr) {
                COMPILE(expression, ast_node_t, curr->key);
                COMPILE(expression, ast_node_t, curr->value);
                nelems++;
            }
            EMIT_OP(OP_MAKE_DICT);
            EMIT_UINT(nelems);
            break;
        }
        case AST_UNARY_EXP:
        {
            ast_unary_exp_t *exp = (ast_unary_exp_t*)node;
            COMPILE(expression, ast_node_t, exp->exp);
            switch (exp->operator) {
                case T_PLUS:    /* do nothing*/     break;
                case T_MINUS:   EMIT_OP(OP_NEGATE); break;
                case T_B_NOT:   EMIT_OP(OP_BNOT);   break;
                case T_L_NOT:   EMIT_OP(OP_NOT);    break;
                default:
                    fatal_error("unknown unary operator");
                    break;
            }
            break;
        }
        case AST_BINARY_EXP:
        {
            ast_binary_exp_t *exp = (ast_binary_exp_t*)node;
            COMPILE(expression, ast_node_t, exp->rexp);
            COMPILE(expression, ast_node_t, exp->lexp);
            switch (exp->operator) {
                case T_PLUS:    EMIT_OP(OP_ADD);    break;
                case T_MINUS:   EMIT_OP(OP_SUB);    break;
                case T_TIMES:   EMIT_OP(OP_MUL);    break;
                case T_DIV:     EMIT_OP(OP_DIV);    break;
                case T_MOD:     EMIT_OP(OP_MOD);    break;
                case T_POW:     EMIT_OP(OP_POW);    break;
                case T_EQ:      EMIT_OP(OP_EQ);     break;
                case T_NEQ:     EMIT_OP(OP_NEQ);    break;
                case T_LT:      EMIT_OP(OP_LT);     break;
                case T_LTE:     EMIT_OP(OP_LTE);    break;
                case T_GT:      EMIT_OP(OP_GT);     break;
                case T_GTE:     EMIT_OP(OP_GTE);    break;
                case T_B_AND:   EMIT_OP(OP_BAND);   break;
                case T_B_OR:    EMIT_OP(OP_BOR);    break;
                case T_B_XOR:   EMIT_OP(OP_BXOR);   break;
                default:
                    /* logical or - needs to shortcircuit */
                    /* logical and - needs to shortcircuit */
                    fatal_error("sorry this operator is not implemented!");
                    break;
            }
            break;
        }
        case AST_SELECTOR:
        {
            fatal_error("selectors are not implemented");
            break;
        }
        case AST_INVOKE:
        {
            ast_invoke_t *inv = (void*)node;
            
            /* 
             * function invocation/parsing/everything is a mess
             * for example, function to invoke is always ident -
             * should be possible to invoke anything. this will
             * be necessary once it becomes possible to return
             * functions
             *
             * this means we actually need 4 invocation opcodes:
             *
             * CALL                     : name of function to call is on stack
             * CALL_IMMEDIATE           : name of function to call is in next instruction
             * CALL_METHOD              : name of method to call is on stack
             * CALL_METHOD_IMMEDIATE    : name of method to call is in next instruction
             */
            
            ast_expressions_t *args = inv->arguments;
            int nargs = 0;
            while (args) {
                COMPILE(expression, ast_node_t, args->exp);
                nargs++;
                args = args->next;
            }
            
            EMIT_OP(OP_PUSH);
            EMIT_UINT(nargs);
            
            if (inv->receiver) {
                fatal_error("method calls are not implemented");
            } else {
                EMIT_OP(OP_CALL);
                EMIT_INTERN(inv->name);
            }
            
            break;
        }
        case AST_INDEX:
        {
            ast_index_t *index = (void*)node;
            
            /*
             * Stack layout for index operation:
             *
             * receiver     : VALUE
             * n_args       : UINT
             * arg_n        : VALUE
             * arg_1        : VALUE
             * arg_0        : VALUE
             *
             * OP_INDEX implementation:
             *
             * 1. Pop receiver from stack
             * 2. Peek number of arguments from stack
             *    (peeking ensures same stack layout as regular function call)
             * 3. Perform indexing operation (either native call or VM call)
             * 4. Pop nargs and arguments from stack
             * 5. Push result
             */
        
            ast_expressions_t *args = index->arguments;
            int nargs = 0;
            while (args) {
                COMPILE(expression, ast_node_t, args->exp);
                nargs++;
                args = args->next;
            }
            
            EMIT_OP(OP_PUSH);
            EMIT_UINT(nargs);
            
            COMPILE(expression, ast_node_t, index->receiver);
            
            EMIT_OP(OP_INDEX);
            
            break;
        }
        default:
        {
            // TODO: error report
        }
    }
    return 1;
}

#include "menace/compiler.h"

#define COMPILE_FN(name) static int compile_##name(context_t *ctx, ast_id_t node)

COMPILE_FN(null) {
    EMIT(OP_PUSH_NULL);
    return 1;
}

COMPILE_FN(int) {
    int foo = 10;
    EMIT(OP_PUSH);
    EMIT(MK_INTVAL(foo));
    return 2;
}

COMPILE_FN(bool) {
    int foo = 10;
    EMIT(foo ? OP_PUSH_TRUE : OP_PUSH_FALSE);
    return 1;
}

COMPILE_FN(string) {
    // insert string into string table
    // plant make string opcode
}

COMPILE_FN(color) {
    // push r,g,b,a onto stack
    // emit make_color
}

COMPILE_FN(ident) {
    // push 
}

COMPILE_FN(symbol) {
    
}

COMPILE_FN(statements) {
    int size = 0;
    // iterate over each statement and compile
    return size;
}

COMPILE_FN(while) {
    // get current pos
    // walk expression
    // EMIT(JMP_IF_FALSE)
    // EMIT(0)
    // mark current pos
    // walk body
    // EMIT(JMP)
    // EMIT(delta)
    // update EMIT(0) to point to current location
}

COMPILE_FN(if_cons) {
    
}

COMPILE_FN(if) {
    // compile expression
    // emit jump-if-false 0
    // size1 = compile statements
    // emit jmp 0
    // size2 = compile if-cons
    // modify jump-if-false to = size1 + 2 + size2
    // modify jmp 0 to = size2
}

COMPILE_FN(pass) {
    return 0;
}

COMPILE_FN(binary_op) {
    int size = 0;
    int op;
    
    ast_id_t l, r;
    
    // compile left expression
    // compile right expression
    
    switch (op) {
        case T_OP_PLUS:
            EMIT(OP_ADD);
            break;
        case T_OP_MINUS:
            EMIT(OP_SUB);
            break;
        case T_OP_TIMES:
            EMIT(OP_MUL);
            break;
        case T_OP_DIV:
            EMIT(OP_DIV);
            break;
        case T_OP_POW:
            /* TODO */
            break;
        case T_OP_EQ:
            EMIT(OP_EQ);
            break;
        case T_OP_NEQ:
            EMIT(OP_NEQ);
            break;
        case T_OP_LT:
            EMIT(OP_LT);
            break;
        case T_OP_LTE:
            EMIT(OP_LTE);
            break;
        case T_OP_GT:
            EMIT(OP_GT);
            break;
        case T_OP_GTE:
            EMIT(OP_GTE);
            break;
        default:
            /* ERROR */
    }
    
    return size;
}

COMPILE_FN(unary_op) {
    
}
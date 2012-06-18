#include "menace/vm.h"
#include "menace/array.h"

static void *jump_table[OPCODE_MAX];
#define OPCODE(code) jump_table[OP_##code] = &&LBL_##code;

#define NEXT()              { goto *jump_table[inst[inst_ptr++].o]; }
#define PUSH(v)             ((stack[++stack_ptr]) = ((VALUE)v))
#define POP()               (stack[stack_ptr--])
#define CURR()              (stack[stack_ptr])
#define TAKE_VALUE()        (inst[inst_ptr++].v)
#define TAKE_UINT()         (inst[inst_ptr++].u)
#define TAKE_INT()          (inst[inst_ptr++].i)
#define GET_LOCAL(ix)       locals[(ix)]
#define SET_LOCAL(ix, v)    locals[(ix)] = (v)
#define CONTEXT             NULL
#define LINE                current_line
#define SET_LINE(l)         (current_line = l)

#define BIN_OP(operator) \
    VALUE l = POP(); \
    VALUE r = POP(); \
    if (VALUE_IS_INT(l)) { \
        if (VALUE_IS_INT(r)) { \
            INT lc = INTVAL(l); \
            INT rc = INTVAL(r); \
            INT res = lc operator rc; \
            PUSH(MK_INTVAL(res)); \
        } else if (VALUE_IS_FLOAT(r)) { \
            REAL lc = (REAL) INTVAL(l); \
            REAL rc = FLOATVAL(r); \
            REAL res = lc operator rc; \
            PUSH(MK_FLOATVAL(CONTEXT, res)); \
        } else { \
            /* TODO: error? or operator overload? */ \
        } \
    } else if (VALUE_IS_FLOAT(l)) { \
        if (VALUE_IS_INT(r)) { \
            REAL lc = FLOATVAL(l); \
            REAL rc = (REAL) INTVAL(r); \
            REAL res = lc operator rc; \
            PUSH(MK_FLOATVAL(CONTEXT, res)); \
        } else if (VALUE_IS_FLOAT(r)) { \
            REAL lc = FLOATVAL(l); \
            REAL rc = FLOATVAL(r); \
            REAL res = lc operator rc; \
            PUSH(MK_FLOATVAL(CONTEXT, res)); \
        } \
    } else { \
        /* TODO: error? */ \
    }
    
#define CMP_OP(operator) \
    VALUE l = POP(); \
    VALUE r = POP(); \
    if (VALUE_IS_INT(l) && VALUE_IS_INT(r)) { \
        INT lc = INTVAL(l); \
        INT rc = INTVAL(r); \
        if (lc operator rc) { PUSH(kTrue); } else { PUSH(kFalse); } \
    } else if (VALUE_IS_INT(l) && VALUE_IS_FLOAT(r)) { \
        REAL lc = (REAL) INTVAL(l); \
        REAL rc = FLOATVAL(r); \
        if (lc operator rc) { PUSH(kTrue); } else { PUSH(kFalse); } \
    } else if (VALUE_IS_FLOAT(l) && VALUE_IS_INT(r)) { \
        REAL lc = FLOATVAL(l); \
        REAL rc = (REAL) INTVAL(r); \
        if (lc operator rc) { PUSH(kTrue); } else { PUSH(kFalse); } \
    } else if (VALUE_IS_FLOAT(l) && VALUE_IS_FLOAT(r)) { \
        REAL lc = FLOATVAL(l); \
        REAL rc = FLOATVAL(r); \
        if (lc operator rc) { PUSH(kTrue); } else { PUSH(kFalse); } \
    } else { \
        /* TODO: error */ \
    }
    
void vm_exec() {
    
    UINT        current_line        = 0;
    VALUE       *stack;
    UINT        stack_ptr           = 0;
    inst_t      *inst;
    int         inst_ptr            = 0;
    VALUE       *locals;
    
    static int jump_table_init = 0;
    if (!jump_table_init) { // TODO: threadsafety
        #include "menace/opcodes.x"
        jump_table_init = 1;
    }
    
    LBL_NOP:
    {
        NEXT();
    }
    LBL_CALL:
    {
        VALUE function = POP();
        if (IS_OBJECT(function)) {
            if (OBJ_IS_NATIVE_FUNCTION(function)) {
                
            } else if (OBJ_IS_FUNCTION(function)) {
                
            }
        }
        NEXT();
    }
    LBL_RETURN:
    {
        NEXT();
    }
    LBL_PUSH:           { VALUE v = TAKE_VALUE(); PUSH(v); NEXT(); }
    LBL_PUSH_TRUE:      { PUSH(kTrue); NEXT(); }
    LBL_PUSH_FALSE:     { PUSH(kFalse); NEXT(); }
    LBL_PUSH_NULL:      { PUSH(kNull); NEXT(); }
    LBL_PUSH_LOCAL:     { UINT ix = TAKE_UINT(); PUSH(GET_LOCAL(ix)); NEXT(); }
    LBL_PUSH_GLOBAL:
    {
        NEXT();
    }
    LBL_SET_LOCAL:
    {
        UINT ix = TAKE_UINT();
        /* no pop - assignment evaluates to val */
        SET_LOCAL(ix, CURR());
        NEXT();
    }
    LBL_SET_GLOBAL:
    {
        NEXT();
    }
    LBL_JMP:
    {
        INT delta = TAKE_INT();
        inst_ptr += delta;
        NEXT();
    }
    LBL_JMP_IF_TRUE:
    {
        INT delta = TAKE_INT();
        VALUE cond = POP();
        if (MTEST(cond)) {
            inst_ptr += delta;
        }
        NEXT();
    }
    LBL_JMP_IF_FALSE:
    {
        INT delta = TAKE_INT();
        VALUE cond = POP();
        if (!MTEST(cond)) {
            inst_ptr += delta;
        }
        NEXT();
    }
    LBL_ADD:    { BIN_OP(+);  NEXT(); }
    LBL_SUB:    { BIN_OP(-);  NEXT(); }
    LBL_MUL:    { BIN_OP(*);  NEXT(); }
    LBL_DIV:    { BIN_OP(/);  NEXT(); }
    LBL_GT:     { CMP_OP(>);  NEXT(); }
    LBL_GTE:    { CMP_OP(>=); NEXT(); }
    LBL_LT:     { CMP_OP(<);  NEXT(); }
    LBL_LTE:    { CMP_OP(<=); NEXT(); }
    LBL_EQ:
    {
        /* Needs work */
        
        VALUE l = POP();
        VALUE r = POP();
        
        if (l == r) {
            /* covers: ints, objects, symbols, true, false, nil */
            PUSH(kTrue);
        } else if (VALUE_IS_INT(l) && VALUE_IS_FLOAT(r)) {
            REAL lc     = (REAL) INTVAL(l);
            REAL rc     = FLOATVAL(r);
            VALUE res   = (lc == rc) ? kTrue : kFalse;
            PUSH(res);
        } else if (VALUE_IS_FLOAT(l) && VALUE_IS_INT(r)) {
            REAL lc     = FLOATVAL(l);
            REAL rc     = (REAL) INTVAL(r);
            VALUE res   = (lc == rc) ? kTrue : kFalse;
            PUSH(res);
        } else {
            PUSH(kFalse);
        }
        
        NEXT();
    }
    LBL_NEQ:
    {
        /* Again, needs work */
        
        VALUE l = POP();
        VALUE r = POP();
        
        if (l == r) {
            PUSH(kFalse);
        } else if (VALUE_IS_INT(l) && VALUE_IS_FLOAT(r)) {
            REAL lc     = (REAL) INTVAL(l);
            REAL rc     = FLOATVAL(r);
            VALUE res   = (lc != rc) ? kTrue : kFalse;
            PUSH(res);
        } else if (VALUE_IS_FLOAT(l) && VALUE_IS_INT(r)) {
            REAL lc     = FLOATVAL(l);
            REAL rc     = (REAL) INTVAL(r);
            VALUE res   = (lc != rc) ? kTrue : kFalse;
            PUSH(res);
        } else {
            PUSH(kTrue);
        }
    }
    LBL_INDEX:
    {
        VALUE receiver = POP();
        UINT arg_count = (UINT) POP();
        if (VALUE_IS_ARRAY(receiver)) {
            if (arg_count == 1) {
                VALUE ix = POP();
                if (VALUE_IS_INT(ix)) {
                    VALUE val = array_get_index(CONTEXT, (obj_array_t*)receiver, INTVAL(ix));
                    PUSH(val);
                } else {
                    // error
                }
            } else {
                // error
            }
        } else if (VALUE_IS_DICT(receiver)) {
            if (arg_count == 1) {
                
            } else {
                // error
            }
        } else if (VALUE_IS_OBJECT(receiver)) {
            // dispatch to object
        } else {
            // error
        }
        NEXT();
    }
    LBL_INDEX_SET:
    {
        NEXT();
    }
    LBL_MAKE_ARRAY:
    {
        UINT size = (UINT) POP();
        obj_array_t *ary = array_create_with_capacity(CONTEXT, size);
        for (UINT i = 0; i < size; i++) {
            VALUE val = POP();
            array_push(CONTEXT, ary, val);
        }
        PUSH(ary);
        NEXT();
    }
    LBL_MAKE_DICT:
    {
        UINT size = (UINT) POP();
        // create dict
        for (UINT i = 0; i < size; i++) {
            // VALUE key = POP();
            // VALUE value = POP();
            // dict_set(CONTEXT, dict, key, value)
        }
        // PUSH(dict)
        NEXT();
    }
    LBL_LINE:
    {
        UINT line = TAKE_UINT();
        SET_LINE(line);
        NEXT();
    }
    LBL_BREAKPOINT:
    {
        NEXT();
    }
    LBL_HALT:
    {
    }
}
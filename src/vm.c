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
    LBL_RETURN:
    {
        NEXT();
    }
    LBL_PUSH:
    {
        VALUE v = TAKE_VALUE();
        PUSH(v);
        NEXT();
    }
    LBL_PUSH_TRUE:
    {
        PUSH(kTrue);
        NEXT();
    }
    LBL_PUSH_FALSE:
    {
        PUSH(kFalse);
        NEXT();
    }
    LBL_PUSH_NULL:
    {
        PUSH(kNull);
        NEXT();
    }
    LBL_PUSH_LOCAL:
    {
        UINT ix = TAKE_UINT();
        PUSH(GET_LOCAL(ix));
        NEXT();
    }
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
        UINT delta = TAKE_INT();
        inst_ptr += delta;
        NEXT();
    }
    LBL_JMP_IF_TRUE:
    {
        UINT delta = TAKE_INT();
        VALUE cond = POP();
        if (VALUE_IS_TRUTHY(cond)) {
            inst_ptr += delta;
        }
        NEXT();
    }
    LBL_JMP_IF_FALSE:
    {
        UINT delta = TAKE_INT();
        VALUE cond = POP();
        if (!VALUE_IS_TRUTHY(cond)) {
            inst_ptr += delta;
        }
        NEXT();
    }
    LBL_ADD:
    {
        VALUE l = POP();
        VALUE r = POP();
        if (VALUE_IS_INT(l)) {
            if (VALUE_IS_INT(r)) {
                PUSH(MK_INTVAL(INTVAL(l) + INTVAL(r)));
            } else if (VALUE_IS_FLOAT(r)) {
                // TODO
            } else {
                // TODO
            }
        } else if (VALUE_IS_FLOAT(l)) {
            
        } else {
            
        }
        NEXT();
    }
    LBL_SUB:
    {
        NEXT();
    }
    LBL_MUL:
    {
        NEXT();
    }
    LBL_DIV:
    {
        NEXT();
    }
    LBL_GT:
    {
        NEXT();
    }
    LBL_GTE:
    {
        NEXT();
    }
    LBL_LT:
    {
        NEXT();
    }
    LBL_LTE:
    {
        NEXT();
    }
    LBL_EQ:
    {
        NEXT();
    }
    LBL_NEQ:
    {
        NEXT();
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
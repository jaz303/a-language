#include "menace/parser.h"
#include "menace/scanner.h"
#include "menace/intern.h"

#include <string.h>

static ast_statements_t*    parse_program(parser_t *p);
static ast_statements_t*    parse_statement_block(parser_t *p);
static ast_node_t*          parse_statement(parser_t *p);
static ast_node_t*          parse_if(parser_t *p);
static ast_conditions_t*    parse_condition(parser_t *p);
static ast_node_t*          parse_while(parser_t *p);
static ast_node_t*          parse_pass(parser_t *p);
static ast_node_t*          parse_return(parser_t *p);
static ast_node_t*          parse_named_function_def(parser_t *p);
static ast_node_t*          parse_expression_or_assign(parser_t *p);
static ast_parameters_t*    parse_parameters(parser_t *p);
static ast_node_t*          parse_expression(parser_t *p);
static ast_node_t*          parse_logical_or_exp(parser_t *p);
static ast_node_t*          parse_logical_and_exp(parser_t *p);
static ast_node_t*          parse_bitwise_exp(parser_t *p);
static ast_node_t*          parse_equality_exp(parser_t *p);
static ast_node_t*          parse_cmp_exp(parser_t *p);
static ast_node_t*          parse_additive_exp(parser_t *p);
static ast_node_t*          parse_multiplicative_exp(parser_t *p);
static ast_node_t*          parse_arithmetic_unary_exp(parser_t *p);
static ast_node_t*          parse_other_unary_exp(parser_t *p);
static ast_node_t*          parse_primary(parser_t *p);
static ast_node_t*          parse_selector(parser_t *p);
static ast_node_t*          parse_value(parser_t *p);
static ast_node_t*          parse_array(parser_t *p);
static ast_node_t*          parse_dict(parser_t *p);
static ast_expressions_t*   parse_expression_list(parser_t *p);

#define NEXT() \
    p->current_token = scanner_get_next_token(p->scanner, &p->current_text, &p->current_len);
    
/* Public API */

int parser_init(parser_t *p, context_t *c, scanner_t *s) {
    p->context = c;
    p->scanner = s;
    p->error = NULL;
    NEXT();
    return 1;
}

ast_statements_t* parser_parse(parser_t *p) {
    return parse_program(p);
}

/* Private */

#define ACCEPT(token, err) \
    if (CURR() != token) { \
        p->error = err; \
        return NULL; \
    } else { \
        NEXT(); \
    }

#define CTX                     p->context
#define TEXT                    p->current_text
#define CURR()                  p->current_token
#define CONSUME(token)          while (CURR() == token) NEXT()
#define AST_MAKE(type, ...)     ast_mk_##type(p->context, ##__VA_ARGS__)
#define SKIP()                  CONSUME(T_WHITESPACE)
#define BLOCK_PRELUDE()         { parse_block_prelude(p); ERROR_CHECK(); }
#define ACCEPT_EOL()            ACCEPT(T_EOL, "expected EOL")
#define ERROR_CHECK()           if (p->error) return NULL;
#define ERROR(msg)              { p->error = msg; return NULL; }

#define PARSE_CHILD(type, name, fn) \
    type *name = parse_##fn(p); \
    ERROR_CHECK();

static INT decode_integer(const char *str) {
    // TODO: detect overflow
    INT val = 0;
    int len = strlen(str);
    if (len > 2 && str[1] == 'x') {
        for (str+=2;*str;str++) {
            if (*str == '_') continue;
            val *= 16;
            if (*str >= 'a')        val += (*str - 'a' + 10);
            else if (*str >= 'A')   val += (*str - 'A' + 10);
            else                    val += (*str - '0');
        }
    } else if (len > 2 && str[1] == 'o') {
        for (str+=2;*str;str++) {
            if (*str == '_') continue;
            val *= 8;
            val += (*str) - '0';
        }
    } else if (len > 2 && str[1] == 'b') {
        for (str+=2;*str;str++) {
            if (*str == '_') continue;
            val *= 2;
            val += (*str) - '0';
        }
    } else {
        for (;*str;str++) {
            if (*str == '_') continue;
            val *= 10;
            val += (*str) - '0';
        }
    }
    return val;
}

static char* decode_string(char *str) {
    return str;
}

static void* parse_block_prelude(parser_t *p) {
    SKIP();
    ACCEPT(T_COLON, "expected `:`");
    SKIP();
    ACCEPT_EOL();
    return NULL;
}

ast_statements_t* parse_program(parser_t *p) {
    return parse_statement_block(p);
}

ast_statements_t* parse_statement_block(parser_t *p) {
    ACCEPT(T_INDENT, "expected <indent>");
    CONSUME(T_EOL);
    
    ast_statements_t *head = NULL, *curr = NULL;
    
    while (CURR() != T_OUTDENT) {
        ast_statements_t *stmts = AST_MAKE(statements, parse_statement(p));
        if (head == NULL) {
            head = stmts;
        }
        if (curr) {
            ast_cons_statements(curr, stmts);
            curr = stmts;
        }
        CONSUME(T_EOL);
    }

    ACCEPT(T_OUTDENT, "expected <outdent>");
    
    return head;
}

ast_node_t* parse_statement(parser_t *p) {
    if (CURR() == T_IF) {
        return parse_if(p);
    } else if (CURR() == T_WHILE) {
        return parse_while(p);
    } else if (CURR() == T_PASS) {
        return parse_pass(p);
    } else if (CURR() == T_RETURN) {
        return parse_return(p);
    } else if (CURR() == T_DEF) {
        return parse_named_function_def(p);
    } else {
        return parse_expression_or_assign(p);
    }
}

ast_node_t* parse_if(parser_t *p) {
    
    ACCEPT(T_IF, "expected `if`");
    SKIP();
    
    ast_conditions_t *head = parse_condition(p);
    ERROR_CHECK();
    
    ast_conditions_t *curr = head;
    
    while (CURR() == T_ELSEIF) {
        NEXT();
        SKIP();
        ast_conditions_t *cond = parse_condition(p);
        ERROR_CHECK();
        ast_cons_cond(curr, cond);
        curr = cond;
    }
    
    if (CURR() == T_ELSE) {
        NEXT();
        BLOCK_PRELUDE();
        ast_statements_t *stmts = parse_statement_block(p);
        ERROR_CHECK();
        ast_conditions_t *cond = AST_MAKE(condition, NULL, stmts);
        ast_cons_cond(curr, cond);
    }
    
    return AST_MAKE(if, head);
    
}

ast_conditions_t* parse_condition(parser_t *p) {
    PARSE_CHILD(ast_node_t, exp, expression);
    SKIP();
    BLOCK_PRELUDE();
    PARSE_CHILD(ast_statements_t, body, statement_block);
    return AST_MAKE(condition, exp, body);
}

ast_node_t* parse_while(parser_t *p) {
    ACCEPT(T_WHILE, "expected `while`");
    SKIP();
    PARSE_CHILD(ast_node_t, condition, expression);
    BLOCK_PRELUDE();
    PARSE_CHILD(ast_statements_t, body, statement_block);
    return AST_MAKE(while, condition, body);
}

ast_node_t* parse_pass(parser_t *p) {
    ACCEPT(T_PASS, "expected `pass`");
    SKIP();
    ACCEPT_EOL();
    return AST_MAKE(pass);
}

ast_node_t* parse_return(parser_t *p) {
    ACCEPT(T_RETURN, "expected `return`");
    SKIP();
    if (CURR() == T_EOL) {
        NEXT();
        return AST_MAKE(return, NULL);
    } else {
        PARSE_CHILD(ast_node_t, exp, expression);
        SKIP();
        ACCEPT_EOL();
        return AST_MAKE(return, exp);
    }
}

ast_node_t* parse_named_function_def(parser_t *p) {
    ACCEPT(T_DEF, "expected `def`");
    SKIP();
    
    if (CURR() != T_IDENT) {
        ACCEPT(T_IDENT, "expected function name identifier");
    }
    
    INTERN name = string_to_intern(CTX, TEXT);
    NEXT();
    SKIP();
    
    ast_parameters_t *params = NULL;
    if (CURR() == T_L_PAREN) {
        params = parse_parameters(p);
        ERROR_CHECK();
    }
    
    BLOCK_PRELUDE();
    
    PARSE_CHILD(ast_statements_t, body, statement_block);
    
    return AST_MAKE(named_function, name, params, body);
}

ast_node_t* parse_expression_or_assign(parser_t *p) {
    PARSE_CHILD(ast_node_t, lval, expression);
    SKIP();
    if (CURR() == T_EQUALS) {
        NEXT();
        SKIP();
        PARSE_CHILD(ast_node_t, rval, expression);
        return AST_MAKE(assign, lval, rval);
    } else {
        ACCEPT_EOL();
        return lval;
    }
}

ast_parameters_t *parse_parameters(parser_t *p) {
    ACCEPT(T_L_PAREN, "expected `(`");
    SKIP();
    
    ast_parameters_t *head = NULL, *curr = NULL;
    while (CURR() == T_IDENT) {
        ast_parameters_t *param = AST_MAKE(parameters, string_to_intern(CTX, TEXT));
        if (head == NULL) head = param;
        if (curr != NULL) ast_cons_parameters(curr, param);
        curr = param;
        NEXT();
        SKIP();
        if (CURR() == T_COMMA) {
            NEXT();
            SKIP();
        } else {
            break;
        }
    }
    
    SKIP();
    ACCEPT(T_R_PAREN, "expected `)`");
    
    return head;
}

ast_node_t* parse_expression(parser_t *p) {
    return parse_logical_or_exp(p);
}

ast_node_t* parse_logical_or_exp(parser_t *p) {
    PARSE_CHILD(ast_node_t, l, logical_and_exp);
    SKIP();
    while (CURR() == T_DBL_PIPE || CURR() == T_OR) {
        PARSE_CHILD(ast_node_t, r, logical_and_exp);
        l = AST_MAKE(binary_exp, T_L_OR, l, r);
        SKIP();
    }
    return l;
}

ast_node_t* parse_logical_and_exp(parser_t *p) {
    PARSE_CHILD(ast_node_t, l, bitwise_exp);
    SKIP();
    while (CURR() == T_DBL_AMPERSAND || CURR() == T_AND) {
        PARSE_CHILD(ast_node_t, r, bitwise_exp);
        l = AST_MAKE(binary_exp, T_L_AND, l, r);
        SKIP();
    }
    return l;
}

ast_node_t* parse_bitwise_exp(parser_t *p) {
    PARSE_CHILD(ast_node_t, l, equality_exp);
    SKIP();
    while (CURR() == T_AMPERSAND || CURR() == T_PIPE || CURR() == T_HAT) {
        PARSE_CHILD(ast_node_t, r, equality_exp);
        token_t op;
        if (CURR() == T_AMPERSAND)      op = T_B_AND;
        else if (CURR() == T_PIPE)      op = T_B_OR;
        else if (CURR() == T_HAT)       op = T_B_XOR;
        l = AST_MAKE(binary_exp, op, l, r);
        SKIP();
    }
    return l;
}

ast_node_t* parse_equality_exp(parser_t *p) {
    PARSE_CHILD(ast_node_t, l, cmp_exp);
    SKIP();
    while (CURR() == T_EQ || CURR() == T_NEQ) {
        PARSE_CHILD(ast_node_t, r, cmp_exp);
        l = AST_MAKE(binary_exp, CURR(), l, r);
        SKIP();
    }
    return l;
}

ast_node_t* parse_cmp_exp(parser_t *p) {
    PARSE_CHILD(ast_node_t, l, additive_exp);
    SKIP();
    while (CURR() == T_LT || CURR() == T_LTE || CURR() == T_GT || CURR() == T_GTE) {
        PARSE_CHILD(ast_node_t, r, additive_exp);
        l = AST_MAKE(binary_exp, CURR(), l, r);
        SKIP();
    }
    return l;    
}

ast_node_t* parse_additive_exp(parser_t *p) {
    PARSE_CHILD(ast_node_t, l, multiplicative_exp);
    SKIP();
    while (CURR() == T_PLUS || CURR() == T_MINUS) {
        PARSE_CHILD(ast_node_t, r, multiplicative_exp);
        l = AST_MAKE(binary_exp, CURR(), l, r);
        SKIP();
    }
    return l;    
}

ast_node_t* parse_multiplicative_exp(parser_t *p) {
    PARSE_CHILD(ast_node_t, l, arithmetic_unary_exp);
    SKIP();
    while (CURR() == T_TIMES || CURR() == T_DIV || CURR() == T_MOD || CURR() == T_POW) {
        PARSE_CHILD(ast_node_t, r, arithmetic_unary_exp);
        l = AST_MAKE(binary_exp, CURR(), l, r);
        SKIP();
    }
    return l;
}

ast_node_t* parse_arithmetic_unary_exp(parser_t *p) {
    if (CURR() == T_PLUS || CURR() == T_MINUS) {
        token_t op = CURR();
        NEXT();
        SKIP();
        PARSE_CHILD(ast_node_t, exp, multiplicative_exp);
        return AST_MAKE(unary_exp, op, exp);
    } else {
        return parse_other_unary_exp(p);
    }
}

ast_node_t* parse_other_unary_exp(parser_t *p) {
    if (CURR() == T_BANG || CURR() == T_TILDE) {
        token_t op = (CURR() == T_BANG) ? T_L_NOT : T_B_NOT;
        NEXT();
        SKIP();
        PARSE_CHILD(ast_node_t, exp, other_unary_exp);
        return AST_MAKE(unary_exp, op, exp);
    } else {
        PARSE_CHILD(ast_node_t, primary, primary);
        if (CURR() == T_DOT || CURR() == T_L_BRACKET) {
            return parse_selector(p);
        }
        return primary;
    }
}

ast_node_t* parse_primary(parser_t *p) {
    if (CURR() == T_L_PAREN) {
        NEXT();
        SKIP();
        PARSE_CHILD(ast_node_t, exp, expression);
        SKIP();
        ACCEPT(T_R_PAREN, "expected `)`");
        return exp;
    } else if (CURR() == T_IDENT) {
        ast_node_t *ident = AST_MAKE(ident, string_to_intern(CTX, TEXT));
        NEXT();
        return ident;
    } else {
        return parse_value(p);
    }
}

ast_node_t* parse_selector(parser_t *p) {
    if (CURR() == T_DOT) {
        NEXT();
        if (CURR() != T_IDENT) {
            ACCEPT(T_IDENT, "expected identifier");
        }
        INTERN name = string_to_intern(CTX, TEXT);
        (void)name;
        NEXT();
        if (CURR() == T_L_PAREN) {
            ast_expressions_t *args = NULL;
            NEXT();
            SKIP();
            if (CURR() == T_R_PAREN) {
                NEXT();
            } else {
                args = parse_expression_list(p);
                ERROR_CHECK();
                SKIP();
                ACCEPT(T_R_PAREN, "expected `)`");
            }
            // TODO: return method call
        } else {
            // TODO: return property access
        }
        return NULL;
    } else if (CURR() == T_L_BRACKET) {
        ast_expressions_t *args = NULL;
        NEXT();
        SKIP();
        if (CURR() == T_R_BRACKET) {
            NEXT();
        } else {
            args = parse_expression_list(p);
            ERROR_CHECK();
            SKIP();
            ACCEPT(T_R_BRACKET, "expected `]`");
        }
        // TODO: return indexing
        return NULL;
    } else {
        ERROR("expected selector (property access, method call or index)")
    }
    return NULL;
}

ast_node_t* parse_value(parser_t *p) {
    if (CURR() == T_INTEGER) {
        INT val = decode_integer(TEXT);
        NEXT();
        return AST_MAKE(integer, val);
    } else if (CURR() == T_STRING) {
        char *str = decode_string(TEXT);
        // TODO: no, this string will get clobbered when NEXT() runs
        NEXT();
        return AST_MAKE(string, str);
    } else if (CURR() == T_SYMBOL) {
        INTERN name = string_to_intern(CTX, TEXT);
        NEXT();
        return AST_MAKE(symbol, name);
    } else if (CURR() == T_TRUE) {
        NEXT();
        return AST_MAKE(false);
    } else if (CURR() == T_FALSE) {
        NEXT();
        return AST_MAKE(true);
    } else if (CURR() == T_L_BRACKET) {
        return parse_array(p);
    } else if (CURR() == T_L_BRACE) {
        return parse_dict(p);
    } else {
        ERROR("expected value");
    }
}

ast_node_t* parse_array(parser_t *p) {
    ACCEPT(T_L_BRACKET, "expected `[`");
    SKIP();
    if (CURR() == T_R_BRACKET) {
        NEXT();
        return AST_MAKE(empty_array);
    } else {
        ast_array_members_t *head = NULL, *curr = NULL;
        while (1) {
            PARSE_CHILD(ast_node_t, value, expression);
            ast_array_members_t *ele = AST_MAKE(array_members, value);
            if (!ele) ERROR("failed to allocate storage for array member");
            if (head == NULL) {
                head = ele;
            } else {
                ast_cons_array_members(curr, ele);
            }
            curr = ele;
            SKIP();
            if (CURR() == T_COMMA) {
                NEXT();
                SKIP();
            } else {
                break;
            }
        }
        ACCEPT(T_R_BRACKET, "expected `]`");
        return AST_MAKE(array, head);
    }
}

ast_node_t* parse_dict(parser_t *p) {
    ACCEPT(T_L_BRACE, "expected `{`");
    SKIP();
    if (CURR() == T_R_BRACE) {
        NEXT();
        return AST_MAKE(empty_dict);
    } else {
        ast_dict_members_t *head = NULL, *curr = NULL;
        while (1) {
            PARSE_CHILD(ast_node_t, key, expression);
            SKIP();
            ACCEPT(T_HASHROCKET, "expected `=>`");
            SKIP();
            PARSE_CHILD(ast_node_t, value, expression);
            ast_dict_members_t *ele = AST_MAKE(dict_members, key, value);
            if (!ele) ERROR("failed to allocate storage for dictionary member");
            if (head == NULL) {
                head = ele;
            } else {
                ast_cons_dict_members(curr, ele);
            }
            curr = ele;
            SKIP();
            if (CURR() == T_COMMA) {
                NEXT();
                SKIP();
            } else {
                break;
            }
        }
        ACCEPT(T_R_BRACE, "expected `}`");
        return AST_MAKE(dict, head);
    }    
}

ast_expressions_t* parse_expression_list(parser_t *p) {
    ast_node_t *exp = parse_expression(p);
    ERROR_CHECK();
    
    ast_expressions_t *head = AST_MAKE(expressions, exp);
    ast_expressions_t *curr = head;
    
    SKIP();
    
    while (CURR() == T_COMMA) {
        NEXT();
        SKIP();
        exp = parse_expression(p);
        ERROR_CHECK();
        ast_expressions_t *tmp = AST_MAKE(expressions, exp);
        ast_cons_expressions(curr, tmp);
        curr = tmp;
        SKIP();
    }

    return head;
}
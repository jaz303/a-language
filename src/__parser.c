#include "menace/parser.h"
#include "menace/global.h"

#define PARSE_ERROR(msg) \
    p->error = msg; \
    if (node != 0) ast_destroy(p->context, node); \
    return 0;

#define NO_NODE() \
    ast_id_t node = 0;

#define NODE_TYPE(t) \
    ast_id_t node = ast_create_node(p->context, t); \
    if (!node) return 0; \
    ast_id_t curr = node; \
    (void)curr; /* suppress unused var warning */


#define CURR()              (p->current_token)
#define SKIP()              { while (CURR() == T_WHITESPACE) { NEXT(); } }
#define NEXT()              (p->current_token = scanner_get_next_token(p->scanner, &p->token_text, &p->token_len))
#define ACCEPT(t, err) \
    if (p->current_token == t) { \
        NEXT(); \
    } else { \
        if (node != 0) ast_destroy(p->context, node); \
        p->error = err; \
        return 0; \
    }

#define APPEND(node) \
    curr = ast_append_node(p->context, curr, node)
    
#define PARSE_CHILD(type) \
    { \
        ast_id_t type##_node = parse_##type(p); \
        if (type##_node == 0) { \
            ast_destroy(p->context, node); \
            return 0; \
        } \
        APPEND(type##_node); \
    }
    
#define BLOCK_PRELUDE() \
    SKIP(); \
    ACCEPT(T_COLON, "expecting `:`"); \
    SKIP(); \
    ACCEPT(T_EOL, "expecting newline");
    
static int decode_int(parser_t *p, const char *str, INT *out) {
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
    *out = val;
    return 1;
}

static int decode_ident(parser_t *p, const char *str, INTERN *out) {
    // TODO: put string into symbol table
    *out = 4;
    return 1;
}

static int decode_string(parser_t *p, const char *str, char **out) {
    // TODO: decode string into characters
    return 1;
}

static ast_id_t parse_statement_block(parser_t *p);
static ast_id_t parse_statement(parser_t *p);
static ast_id_t parse_while(parser_t *p);
static ast_id_t parse_if(parser_t *p);
static ast_id_t parse_function_definition(parser_t *p);
static ast_id_t parse_expression(parser_t *p);
static ast_id_t parse_value(parser_t *p);
static ast_id_t parse_anonymous_function(parser_t *p);
static ast_id_t parse_parameter_list(parser_t *p);

int parser_init(parser_t *p, context_t *c, scanner_t *s) {
    p->context = c;
    p->scanner = s;
    NEXT();
    return 1;
}

ast_id_t parser_parse(parser_t *p) {
    return parse_statement_block(p);
}

ast_id_t parse_statement_block(parser_t *p) {
    
    NODE_TYPE(AST_STATEMENTS);

    ACCEPT(T_INDENT, "expecting indent");
    
    while (CURR() == T_EOL) NEXT();
    while (CURR() != T_OUTDENT) {
        PARSE_CHILD(statement);
        while (CURR() == T_EOL) NEXT();
    }
    
    ACCEPT(T_OUTDENT, "expecting outdent");
    
    return node;

}

// reads a statement, including the statement terminator (currently only a newline)
// should statement terminators be expanded to include a semicolon, trailing whitespace
// thereafter will also be consumed
ast_id_t parse_statement(parser_t *p) {
    NO_NODE();
    
    if (CURR() == T_WHILE) {
        return parse_while(p);
    } else if (CURR() == T_IF) {
        return parse_if(p);
    } else if (CURR() == T_DEF) {
        return parse_function_definition(p);
    } else if (CURR() == T_PASS) {
        NEXT();
        SKIP();
        ACCEPT(T_EOL, "expecting newline");
        return ast_create_node(p->context, AST_PASS);
    }
    
    PARSE_ERROR("expecting statement");
}

ast_id_t parse_while(parser_t *p) {
    NODE_TYPE(AST_WHILE);
    
    ACCEPT(T_WHILE, "expecting `while`");
    SKIP();
    PARSE_CHILD(expression);
    BLOCK_PRELUDE();
    
    PARSE_CHILD(statement_block);
    
    return node;
}

ast_id_t parse_if(parser_t *p) {
    NODE_TYPE(AST_IF);
    
    ACCEPT(T_IF, "expecting `if`");
    SKIP();
    PARSE_CHILD(expression);
    BLOCK_PRELUDE();
    PARSE_CHILD(statement_block);
    
    while (CURR() == T_ELSEIF) {
        NEXT();
        SKIP();
        PARSE_CHILD(expression);
        BLOCK_PRELUDE();
        PARSE_CHILD(statement_block);
    }
    
    if (CURR() == T_ELSE) {
        NEXT();
        BLOCK_PRELUDE();
        PARSE_CHILD(statement_block);
    }
    
    return node;
}

ast_id_t parse_function_definition(parser_t *p) {
    
    // NODE_TYPE(...);
    // 
    // ACCEPT(T_DEF);
    // 
    // if (CURR() == T_IDENT) {
    //     ast_node_append_symbol(p->context, node, p->token_text, p->token_len);
    //     NEXT();
    // } else {
    //     ACCEPT(T_IDENT, "expecting identifier");
    // }
    // 
    // SKIP();
    // ACCEPT(T_L_PAREN);
    // PARSE_CHILD(argument_list);
    // ACCEPT(T_R_PAREN);
    // BLOCK_PRELUDE();
    // 
    // PARSE_CHILD(statement_block);
    // 
    // return node;
    return 0;
}

ast_id_t parse_expression(parser_t *p) {
    return parse_value(p);
}

ast_id_t parse_value(parser_t *p) {
    if (CURR() == T_INTEGER) {
        INT val;
        if (decode_int(p, p->token_text, &val)) {
            NODE_TYPE(AST_LITERAL_INT);
            ast_id_t val_node = ast_get_free_value(p->context);
            p->context->ast_pool.values[AST_INDEX(val_node)].val_int = val;
            APPEND(val_node);
            NEXT();
            return node;
        } else {
            return 0;
        }
    } else if (CURR() == T_STRING) {
        NODE_TYPE(AST_LITERAL_STRING);
        NEXT();
        return node;
    } else if (CURR() == T_IDENT) {
        NODE_TYPE(AST_LITERAL_IDENT);
        NEXT();
        return node;
    } else if (CURR() == T_SYMBOL) {
        NODE_TYPE(AST_LITERAL_SYMBOL);
        NEXT();
        return node;
    } else if (CURR() == T_TRUE) {
        NODE_TYPE(AST_LITERAL_BOOL);
        ast_id_t val = ast_get_free_value(p->context);
        p->context->ast_pool.values[AST_INDEX(val)].val_int = 1;
        APPEND(val);
        NEXT();
        return node;
    } else if (CURR() == T_FALSE) {
        NODE_TYPE(AST_LITERAL_BOOL);
        ast_id_t val = ast_get_free_value(p->context);
        p->context->ast_pool.values[AST_INDEX(val)].val_int = 0;
        APPEND(val);
        NEXT();
        return node;
    } else {
        return 0;
    }
}

ast_id_t parse_anonymous_function(parser_t *p) {

    // NODE_TYPE(...);
    // 
    // ACCEPT(T_DEF);
    // SKIP();
    // ACCEPT(T_L_PAREN);
    // PARSE_CHILD(argument_list);
    // ACCEPT(T_R_PAREN);
    // BLOCK_PRELUDE();
    // 
    // PARSE_CHILD(statement_block);
    // 
    // return node;
    return 0;
}

ast_id_t parse_parameter_list(parser_t *p) {
    NODE_TYPE(AST_PARAMETER_LIST);
    SKIP();
    // TODO: parse param list
    return node;
}

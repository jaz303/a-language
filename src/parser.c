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
    ast_id_t curr = node; 

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
    
#define PARSE_CHILD(type) \
    { \
        ast_id_t type##_node = parse_##type(p); \
        if (type##_node == 0) { \
            ast_destroy(p->context, node); \
            return 0; \
        } \
        curr = ast_append_node(p->context, curr, type##_node); \
    }
    
#define BLOCK_PRELUDE() \
    SKIP(); \
    ACCEPT(T_COLON, "expecting `:`"); \
    SKIP(); \
    ACCEPT(T_EOL, "expecting newline");

static ast_id_t parse_statement_block(parser_t *p);
static ast_id_t parse_statement(parser_t *p);
static ast_id_t parse_while(parser_t *p);
static ast_id_t parse_if(parser_t *p);
static ast_id_t parse_function_definition(parser_t *p);
static ast_id_t parse_expression(parser_t *p);
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
    
    while (CURR() != T_OUTDENT) {
        PARSE_CHILD(statement);
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
    return 0;
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

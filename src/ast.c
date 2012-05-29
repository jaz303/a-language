#include "menace/ast.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define AST_PAGE_KB     8
#define INT_POOL_MIN    -128
#define INT_POOL_MAX    127

/* some AST leaf nodes can be shared */
ast_pass_t                  shared_pass;
ast_literal_collection_t    shared_empty_array;
ast_literal_collection_t    shared_empty_dict;
ast_literal_t               shared_null;
ast_literal_t               shared_true;
ast_literal_t               shared_false;
ast_literal_t               shared_int[INT_POOL_MAX - INT_POOL_MIN + 1];

void ast_global_init() {
    
    shared_pass.type = AST_PASS;
    
    shared_empty_array.type = AST_ARRAY;
    shared_empty_array.head = NULL;
    
    shared_empty_dict.type = AST_DICT;
    shared_empty_dict.head = NULL;
    
    shared_null.type = AST_LITERAL;
    shared_null.value = kNull;
    
    shared_true.type = AST_LITERAL;
    shared_true.value = kTrue;
    
    shared_false.type = AST_LITERAL;
    shared_false.value = kFalse;
    
    for (int i = INT_POOL_MIN; i <= INT_POOL_MAX; i++) {
        shared_int[i - INT_POOL_MIN].type = AST_LITERAL;
        shared_int[i - INT_POOL_MIN].value = MK_INTVAL(i);
    }
    
}

void ast_init(context_t *ctx) {
    // TODO: mojo
}

void* ast_alloc(context_t *ctx, size_t sz) {
    // TODO: ast node to be allocated from context pool
    void *ast_node = malloc(sz);
    if (!ast_node) memory_error();
    return ast_node;
}

void* ast_alloc_with_type(context_t *ctx, size_t sz, ast_node_type_t type) {
    ast_node_t *node = ast_alloc(ctx, sz);
    node->type = type;
    return node;
}

void ast_cleanup(context_t *ctx) {
    // TODO: clean up all allocated nodes
}

/* Constructors */

#define ALLOC(c_type) \
    c_type *node = ast_alloc(ctx, sizeof(c_type)); \
    if (node == NULL) { \
        return NULL; \
    } \

#define NODE(c_type, ast_type) \
    c_type *node = ast_alloc_with_type(ctx, sizeof(c_type), ast_type); \
    if (node == NULL) { \
        return NULL; \
    } \

ast_statements_t* ast_mk_statements(context_t *ctx, ast_node_t *stmt) {
    ALLOC(ast_statements_t);
    node->statement = stmt;
    node->next = NULL;
    return node;
}

void ast_cons_statements(ast_statements_t *stmts, ast_statements_t *cons) {
    stmts->next = cons;
}

ast_node_t* ast_mk_if(context_t *ctx, ast_conditions_t *conditions) {
    NODE(ast_if_t, AST_IF);
    node->conditions = conditions;
    return (ast_node_t*) node;
}

ast_conditions_t* ast_mk_condition(context_t *ctx, ast_node_t *exp, ast_statements_t *body) {
    ALLOC(ast_conditions_t);
    node->exp = exp;
    node->body = body;
    node->next = NULL;
    return node;
}

void ast_cons_cond(ast_conditions_t *conditions, ast_conditions_t *cons) {
    conditions->next = cons;
}

ast_node_t* ast_mk_while(context_t *ctx, ast_node_t *exp, ast_statements_t *body) {
    NODE(ast_while_t, AST_WHILE);
    node->condition = exp;
    node->body = body;
    return (ast_node_t*) node;
}

ast_node_t* ast_mk_pass(context_t *ctx) {
    return (ast_node_t*) &shared_pass;
}

ast_node_t* ast_mk_assign(context_t *ctx, ast_node_t *target, ast_node_t *value) {
    NODE(ast_assign_t, AST_ASSIGN);
    node->target = target;
    node->value = value;
    return (ast_node_t*) node;
}

ast_node_t* ast_mk_return(context_t *ctx, ast_node_t *retval) {
    NODE(ast_return_t, AST_RETURN);
    node->exp = retval;
    return (ast_node_t*) node;
}

ast_node_t* ast_mk_named_function(context_t *ctx, INTERN name, ast_parameters_t *params, ast_statements_t *body) {
    NODE(ast_function_t, AST_FUNCTION);
    node->name = name;
    node->parameters = params;
    node->body = body;
    return (ast_node_t*) node;
}

ast_parameters_t* ast_mk_parameters(context_t *ctx, INTERN id) {
    ALLOC(ast_parameters_t);
    node->name = id;
    node->next = NULL;
    return node;
}

void ast_cons_parameters(ast_parameters_t *params, ast_parameters_t *cons) {
    params->next = cons;
}

ast_node_t* ast_mk_unary_exp(context_t *ctx, int operator, ast_node_t *exp) {
    NODE(ast_unary_exp_t, AST_UNARY_EXP);
    node->operator = operator;
    node->exp = exp;
    return (ast_node_t*) node;
}

ast_node_t* ast_mk_binary_exp(context_t *ctx, int operator, ast_node_t *lexp, ast_node_t *rexp) {
    NODE(ast_binary_exp_t, AST_BINARY_EXP);
    node->operator = operator;
    node->lexp = lexp;
    node->rexp = rexp;
    return (ast_node_t*) node;
}

ast_node_t* ast_mk_integer(context_t *ctx, INT val) {
    if (val >= INT_POOL_MIN && val <= INT_POOL_MAX) {
        return (ast_node_t*) &shared_int[val - INT_POOL_MIN];
    } else {
        NODE(ast_literal_t, AST_LITERAL);
        node->value = MK_INTVAL(val);
        return (ast_node_t*) node;
    }
}

ast_node_t* ast_mk_string(context_t *ctx, const char *str) {
    return NULL;
}

ast_node_t* ast_mk_symbol(context_t *ctx, INTERN name) {
    NODE(ast_literal_t, AST_LITERAL);
    node->value = MK_SYMBOL(name);
    return (ast_node_t *)node;
}

ast_node_t* ast_mk_true(context_t *ctx) {
    return (ast_node_t*) &shared_true;
}

ast_node_t* ast_mk_false(context_t *ctx) {
    return (ast_node_t*) &shared_false;
}

ast_node_t* ast_mk_ident(context_t *ctx, INTERN name) {
    NODE(ast_ident_t, AST_IDENT);
    node->name = name;
    return (ast_node_t *)node;
}

ast_node_t* ast_mk_empty_array(context_t *ctx) {
    return (ast_node_t*) &shared_empty_array;
}

ast_node_t* ast_mk_array(context_t *ctx, ast_array_members_t *members) {
    NODE(ast_literal_collection_t, AST_ARRAY);
    node->head = members;
    return (ast_node_t*) node;
}

ast_array_members_t* ast_mk_array_members(context_t *ctx, ast_node_t *value) {
    ALLOC(ast_array_members_t);
    node->value = value;
    node->next = NULL;
    return node;
}

void ast_cons_array_members(ast_array_members_t *mem, ast_array_members_t *cons) {
    mem->next = cons;
}

ast_node_t* ast_mk_empty_dict(context_t *ctx) {
    return (ast_node_t*) &shared_empty_dict;
}

ast_node_t* ast_mk_dict(context_t *ctx, ast_dict_members_t *members) {
    NODE(ast_literal_collection_t, AST_DICT);
    node->head = members;
    return (ast_node_t*) node;
}

ast_dict_members_t* ast_mk_dict_members(context_t *ctx, ast_node_t *key, ast_node_t *value) {
    ALLOC(ast_dict_members_t);
    node->key = key;
    node->value = value;
    node->next = NULL;
    return node;
}

void ast_cons_dict_members(ast_dict_members_t *mem, ast_dict_members_t *cons) {
    mem->next = cons;
}

ast_expressions_t* ast_mk_expressions(context_t *ctx, ast_node_t *exp) {
    ALLOC(ast_expressions_t);
    node->exp = exp;
    node->next = NULL;
    return node;
}

void ast_cons_expressions(ast_expressions_t* exp, ast_expressions_t* cons) {
    exp->next = cons;
}

ast_node_t* ast_mk_selector(context_t *ctx, ast_node_t *receiver, INTERN name) {
    NODE(ast_selector_t, AST_SELECTOR);
    node->receiver = receiver;
    node->name = name;
    return (ast_node_t*) node;
}

ast_node_t* ast_mk_invoke(context_t *ctx, ast_node_t *receiver, INTERN name, ast_expressions_t *args) {
    NODE(ast_invoke_t, AST_INVOKE);
    node->receiver = receiver;
    node->name = name;
    node->arguments = args;
    return (ast_node_t*) node;
}

ast_node_t* ast_mk_index(context_t *ctx, ast_node_t *receiver, ast_expressions_t *args) {
    NODE(ast_index_t, AST_INDEX);
    node->receiver = receiver;
    node->arguments = args;
    return (ast_node_t*) node;
}

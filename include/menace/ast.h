#ifndef MENACE_AST_H
#define MENACE_AST_H

#include "menace/global.h"

void                    ast_global_init();
void                    ast_init(context_t *ctx);
void*                   ast_alloc(context_t *ctx, size_t sz);
void*                   ast_alloc_with_type(context_t *ctx, size_t sz, ast_node_type_t type);
void                    ast_cleanup(context_t *ctx);
                        
ast_statements_t*       ast_mk_statements(context_t *ctx, ast_node_t *stmt);
void                    ast_cons_statements(ast_statements_t *stmts, ast_statements_t *cons);
ast_node_t*             ast_mk_if(context_t *ctx, ast_conditions_t *conditions);
ast_conditions_t*       ast_mk_condition(context_t *ctx, ast_node_t *exp, ast_statements_t *body);
void                    ast_cons_cond(ast_conditions_t *conditions, ast_conditions_t *cons);
ast_node_t*             ast_mk_while(context_t *ctx, ast_node_t *exp, ast_statements_t *body);
ast_node_t*             ast_mk_for(context_t *ctx, INTERN key_var, INTERN value_var, ast_node_t *exp, ast_statements_t *body);
ast_node_t*             ast_mk_pass(context_t *ctx);
ast_node_t*             ast_mk_assign(context_t *ctx, ast_node_t *target, ast_node_t *value);
ast_node_t*             ast_mk_return(context_t *ctx, ast_node_t *retval);
ast_node_t*             ast_mk_named_function(context_t *ctx, INTERN name, ast_parameters_t *params, ast_statements_t *body);
ast_parameters_t*       ast_mk_parameters(context_t *ctx, INTERN id);
void                    ast_cons_parameters(ast_parameters_t *params, ast_parameters_t *cons);
ast_node_t*             ast_mk_unary_exp(context_t *ctx, int operator, ast_node_t *exp);
ast_node_t*             ast_mk_binary_exp(context_t *ctx, int operator, ast_node_t *lexp, ast_node_t *rexp);
ast_node_t*             ast_mk_integer(context_t *ctx, INT val);
ast_node_t*             ast_mk_string(context_t *ctx, const char *str);
ast_node_t*             ast_mk_symbol(context_t *ctx, INTERN name);
ast_node_t*             ast_mk_true(context_t *ctx);
ast_node_t*             ast_mk_false(context_t *ctx);
ast_node_t*             ast_mk_ident(context_t *ctx, INTERN name);
ast_node_t*             ast_mk_empty_array(context_t *ctx);
ast_node_t*             ast_mk_array(context_t *ctx, ast_array_members_t *members);
ast_array_members_t*    ast_mk_array_members(context_t *ctx, ast_node_t *value);
void                    ast_cons_array_members(ast_array_members_t *mem, ast_array_members_t *cons);
ast_node_t*             ast_mk_empty_dict(context_t *ctx);
ast_node_t*             ast_mk_dict(context_t *ctx, ast_dict_members_t *members);
ast_dict_members_t*     ast_mk_dict_members(context_t *ctx, ast_node_t *key, ast_node_t *value);
void                    ast_cons_dict_members(ast_dict_members_t *mem, ast_dict_members_t *cons);
ast_expressions_t*      ast_mk_expressions(context_t *ctx, ast_node_t *exp);
void                    ast_cons_expressions(ast_expressions_t* exp, ast_expressions_t* cons);
ast_node_t*             ast_mk_selector(context_t *ctx, ast_node_t *receiver, INTERN name);
ast_node_t*             ast_mk_invoke(context_t *ctx, ast_node_t *receiver, INTERN name, ast_expressions_t *args);
ast_node_t*             ast_mk_index(context_t *ctx, ast_node_t *receiver, ast_expressions_t *args);

#endif

#include "menace/global.h"
#include "menace/intern.h"

static void do_pretty_print_statements(context_t *ctx, ast_statements_t *stmts, FILE *stream, int indent);
static void do_pretty_print_exp(context_t *ctx, ast_node_t *node, FILE *stream);
static void do_pretty_print_exp_list(context_t *ctx, ast_expressions_t *exps, FILE *stream);

/* Public API */

void pretty_print(context_t *ctx, ast_statements_t *stmts, FILE *stream) {
    do_pretty_print_statements(ctx, stmts, stream, 0);
    fputc('\n', stream);
}

/* Private */

#define INDENT() for (int i = 0; i < indent; i++) { fputs("    ", stream); }

static void do_pretty_print_statements(context_t *ctx, ast_statements_t *stmts, FILE *stream, int indent) {
    while (stmts) {
        INDENT();
        
        ast_node_t *node = stmts->statement;
        switch (node->type) {
            case AST_WHILE:
            {
                ast_while_t *w = (ast_while_t*)node;
                
                fprintf(stream, "while ");
                do_pretty_print_exp(ctx, w->condition, stream);
                fputs(":\n", stream);
                do_pretty_print_statements(ctx, w->body, stream, indent + 1);
                
                break;
            }
            case AST_FOR:
            {
                ast_for_t *f = (ast_for_t*)node;
                
                fprintf(stream, "for ");
                if (f->key_var) {
                    fprintf(stream, "%s, ", intern_to_string(ctx, f->key_var));
                }
                fprintf(stream, "%s in ", intern_to_string(ctx, f->value_var));
                do_pretty_print_exp(ctx, f->exp, stream);
                fputs(":\n", stream);
                do_pretty_print_statements(ctx, f->body, stream, indent + 1);
                
                break;
            }
            case AST_ASSIGN:
            {
                ast_assign_t *a = (ast_assign_t*)node;
                
                do_pretty_print_exp(ctx, a->target, stream);
                fputs(" = ", stream);
                do_pretty_print_exp(ctx, a->value, stream);
                fputc('\n', stream);
                
                break;
            }
            case AST_IF:
            {
                ast_if_t *i = (ast_if_t *)node;
                ast_conditions_t *curr = i->conditions;

                int ix = 0;
                while (curr) {
                    if (ix == 0) {
                        fputs("if ", stream);
                    } else if (curr->exp) {
                        INDENT();
                        fputs("elseif ", stream);
                    } else {
                        INDENT();
                        fputs("else", stream);
                    }
                    if (curr->exp) {
                        do_pretty_print_exp(ctx, curr->exp, stream);
                    }
                    fputs(":\n", stream);
                    do_pretty_print_statements(ctx, curr->body, stream, indent + 1);
                    ix++;
                    curr = curr->next;
                }

                break;
            }
            case AST_PASS:
            {
                fputs("pass\n", stream);
                
                break;
            }
            case AST_RETURN:
            {
                ast_return_t *r = (ast_return_t*)node;
                
                if (r->exp == NULL) {
                    fputs("return\n", stream);
                } else {
                    fputs("return ", stream);
                    do_pretty_print_exp(ctx, r->exp, stream);
                    fputc('\n', stream);
                }
                
                break;
            }
            case AST_FUNCTION:
            {
                ast_function_t *f = (ast_function_t*)node;
                
                fprintf(stream, "def %s", intern_to_string(ctx, f->name));
                
                ast_parameters_t *curr = f->parameters;
                if (curr) {
                    fputc('(', stream);
                    int ix = 0;
                    while (curr) {
                        if (ix) {
                            fputs(", ", stream);
                        }
                        fputs(intern_to_string(ctx, curr->name), stream);
                        curr = curr->next;
                        ix++;
                    }
                    fputc(')', stream);
                }
                
                fputs(":\n", stream);
                
                do_pretty_print_statements(ctx, f->body, stream, indent + 1);
                
                break;
            }
            default: /* handle expression statements */
            {
                do_pretty_print_exp(ctx, stmts->statement, stream);
                fputs("\n", stream);
                
                break;
            }
        }
        
        stmts = stmts->next;
    }
}

static void do_pretty_print_exp(context_t *ctx, ast_node_t *node, FILE *stream) {
    switch (node->type) {
        case AST_IDENT:
        {
            ast_ident_t *i = (ast_ident_t*)node;
            fprintf(stream, "%s", intern_to_string(ctx, i->name));
            break;
        }
        case AST_LITERAL:
        {
            VALUE v = ((ast_literal_t*)node)->value;
            if (VALUE_IS_INT(v)) {
                fprintf(stream, "%lld", (long long int) INTVAL(v));
            } else if (VALUE_IS_SYMBOL(v)) {
                fprintf(stream, ":%s", intern_to_string(ctx, SYMBOLVAL(v)));
            } else if (v == kNull) {
                fputs("null", stream);
            } else if (v == kTrue) {
                fputs("true", stream);
            } else if (v == kFalse) {
                fputs("false", stream);
            } else {
                fputs("<unknown-literal>", stream);
            }
            
            break;
        }
        case AST_STRING:
        {
            ast_string_t *s = (ast_string_t*)node;
            
            fputc('"', stream);
            
            const char *str = intern_to_string(ctx, s->string);
            while (*str) {
                switch (*str) {
                    case '\r':      fputs("\\r", stream); break;
                    case '\n':      fputs("\\n", stream); break;
                    case '\f':      fputs("\\f", stream); break;
                    case '\t':      fputs("\\t", stream); break;
                    case '\b':      fputs("\\b", stream); break;
                    case '\'':      fputs("\\'", stream); break;
                    case '"':       fputs("\\\"", stream); break;
                    case '\\':      fputs("\\\\", stream); break;
                    default:        fputc(*str, stream); break;
                }
                str++;
            }
            
            fputc('"', stream);
            
            break;
        }
        case AST_ARRAY:
        {
            ast_literal_collection_t *a = (ast_literal_collection_t*)node;
            ast_array_members_t *curr = (ast_array_members_t*)a->head;
            
            fputc('[', stream);
            int ix = 0;
            while (curr) {
                if (ix++) fputs(", ", stream);
                do_pretty_print_exp(ctx, curr->value, stream);
                curr = curr->next;
            }
            fputc(']', stream);
            
            break;
        }
        case AST_DICT:
        {
            ast_literal_collection_t *d = (ast_literal_collection_t*)node;
            ast_dict_members_t *curr = (ast_dict_members_t*)d->head;
            
            fputc('{', stream);
            int ix = 0;
            while (curr) {
                if (ix++) fputs(", ", stream);
                do_pretty_print_exp(ctx, curr->key, stream);
                fputs(" => ", stream);
                do_pretty_print_exp(ctx, curr->value, stream);
                curr = curr->next;
            }
            fputc('}', stream);
            
            break;
        }
        case AST_UNARY_EXP:
        {
            ast_unary_exp_t *u = (ast_unary_exp_t*)node;
            
            fputs(token_get_name(u->operator), stream);
            do_pretty_print_exp(ctx, u->exp, stream);
            
            break;
        }
        case AST_BINARY_EXP:
        {
            ast_binary_exp_t *b = (ast_binary_exp_t*)node;
            
            fputc('(', stream);
            do_pretty_print_exp(ctx, b->lexp, stream);
            fputs(token_get_name(b->operator), stream);
            do_pretty_print_exp(ctx, b->rexp, stream);
            fputc(')', stream);
            
            break;
        }
        case AST_SELECTOR:
        {
            ast_selector_t *s = (ast_selector_t*)node;
            
            do_pretty_print_exp(ctx, s->receiver, stream);
            fputc('.', stream);
            fputs(intern_to_string(ctx, s->name),stream);
            
            break;
        }
        case AST_INVOKE:
        {
            ast_invoke_t *i = (ast_invoke_t*)node;
            
            if (i->receiver) {
                do_pretty_print_exp(ctx, i->receiver, stream);
                fputc('.', stream);
            }
            
            fputs(intern_to_string(ctx, i->name),stream);
            fputc('(', stream);
            do_pretty_print_exp_list(ctx, i->arguments, stream);
            fputc(')', stream);
            
            break;
        }
        case AST_INDEX:
        {
            ast_index_t *i = (ast_index_t*)node;
            
            do_pretty_print_exp(ctx, i->receiver, stream);
            fputc('[', stream);
            do_pretty_print_exp_list(ctx, i->arguments, stream);
            fputc(']', stream);
            
            break;
        }
        
        /* statement nodes; these should never make it into this function */
        case AST_FOR:
        case AST_WHILE:
        case AST_ASSIGN:
        case AST_IF:
        case AST_PASS:
        case AST_RETURN:
        case AST_FUNCTION:
        {
            fputs("<error>", stream);
        }
    }
}

static void do_pretty_print_exp_list(context_t *ctx, ast_expressions_t *exps, FILE *stream) {
    int ix = 0;
    while (exps) {
        if (ix++) fputs(", ", stream);
        do_pretty_print_exp(ctx, exps->exp, stream);
        exps = exps->next;
    }
}

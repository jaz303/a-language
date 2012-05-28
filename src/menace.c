#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <sysexits.h>

#include "menace/global.h"
#include "menace/scanner.h"
#include "menace/parser.h"
#include "menace/debug.h"

typedef struct active_file {
    FILE            *file;
    context_t       context;
    scanner_t       *scanner;
    parser_t        parser;
} active_file_t;

/*
 * Global State
 */

enum {
    RUN,
    HELP,
    SYNTAX_CHECK,
    PRETTY_PRINT
};

int mode    = RUN;
int verbose = 0;

static active_file_t* open_file(const char *filename) {
    
    active_file_t *out = malloc(sizeof(active_file_t));
    if (out == NULL) {
        fprintf(stderr, "couldn't allocate storage for active file\n");
        exit(EX_UNAVAILABLE);
    }
    
    out->file = fopen(filename, "r");
    if (out->file == NULL) {
        fprintf(stderr, "couldn't read file %s\n", filename);
        exit(EX_NOINPUT);
    }
    
    out->scanner = scanner_create_for_file(out->file);
    if (out->scanner == NULL) {
        fprintf(stderr, "couldn't create scanner\n");
        exit(EX_UNAVAILABLE);
    }
    
    if (!context_init(&out->context)) {
        fprintf(stderr, "couldn't initialise context\n");
        exit(EX_UNAVAILABLE);
    }
    
    if (!parser_init(&out->parser, &out->context, out->scanner)) {
        fprintf(stderr, "couldn't initialise parser\n");
        exit(EX_UNAVAILABLE);
    }
    
    return out;
    
}

void display_usage(int extended) {
    printf( "Usage: menace [switches] [--] [programfile] [arguments]\n");
    
    if (extended) {
        printf( "  -h, --help       display this message\n"
                "  -c               syntax check only\n"
                "  --pretty-print   parse programfile and pretty print to stdout\n"
                "  -v, --verbose    be verbose\n"
            );
    }
}

void require_arg(int ix, int argc) {
    if (ix >= argc) {
        display_usage(0);
        exit(EX_USAGE);
    }
}

void do_run(int ix, int argc, char *argv[]) {
    require_arg(ix, argc);
    active_file_t *input = open_file(argv[ix]);
    
    fclose(input->file);
}

void do_pretty_print(int ix, int argc, char *argv[]) {
    require_arg(ix, argc);
    active_file_t *input = open_file(argv[ix]);
    
    ast_statements_t *stmts = parser_parse(&input->parser);
    if (stmts == NULL) {
        printf("parse error\n");
        exit(EX_DATAERR);
    }
    
    fclose(input->file);
    
    pretty_print(&input->context, stmts, stdout);
}

void do_syntax_check(int ix, int argc, char *argv[]) {
    require_arg(ix, argc);
    active_file_t *input = open_file(argv[ix]);
    
    parser_parse(&input->parser);
    fclose(input->file);
    
    if (input->parser.error) {
        exit(EX_DATAERR);
    } else {
        printf("%s: no syntax errors detected\n", argv[ix]);
    }
}

int main(int argc, char *argv[]) {
    
    while (1) {
        
        int option_index;
        static struct option options[] = {
            { "help",           0, &mode,       HELP },
            { "pretty-print",   0, &mode,       PRETTY_PRINT },
            { "verbose",        0, &verbose,    1 },
            { 0, 0, 0, 0 }
        };
        
        int c = getopt_long(argc, argv, "chv", options, &option_index);
        if (c == -1) {
            break;
        }
        
        switch (c) {
            case 'c':
                mode = SYNTAX_CHECK;
                break;
            case 'h':
                mode = HELP;
                break;
            case 'v':
                verbose = 1;
                break;
        }
        
    }
    
    switch (mode) {
        case HELP:              display_usage(1);                       break;
        case RUN:               do_run(optind, argc, argv);             break;
        case PRETTY_PRINT:      do_pretty_print(optind, argc, argv);    break;
        case SYNTAX_CHECK:      do_syntax_check(optind, argc, argv);    break;
    }
    
    exit(0);
    
}

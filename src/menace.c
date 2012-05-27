#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <sysexits.h>

#include "menace/global.h"
#include "menace/scanner.h"
#include "menace/parser.h"
#include "menace/debug.h"

void display_usage(int extended) {
    printf( "Usage: menace [switches] [--] [programfile] [arguments]\n");
    
    if (extended) {
        printf( "  -h, --help       display this message\n"
                "  --pretty-print   parse programfile and pretty print to stdout\n"
            );
    }
}

void require_arg(int ix, int argc) {
    if (ix >= argc) {
        display_usage(0);
        exit(EX_USAGE);
    }
}

void init_context(context_t *ctx) {
    if (!context_init(ctx)) {
        fatal_error("coldn't initialise context");
    }
}

void do_run(int ix, int argc, char *argv[]) {
    require_arg(ix, argc);
    
    context_t ctx;
    init_context(&ctx);
    
}

void do_pretty_print(int ix, int argc, char *argv[]) {
    require_arg(ix, argc);
    
    FILE *file = fopen(argv[ix], "r");
    if (file == NULL) {
        printf("couldn't read file %s\n", argv[ix]);
        exit(EX_NOINPUT);
    }
    
    context_t ctx;
    init_context(&ctx);
    
    scanner_t *scanner = scanner_create_for_file(file);
    
    parser_t parser;
    parser_init(&parser, &ctx, scanner);
    
    ast_statements_t *stmts = parser_parse(&parser);
    if (stmts == NULL) {
        printf("parse error\n");
        exit(EX_DATAERR);
    }
    
    fclose(file);
    
    pretty_print(&ctx, stmts, stdout);
}

enum {
    RUN             = 0,
    HELP            = 1,
    PRETTY_PRINT    = 2
};

int mode = RUN;

int main(int argc, char *argv[]) {
    
    while (1) {
        
        int option_index;
        static struct option options[] = {
            { "help",           0, &mode, HELP },
            { "pretty-print",   0, &mode, PRETTY_PRINT },
            { 0, 0, 0, 0 }
        };
        
        int c = getopt_long(argc, argv, "h", options, &option_index);
        if (c == -1) {
            break;
        }
        
        switch (c) {
            case 'h':
                mode = HELP;
                break;
        }
        
    }
    
    switch (mode) {
        case HELP:              display_usage(1);                       break;
        case RUN:               do_run(optind, argc, argv);             break;
        case PRETTY_PRINT:      do_pretty_print(optind, argc, argv);    break;
    }
    
    return 0;
    
}

#include <stdio.h> 

#include "menace/global.h"
#include "menace/scanner.h"

int main(int argc, char *argv[]) {
    
    scanner_t *scanner = scanner_create_for_file(stdin);
    token_t token;
    
    do {
        token = scanner_get_next_token(scanner, NULL, NULL);
        printf("%s\n", token_get_name(token));
    } while (token != T_EOF);
    
    return 0;

}
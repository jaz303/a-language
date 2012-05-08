#include "menace/global.h"

#define TOKEN(symbol, string) string
const char const *token_names[] = {
    "INVALID",
    #include "menace/tokens.x"
};
#undef TOKEN
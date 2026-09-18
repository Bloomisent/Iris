#include <stdlib.h>
#include <string.h>

#include "include/token.h"

Token_T* Init_Token(int type,const char* value) {
    Token_T* t = calloc(1, sizeof *t);
    if (!t) exit(EXIT_FAILURE);

    t->type = (TokenType)type;

    size_t n = value ? strlen(value) : 0;
    t->value = malloc(n + 1);

    if (!t->value) exit(EXIT_FAILURE);

    if (value) {
        memcpy(t->value, value, n + 1);
    } else {
        t->value[0] = '\0';
    };

    return t;
}

void Token_Free(Token_T* t) {
    if (t) {
        free(t->value);
        free(t);
    }
}

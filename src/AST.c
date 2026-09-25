#include <stdlib.h>
#include <string.h>

#include "include/AST.h"

char* Iris_Strdup( const char* s) {
    if (!s) return NULL;
    size_t n = strlen(s);
    char* p = malloc(n + 1);
    if (!p) exit(EXIT_FAILURE);
    memcpy(p, s, n + 1);
    return p;
}

AST_T* Init_AST(int type) {
    AST_T* a = calloc(1, sizeof *a);
    if (!a) exit(EXIT_FAILURE);
    a->type = (AST_Type)type;
    return a;
}

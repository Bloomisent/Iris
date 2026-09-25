#include <stdlib.h>
#include <string.h>

#include "include/scope.h"

static AST_T* add(AST_T*** a, size_t* n, AST_T* v) {
    AST_T** p = realloc(*a, (*n + 1) * sizeof *p);
    if (!p) {
        exit(EXIT_FAILURE);
    };
    *a = p;
    (*a)[(*n)++] = v;
    return v;
}

Scope_T* Init_Scope(void) {

    Scope_T* s = calloc(1, sizeof *s);

    if (!s) {
        exit(EXIT_FAILURE);
    };
    return s;
}

Scope_T* Init_Scope_Child(Scope_T* p) {
    Scope_T* s = Init_Scope();
    s->parent = p;
    return s;
}

AST_T* Scope_Add_Function_Definition(Scope_T* s, AST_T* v) {
    return add(&s->function_definitions, &s->function_definitions_size,v);
}

AST_T* Scope_Add_Table_Definition(Scope_T* s, AST_T* v) {
    return add(&s->table_definitions, &s->table_definitions_size, v);
}

AST_T* Scope_Add_Dictionary_Definition(Scope_T* s, AST_T* v) {
    return add(&s->dictionary_definitions, &s->dictionary_definitions_size, v);
}

AST_T* Scope_Add_Class_Definition(Scope_T* s, AST_T* v) {
    return add(&s->class_definitions, &s->class_definitions_size, v);
}

AST_T* Scope_Add_Variable_Definition(Scope_T* s, AST_T* v) {
    return add(&s->variable_definitions, &s->variable_definitions_size, v);
}

AST_T* Scope_Get_Function_Definition(Scope_T* s, const char* n) {

    for (;s;s=s->parent) {
        for (size_t i=s->function_definitions_size;i--;) {
            if (strcmp(s->function_definitions[i]->function_definition_name, n)== 0) {
                return s->function_definitions[i];
            }
        }
    }

    return NULL;
}

AST_T* Scope_Get_Table_Definition(Scope_T* s, const char* n) {
    for (;s;s=s->parent) {
        for (size_t i =s->table_definitions_size;i--;) {
            if (strcmp(s->table_definitions[i]->table_definition_name,n) == 0) {
                return s->table_definitions[i];
            };
        }
    }

    return NULL;
}

AST_T* Scope_Get_Dictionary_Definition(Scope_T* s, const char* n) {
    for (;s;s=s->parent) {
        for (size_t i =s->dictionary_definitions_size;i--;) {
            if (strcmp(s->dictionary_definitions[i]->dictionary_definition_name, n) == 0) {
                return s->dictionary_definitions[i];
            }
        }
    }

    return NULL;
}

AST_T* Scope_Get_Class_Definition(Scope_T* s, const char* n) {
    for (; s;s=s->parent) {
        for (size_t i=s->class_definitions_size;i--;) {
            if (strcmp(s->class_definitions[i]->class_definition_name, n) == 0) {
                return s->class_definitions[i];
            }
        }
    }

    return NULL;
}

AST_T* Scope_Get_Variable_Definition(Scope_T* s, const char* n) {
    for (;s;s=s->parent) {
        for (size_t i=s->variable_definitions_size; i--;) {
            if (strcmp(s->variable_definitions[i]->variable_definition_variable_name, n) == 0) {
                return s->variable_definitions[i];
            }
        }
    }

    return NULL;
}

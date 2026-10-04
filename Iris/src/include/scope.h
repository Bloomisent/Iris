#pragma once

#include <stddef.h>
#include "AST.h"

typedef struct SCOPE_STRUCT {

    struct SCOPE_STRUCT* parent;

    AST_T** variable_definitions;
    size_t variable_definitions_size;

    AST_T** function_definitions;
    size_t function_definitions_size;

    AST_T** table_definitions;
    size_t table_definitions_size;

    AST_T** dictionary_definitions;
    size_t dictionary_definitions_size;

    AST_T** class_definitions;
    size_t class_definitions_size;

} Scope_T;

Scope_T* Init_Scope(void);

Scope_T* Init_Scope_Child(
    Scope_T* parent
);

AST_T* Scope_Add_Variable_Definition(
    Scope_T* scope,
    AST_T* variable_definition
);

AST_T* Scope_Add_Function_Definition(
    Scope_T* scope,
    AST_T* function_definition
);

AST_T* Scope_Add_Table_Definition(
    Scope_T* scope,
    AST_T* table_definition
);

AST_T* Scope_Add_Dictionary_Definition(
    Scope_T* scope,
    AST_T* dictionary_definition
);

AST_T* Scope_Add_Class_Definition(
    Scope_T* scope,
    AST_T* class_definition
);

AST_T* Scope_Get_Variable_Definition(
    Scope_T* scope,
    const char* name
);

AST_T* Scope_Get_Function_Definition(
    Scope_T* scope,
    const char* name
);

AST_T* Scope_Get_Table_Definition(
    Scope_T* scope,
    const char* name
);

AST_T* Scope_Get_Dictionary_Definition(
    Scope_T* scope,
    const char* name
);

AST_T* Scope_Get_Class_Definition(
    Scope_T* scope,
    const char* name
);

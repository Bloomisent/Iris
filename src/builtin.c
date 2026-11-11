#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "include/builtin.h"

AST_T* builtin_function_print(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size <= 0) {
        Iris_Throw(visitor, NULL, "function 'print'", "argument underflow. (0 to 1)");
    };

    for (int i=0;i<args_size;i++){
        AST_T* visited_ast = Visitor_Visit(visitor, args[i]);

        switch (visited_ast->type) {
            case AST_STRING: {
                if (strcmp(visited_ast->string_value, "%n") == 0) {
                    printf("\n");
                } else {
                    char *current = visited_ast->string_value;
                    char *next_match = strstr(current, "%n");
                    while (next_match != NULL) {
                        printf("%.*s", (int)(next_match - current), current);
                        printf("\n");
                        current = next_match + 2;
                        next_match = strstr(current, "%n");
                    };
                    printf("%s", current);
                };
                break;
            };
            case AST_BOOL: printf("%s", visited_ast->bool_value ? "true " : "false "); break;
            case AST_NULL: printf("null "); break;
            case AST_NUMBER : printf("%g ", visited_ast->number_value); break;
            case AST_INTEGER : printf("%d ", visited_ast->integer_value); break;
            case AST_CLASS : printf("%s ", visited_ast->class_name); break;
            case AST_CLASS_INSTANTIATION :
                printf("%s ", visited_ast->instance_class_name);
                break;
            case AST_TUPLE_DEFINITION : {
                printf("(");
                for (size_t j = 0; j < visited_ast->table_size; j++) {
                    AST_T* elem = Visitor_Visit(visitor, visited_ast->table_definition_value[j]);
                    switch (elem->type) {
                        case AST_STRING : printf(" %s", elem->string_value); break;
                        case AST_NUMBER : printf(" %g", elem->number_value); break;
                        case AST_INTEGER : printf("%i ", elem->integer_value); break;
                        case AST_BOOL: printf(" %s", elem->bool_value ? "true" : "false"); break;
                        case AST_NULL: printf(" null"); break;
                        case AST_CLASS : printf(" %s ", elem->class_name); break;
                        case AST_CLASS_INSTANTIATION :
                            printf(" %s", elem->instance_class_name);
                            break;
                        case AST_TUPLE_DEFINITION : printf(" "); AST_T** new_argse = calloc(1, sizeof(struct AST_STRUCT*)); new_argse[0] = elem; builtin_function_print(visitor, new_argse, 1); break;
                        case AST_TABLE_DEFINITION : printf(" "); AST_T** new_args = calloc(1, sizeof(struct AST_STRUCT*)); new_args[0] = elem; builtin_function_print(visitor, new_args, 1); break;
                        case AST_DICTIONARY_DEFINITION : printf(" "); AST_T** new_argss = calloc(1, sizeof(struct AST_STRUCT*)); new_argss[0] = elem; builtin_function_print(visitor, new_argss, 1); break;
                        default : printf(" %p", elem); break;
                    }
                    if (j < (visited_ast->tuple_size-1)){
                        printf(",");
                    };
                }
                printf(" )");
                break;
            };
            case AST_TABLE_DEFINITION : {
                printf("[");
                for (size_t j = 0; j < visited_ast->table_size; j++) {
                    AST_T* elem = Visitor_Visit(visitor, visited_ast->table_definition_value[j]);
                    switch (elem->type) {
                        case AST_STRING : printf(" %s", elem->string_value); break;
                        case AST_NUMBER : printf(" %g", elem->number_value); break;
                        case AST_INTEGER : printf("%i ", elem->integer_value); break;
                        case AST_BOOL: printf(" %s", elem->bool_value ? "true" : "false"); break;
                        case AST_NULL: printf(" null"); break;
                        case AST_CLASS : printf(" %s ", elem->class_name); break;
                        case AST_CLASS_INSTANTIATION :
                            printf(" %s", elem->instance_class_name);
                            break;
                        case AST_TUPLE_DEFINITION : printf(" "); AST_T** new_argse = calloc(1, sizeof(struct AST_STRUCT*)); new_argse[0] = elem; builtin_function_print(visitor, new_argse, 1); break;
                        case AST_TABLE_DEFINITION : printf(" "); AST_T** new_args = calloc(1, sizeof(struct AST_STRUCT*)); new_args[0] = elem; builtin_function_print(visitor, new_args, 1); break;
                        case AST_DICTIONARY_DEFINITION : printf(" "); AST_T** new_argss = calloc(1, sizeof(struct AST_STRUCT*)); new_argss[0] = elem; builtin_function_print(visitor, new_argss, 1); break;
                        default : printf(" %p", elem); break;
                    }
                    if (j < (visited_ast->table_size-1)){
                        printf(",");
                    };
                }
                printf(" ] ");
                break;
            };
            case AST_DICTIONARY_DEFINITION : {
                printf("[");
                for (size_t j = 0; j < visited_ast->dictionary_size; j++) {
                    AST_T* name = Visitor_Visit(visitor, visited_ast->dictionary_definition_value_name[j]);
                    AST_T* elem = Visitor_Visit(visitor, visited_ast->dictionary_definition_value[j]);
                    switch (elem->type) {
                        case AST_STRING : printf(" '%s' : %s", name->string_value, elem->string_value); break;
                        case AST_NUMBER : printf(" '%s' : %g", name->string_value, elem->number_value); break;
                        case AST_INTEGER : printf(" '%s' : %i", name->string_value, elem->integer_value); break;
                        case AST_BOOL: printf(" '%s' : %s", name->string_value, elem->bool_value ? "true" : "false"); break;
                        case AST_NULL: printf(" '%s' : null", name->string_value); break;
                        case AST_CLASS : printf(" '%s' : %s ", name->string_value, elem->class_name); break;
                        case AST_CLASS_INSTANTIATION :
                            printf(" '%s' : %s", name->string_value, elem->instance_class_name);
                            break;
                        case AST_TUPLE_DEFINITION : printf(" '%s' : ", name->string_value); AST_T** new_argse = calloc(1, sizeof(struct AST_STRUCT*)); new_argse[0] = elem; builtin_function_print(visitor, new_argse, 1); break;
                        case AST_TABLE_DEFINITION : printf(" '%s' : ", name->string_value); AST_T** new_args = calloc(1, sizeof(struct AST_STRUCT*)); new_args[0] = elem; builtin_function_print(visitor, new_args, 1); break;
                        case AST_DICTIONARY_DEFINITION : printf(" '%s' : ", name->string_value); AST_T** new_args2 = calloc(1, sizeof(struct AST_STRUCT*)); new_args2[0] = elem; builtin_function_print(visitor, new_args2, 1); break;
                        default : printf(" '%s' : %p", name->string_value, elem); break;
                    };
                    if (j < (visited_ast->dictionary_size-1)){
                        printf(",");
                    };
                }
                printf(" ] ");
                break;
            };
            default: printf("%p ", visited_ast); break;
        }
    };

    return Init_AST(AST_NOOP);
};

AST_T* builtin_function_err(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size <= 0) {
        Iris_Throw(visitor, NULL, "function 'err'", "argument underflow. (0 to 1)");
    };

    printf(RED "[ERROR]: " RESET);

    for (int i=0;i<args_size;i++){
        AST_T* visited_ast = Visitor_Visit(visitor, args[i]);

        switch (visited_ast->type) {
            case AST_STRING: {
                if (strcmp(visited_ast->string_value, "%n") == 0) {
                    printf("\n");
                } else {
                    char *current = visited_ast->string_value;
                    char *next_match = strstr(current, "%n");
                    while (next_match != NULL) {
                        printf("%.*s", (int)(next_match - current), current);
                        printf("\n");
                        current = next_match + 2;
                        next_match = strstr(current, "%n");
                    };
                    printf("%s", current);
                };
                break;
            };
            case AST_BOOL: printf("%s", visited_ast->bool_value ? "true " : "false "); break;
            case AST_NULL: printf("null "); break;
            case AST_NUMBER : printf("%g ", visited_ast->number_value); break;
            case AST_INTEGER : printf("%i ", visited_ast->integer_value); break;
            case AST_CLASS : printf("%s ", visited_ast->class_name); break;
            case AST_CLASS_INSTANTIATION : printf("%s ", visited_ast->instance_class_name); break;
            case AST_TUPLE_DEFINITION : {
                printf("(");
                for (size_t j = 0; j < visited_ast->tuple_size; j++) {
                    AST_T* elem = Visitor_Visit(visitor, visited_ast->tuple_definition_value[j]);
                    switch (elem->type) {
                        case AST_STRING : printf(" %s", elem->string_value); break;
                        case AST_NUMBER : printf(" %g", elem->number_value); break;
                        case AST_INTEGER : printf("%i ", elem->integer_value); break;
                        case AST_BOOL: printf(" %s", elem->bool_value ? "true" : "false"); break;
                        case AST_NULL: printf(" null"); break;
                        case AST_CLASS : printf(" %s ", elem->class_name); break;
                        case AST_CLASS_INSTANTIATION :
                            printf(" %s", elem->instance_class_name);
                            break;
                        case AST_TUPLE_DEFINITION : printf(" "); AST_T** new_argse = calloc(1, sizeof(struct AST_STRUCT*)); new_argse[0] = elem; builtin_function_print(visitor, new_argse, 1); break;
                        case AST_TABLE_DEFINITION : printf(" "); AST_T** new_args = calloc(1, sizeof(struct AST_STRUCT*)); new_args[0] = elem; builtin_function_print(visitor, new_args, 1); break;
                        case AST_DICTIONARY_DEFINITION : printf(" "); AST_T** new_argss = calloc(1, sizeof(struct AST_STRUCT*)); new_argss[0] = elem; builtin_function_print(visitor, new_argss, 1); break;
                        default : printf(" %p", elem); break;
                    }
                    if (j < (visited_ast->tuple_size-1)){
                        printf(",");
                    };
                }
                printf(" )");
                break;
            };
            case AST_TABLE_DEFINITION : {
                printf("[");
                for (size_t j = 0; j < visited_ast->table_size; j++) {
                    AST_T* elem = Visitor_Visit(visitor, visited_ast->table_definition_value[j]);
                    switch (elem->type) {
                        case AST_STRING : printf(" %s", elem->string_value); break;
                        case AST_NUMBER : printf(" %g", elem->number_value); break;
                        case AST_INTEGER : printf("%i ", elem->integer_value); break;
                        case AST_BOOL: printf(" %s", elem->bool_value ? "true" : "false"); break;
                        case AST_NULL: printf(" null"); break;
                        case AST_CLASS : printf(" %s ", elem->class_name); break;
                        case AST_CLASS_INSTANTIATION :
                            printf(" %s", elem->instance_class_name);
                            break;
                        case AST_TUPLE_DEFINITION : printf(" "); AST_T** new_argse = calloc(1, sizeof(struct AST_STRUCT*)); new_argse[0] = elem; builtin_function_print(visitor, new_argse, 1); break;
                        case AST_TABLE_DEFINITION : printf(" "); AST_T** new_args = calloc(1, sizeof(struct AST_STRUCT*)); new_args[0] = elem; builtin_function_print(visitor, new_args, 1); break;
                        case AST_DICTIONARY_DEFINITION : printf(" "); AST_T** new_argss = calloc(1, sizeof(struct AST_STRUCT*)); new_argss[0] = elem; builtin_function_print(visitor, new_argss, 1); break;
                        default : printf(" %p", elem); break;
                    }
                    if (j < (visited_ast->table_size-1)){
                        printf(",");
                    };
                }
                printf(" ] ");
                break;
            };
            case AST_DICTIONARY_DEFINITION : {
                printf("[");
                for (size_t j = 0; j < visited_ast->dictionary_size; j++) {
                    AST_T* name = Visitor_Visit(visitor, visited_ast->dictionary_definition_value_name[j]);
                    AST_T* elem = Visitor_Visit(visitor, visited_ast->dictionary_definition_value[j]);
                    switch (elem->type) {
                        case AST_STRING : printf(" '%s' : %s", name->string_value, elem->string_value); break;
                        case AST_NUMBER : printf(" '%s' : %g", name->string_value, elem->number_value); break;
                        case AST_INTEGER : printf(" '%s' : %i", name->string_value, elem->integer_value); break;
                        case AST_BOOL: printf(" '%s' : %s", name->string_value, elem->bool_value ? "true" : "false"); break;
                        case AST_NULL: printf(" '%s' : null", name->string_value); break;
                        case AST_CLASS : printf(" '%s' : %s", name->string_value, elem->class_name); break;
                        case AST_CLASS_INSTANTIATION :
                            printf(" '%s' : %s", name->string_value, elem->instance_class_name);
                            break;
                        case AST_TUPLE_DEFINITION : printf(" '%s' : ", name->string_value); AST_T** new_argse = calloc(1, sizeof(struct AST_STRUCT*)); new_argse[0] = elem; builtin_function_print(visitor, new_argse, 1); break;
                        case AST_TABLE_DEFINITION : printf(" '%s' : ", name->string_value); AST_T** new_args = calloc(1, sizeof(struct AST_STRUCT*)); new_args[0] = elem; builtin_function_print(visitor, new_args, 1); break;
                        case AST_DICTIONARY_DEFINITION : printf(" '%s' : ", name->string_value); AST_T** new_args2 = calloc(1, sizeof(struct AST_STRUCT*)); new_args2[0] = elem; builtin_function_print(visitor, new_args2, 1); break;
                        default : printf(" '%s' : %p", name->string_value, elem); break;
                    };
                    if (j < (visited_ast->dictionary_size-1)){
                        printf(",");
                    };
                }
                printf(" ] ");
                break;
            };
            default: printf("%p ", visited_ast); break;
        }
    };

    exit(1);

    return Init_AST(AST_NOOP);
};

AST_T* builtin_function_exit(Visitor_T* visitor, AST_T** args, int args_size){
    exit(1);
    return Init_AST(AST_NOOP);
};

AST_T* builtin_function_type(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size > 1) {
        Iris_Throw(visitor, NULL, "function 'type'", "argument overflow. (%d to 1)", args_size);
    } else if (args_size < 1) {
        Iris_Throw(visitor, NULL, "function 'type'", "argument underflow. (1 to %d)", args_size);
    }

    AST_T* newvar = Init_AST(AST_STRING);

    AST_T* var = Visitor_Visit(visitor, args[0]);

    switch (var->type) {
        case AST_STRING:
            newvar->string_value = "STRING";
            return newvar;
            break;
        case AST_NUMBER:
            newvar->string_value = "NUMBER";
            return newvar;
            break;
        case AST_INTEGER:
            newvar->string_value = "INTEGER";
            return newvar;
            break;
        case AST_BOOL:
            newvar->string_value = "BOOL";
            return newvar;
            break;
        case AST_NULL:
            newvar->string_value = "NULL";
            return newvar;
            break;
        case AST_TUPLE:
            newvar->string_value = "TUPLE";
            return newvar;
            break;
        case AST_TUPLE_DEFINITION:
            newvar->string_value = "TUPLE";
            return newvar;
            break;
        case AST_TABLE:
            newvar->string_value = "TABLE";
            return newvar;
            break;
        case AST_TABLE_DEFINITION:
            newvar->string_value = "TABLE";
            return newvar;
            break;
        case AST_DICTIONARY:
            newvar->string_value = "DICTIONARY";
            return newvar;
            break;
        case AST_DICTIONARY_DEFINITION:
            newvar->string_value = "DICTIONARY";
            return newvar;
            break;
        case AST_CLASS:
            newvar->string_value = "CLASS";
            return newvar;
            break;
        case AST_CLASS_DEFINITION:
            newvar->string_value = "CLASS";
            return newvar;
            break;
        case AST_CLASS_INSTANTIATION:
            newvar->string_value = var->instance_class_name;
            return newvar;
            break;
        default:
            Iris_Throw(visitor, var, "function 'type'", "unsupported type");
    };

    return Init_AST(AST_NOOP);
};

AST_T* builtin_function_dict_get_from_index(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size > 2) {
        Iris_Throw(visitor, NULL, "function 'dict_get_from_index'", "argument overflow. (%d to 2)", args_size);
    } else if (args_size < 2) {
        Iris_Throw(visitor, NULL, "function 'dict_get_from_index'", "argument underflow. (2 to %d)", args_size);
    }

    AST_T* dict = Visitor_Visit(visitor, args[0]);
    AST_T* index = Visitor_Visit(visitor, args[1]);

    if (dict->type != AST_DICTIONARY_DEFINITION) {
        Iris_Throw(visitor, dict, "function 'dict_get_from_index'", "type of argument 1 expects a dictionary but did not receive one");
    };
    if (dict == NULL || dict->dictionary_definition_value == NULL || dict->dictionary_definition_value_name == NULL) {
        Iris_Throw(visitor, dict, "function 'dict_get_from_index'", "argument 1 is null");
    };
    if (index->type != AST_STRING) {
        Iris_Throw(visitor, index, "function 'dict_get_from_index'", "type of argument 2 expects a string but did not receive one");
    };
    if (index->string_value == (void*)0){
        Iris_Throw(visitor, index, "function 'dict_get_from_index'", "argument 2 is null");
    };

    AST_T* d_val;
    AST_T* d_val_name;

    for (int x=0;x<dict->dictionary_size;x++) {
        d_val = Visitor_Visit(visitor, dict->dictionary_definition_value[x]);
        d_val_name = Visitor_Visit(visitor, dict->dictionary_definition_value_name[x]);
        if (strcmp(index->string_value, d_val_name->string_value) == 0){
            return d_val;
        };
    };

    d_val = Init_AST(AST_BOOL);
    d_val->bool_value = false;

    return d_val;
};

AST_T* builtin_function_dict_get_index(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size > 2) {
        Iris_Throw(visitor, NULL, "function 'dict_get_index'", "argument overflow. (%d to 2)", args_size);
    } else if (args_size < 2) {
        Iris_Throw(visitor, NULL, "function 'dict_get_index'", "argument underflow. (2 to %d)", args_size);
    }

    AST_T* dict = Visitor_Visit(visitor, args[0]);
    AST_T* value = Visitor_Visit(visitor, args[1]);

    if (dict == NULL || dict->type != AST_DICTIONARY_DEFINITION) {
        Iris_Throw(visitor, dict, "function 'dict_get_index'", "type of argument 1 expects a table but did not receive one");
    }
    if (value == NULL || value->type == AST_NOOP) {
        Iris_Throw(visitor, value, "function 'dict_get_index'", "type of argument 2 expects any value but did not receive one");
    };

    AST_T* num = Init_AST(AST_INTEGER);
    AST_T* indx = Init_AST(AST_STRING);

    for (int x=0;x<dict->dictionary_size;x++) {
        AST_T* d_val = Visitor_Visit(visitor, dict->dictionary_definition_value[x]);
        if (d_val->type != value->type) {
            continue;
        } else {
            switch (d_val->type) {
                case AST_CLASS_DEFINITION: {
                    if (d_val->class_definition_value == value->class_definition_value) {
                        num->integer_value = x;
                        return num;
                        break;
                    };
                };
                case AST_DICTIONARY_DEFINITION: {
                    if (d_val->dictionary_definition_value == value->dictionary_definition_value) {
                        num->integer_value = x;
                        return num;
                        break;
                    };
                };
                case AST_TABLE_DEFINITION: {
                    if (d_val->table_definition_value == value->table_definition_value) {
                        num->integer_value = x;
                        return num;
                        break;
                    };
                };
                case AST_STRING: {
                    if (strcmp(d_val->string_value, value->string_value) == 0) {
                        num->integer_value = x;
                        return num;
                        break;
                    };
                };
                case AST_NUMBER: {
                    if (d_val->number_value == value->number_value) {
                        num->integer_value = x;
                        return num;
                        break;
                    };
                };
                case AST_INTEGER: {
                    if (d_val->integer_value == value->integer_value) {
                        num->integer_value = x;
                        return num;
                        break;
                    };
                };
                case AST_BOOL: {
                    if (d_val->bool_value == value->bool_value) {
                        num->integer_value = x;
                        return num;
                        break;
                    };
                };
                default: num->integer_value = -1; return num; break;
            };
        };
    };

    if (num->integer_value != -1) {
        indx->string_value = dict->dictionary_definition_value_name[num->integer_value]->string_value;
    } else {
        indx->string_value = "NULL";
    };
    return indx;
};

AST_T* builtin_function_dict_set_index(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size > 3) {
        Iris_Throw(visitor, NULL, "function 'dict_set_index'", "argument overflow. (%d to 3)", args_size);
    } else if (args_size < 3) {
        Iris_Throw(visitor, NULL, "function 'dict_set_index'", "argument underflow. (3 to %d)", args_size);
    }

    AST_T* dict = Visitor_Visit(visitor, args[0]);
    AST_T* index = Visitor_Visit(visitor, args[1]);
    AST_T* value = Visitor_Visit(visitor, args[2]);

    if (dict == NULL || dict->type != AST_DICTIONARY_DEFINITION) {
        Iris_Throw(visitor, dict, "function 'dict_set_index'", "type of argument 1 expects a dictionary but did not receive one");
    };
    if (dict->variable_const) {
        Iris_Throw(visitor, dict, "function 'dict_set_index'", "dictionary is a const");
    };
    if (index == NULL || index->type != AST_STRING) {
        Iris_Throw(visitor, index, "function 'dict_set_index'", "type of argument 2 expects a string but did not receive one");
    }
    if (value == NULL || value->type == AST_NOOP) {
        Iris_Throw(visitor, value, "function 'dict_set_index'", "type of argument 3 expects any value but did not receive one");
    };

    for (int x = 0;x<dict->dictionary_size;x++) {
        AST_T* d_name = Visitor_Visit(visitor, dict->dictionary_definition_value_name[x]);
        if (strcmp(d_name->string_value, index->string_value) == 0) {
            dict->dictionary_definition_value[x] = value;
            return Init_AST(AST_NOOP);
        };
    };

    dict->dictionary_size += 1;
    size_t new_size = dict->dictionary_size;

    dict->dictionary_definition_value = realloc(
        dict->dictionary_definition_value,
        new_size * sizeof(struct AST_STRUCT*)
    );
    dict->dictionary_definition_value_name = realloc(
        dict->dictionary_definition_value_name,
        new_size * sizeof(struct AST_STRUCT*)
    );

    dict->dictionary_definition_value[new_size-1] = value;
    dict->dictionary_definition_value_name[new_size-1] = index;

    return Init_AST(AST_NOOP);
};

AST_T* builtin_function_tuple_get_from_index(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size > 2) {
        Iris_Throw(visitor, NULL, "function 'tuple_get_from_index'", "argument overflow. (%d to 2)", args_size);
    } else if (args_size < 2) {
        Iris_Throw(visitor, NULL, "function 'tuple_get_from_index'", "argument underflow. (2 to %d)", args_size);
    }

    AST_T* tuple = Visitor_Visit(visitor, args[0]);
    AST_T* index = Visitor_Visit(visitor, args[1]);

    if (tuple == NULL || tuple->type != AST_TUPLE_DEFINITION) {
        Iris_Throw(visitor, tuple, "function 'tuple_get_from_index'", "type of argument 1 expects a tuple but did not receive one");
    }
    if (index->type != AST_INTEGER) {
        Iris_Throw(visitor, index, "function 'tuple_get_from_index'", "type of argument 2 expects an integer but did not receive one");
    };

    AST_T* ast_var = Init_AST(AST_BOOL);

    if (index->integer_value <= 0){
        ast_var->bool_value = false;
        return ast_var;
    };
    if ((size_t)index->integer_value > tuple->tuple_size){
        ast_var->bool_value = false;
        return ast_var;
    };

    AST_T* elem = Visitor_Visit(visitor, tuple->tuple_definition_value[index->integer_value - 1]);
    ast_var->type = elem->type;
    ast_var->scope = tuple->scope;

    switch (elem->type) {
        case AST_STRING: ast_var->type = AST_STRING; ast_var->scope = tuple->scope; ast_var->string_value = elem->string_value; return ast_var; break;
        case AST_NUMBER: ast_var->type = AST_NUMBER; ast_var->scope = tuple->scope; ast_var->number_value = elem->number_value; return ast_var; break;
        case AST_INTEGER: ast_var->type = AST_INTEGER; ast_var->scope = tuple->scope; ast_var->integer_value = elem->integer_value; return ast_var; break;
        case AST_BOOL: ast_var->type = AST_BOOL; ast_var->scope = tuple->scope; ast_var->bool_value = elem->bool_value; return ast_var; break;
        case AST_NULL: ast_var->type = AST_NULL; ast_var->scope = tuple->scope; return ast_var; break;
        case AST_TUPLE_DEFINITION : {
            for (size_t i=0;i<ast_var->tuple_size;i++) {
                ast_var->tuple_definition_value[i] = elem->tuple_definition_value[i];
            };
            return ast_var;
            break;
        }
        case AST_TABLE_DEFINITION : {
            for (size_t i=0;i<ast_var->table_size;i++) {
                ast_var->table_definition_value[i] = elem->table_definition_value[i];
            };
            return ast_var;
            break;
        }
        case AST_DICTIONARY_DEFINITION : {
            for (size_t i=0;i<ast_var->dictionary_size;i++) {
                ast_var->dictionary_definition_value_name[i] = elem->dictionary_definition_value_name[i];
                ast_var->dictionary_definition_value[i] = elem->dictionary_definition_value[i];
            };
            return ast_var;
            break;
        }
        case AST_CLASS_DEFINITION : {
            for (size_t i=0;i<ast_var->class_size;i++) {
                ast_var->class_definition_value_name[i] = elem->class_definition_value_name[i];
                ast_var->class_definition_value[i] = elem->class_definition_value[i];
            };
            return ast_var;
            break;
        }
        default: Iris_Throw(visitor, elem, "function 'tuple_get_from_index'", "unsupported type %d", elem->type);
    };

    return Init_AST(AST_NOOP);
}

AST_T* builtin_function_table_get_from_index(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size > 2) {
        Iris_Throw(visitor, NULL, "function 'table_get_from_index'", "argument overflow. (%d to 2)", args_size);
    } else if (args_size < 2) {
        Iris_Throw(visitor, NULL, "function 'table_get_from_index'", "argument underflow. (2 to %d)", args_size);
    }

    AST_T* table = Visitor_Visit(visitor, args[0]);
    AST_T* index = Visitor_Visit(visitor, args[1]);

    if (table == NULL || table->type != AST_TABLE_DEFINITION) {
        Iris_Throw(visitor, table, "function 'table_get_from_index'", "type of argument 1 expects a table but did not receive one");
    }
    if (index->type != AST_INTEGER) {
        Iris_Throw(visitor, index, "function 'table_get_from_index'", "type of argument 2 expects an integer but did not receive one");
    };

    AST_T* ast_var = Init_AST(AST_BOOL);

    if (index->integer_value <= 0){
        ast_var->bool_value = false;
        return ast_var;
    };
    if ((size_t)index->integer_value > table->table_size){
        ast_var->bool_value = false;
        return ast_var;
    };

    AST_T* elem = Visitor_Visit(visitor, table->table_definition_value[index->integer_value - 1]);
    ast_var->type = elem->type;
    ast_var->scope = table->scope;

    switch (elem->type) {
        case AST_STRING: ast_var->type = AST_STRING; ast_var->scope = table->scope; ast_var->string_value = elem->string_value; return ast_var; break;
        case AST_NUMBER: ast_var->type = AST_NUMBER; ast_var->scope = table->scope; ast_var->number_value = elem->number_value; return ast_var; break;
        case AST_INTEGER: ast_var->type = AST_INTEGER; ast_var->scope = table->scope; ast_var->integer_value = elem->integer_value; return ast_var; break;
        case AST_BOOL: ast_var->type = AST_BOOL; ast_var->scope = table->scope; ast_var->bool_value = elem->bool_value; return ast_var; break;
        case AST_NULL: ast_var->type = AST_NULL; ast_var->scope = table->scope; return ast_var; break;
        case AST_TABLE_DEFINITION : {
            for (size_t i=0;i<ast_var->table_size;i++) {
                ast_var->table_definition_value[i] = elem->table_definition_value[i];
            };
            return ast_var;
            break;
        }
        case AST_DICTIONARY_DEFINITION : {
            for (size_t i=0;i<ast_var->dictionary_size;i++) {
                ast_var->dictionary_definition_value_name[i] = elem->dictionary_definition_value_name[i];
                ast_var->dictionary_definition_value[i] = elem->dictionary_definition_value[i];
            };
            return ast_var;
            break;
        }
        case AST_CLASS_DEFINITION : {
            for (size_t i=0;i<ast_var->class_size;i++) {
                ast_var->class_definition_value_name[i] = elem->class_definition_value_name[i];
                ast_var->class_definition_value[i] = elem->class_definition_value[i];
            };
            return ast_var;
            break;
        }
        default: Iris_Throw(visitor, elem, "function 'table_get_from_index'", "unsupported type %d", elem->type);
    };

    return Init_AST(AST_NOOP);
};

AST_T* builtin_function_table_get_index(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size > 2) {
        Iris_Throw(visitor, NULL, "function 'table_get_index'", "argument overflow. (%d to 2)", args_size);
    } else if (args_size < 2) {
        Iris_Throw(visitor, NULL, "function 'table_get_index'", "argument underflow. (2 to %d)", args_size);
    }

    AST_T* table = Visitor_Visit(visitor, args[0]);
    AST_T* value = Visitor_Visit(visitor, args[1]);

    if (table == NULL || table->type != AST_TABLE_DEFINITION) {
        Iris_Throw(visitor, table, "function 'table_get_index'", "type of argument 1 expects a table but did not receive one");
    }
    if (value == NULL || value->type == AST_NOOP) {
        Iris_Throw(visitor, value, "function 'table_get_index'", "type of argument 2 expects any value but did not receive one");
    };

    AST_T* num = Init_AST(AST_INTEGER);

    for (int x=0;x<table->table_size;x++) {
        AST_T* t_val = Visitor_Visit(visitor, table->table_definition_value[x]);
        if (t_val->type != value->type) {
            num->integer_value = 0;
            return num;
        } else {
            switch (t_val->type) {
                case AST_CLASS_DEFINITION: {
                    if (t_val->class_definition_value == value->class_definition_value) {
                        num->integer_value = x;
                        return num;
                        break;
                    };
                };
                case AST_DICTIONARY_DEFINITION: {
                    if (t_val->dictionary_definition_value == value->dictionary_definition_value) {
                        num->integer_value = x;
                        return num;
                        break;
                    };
                };
                case AST_TABLE_DEFINITION: {
                    if (t_val->table_definition_value == value->table_definition_value) {
                        num->integer_value = x;
                        return num;
                        break;
                    };
                };
                case AST_STRING: {
                    if (strcmp(t_val->string_value, value->string_value) == 0) {
                        num->integer_value = x;
                        return num;
                        break;
                    };
                };
                case AST_NUMBER: {
                    if (t_val->number_value == value->number_value) {
                        num->integer_value = x;
                        return num;
                        break;
                    };
                };
                case AST_INTEGER: {
                    if (t_val->integer_value == value->integer_value) {
                        num->integer_value = x;
                        return num;
                        break;
                    };
                };
                case AST_BOOL: {
                    if (t_val->bool_value == value->bool_value) {
                        num->integer_value = x;
                        return num;
                        break;
                    };
                };
                case AST_NULL: {
                    if (t_val->type == value->type) {
                        num->integer_value = x;
                        return num;
                        break;
                    };
                };
                default: num->integer_value = -1; break;
            };
        };
    };

    return num;
};

AST_T* builtin_function_table_set_index(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size > 3) {
        Iris_Throw(visitor, NULL, "function 'table_set_index'", "argument overflow. (%d to 3)", args_size);
    } else if (args_size < 3) {
        Iris_Throw(visitor, NULL, "function 'table_set_index'", "argument underflow. (3 to %d)", args_size);
    }

    AST_T* table = Visitor_Visit(visitor, args[0]);
    AST_T* index = Visitor_Visit(visitor, args[1]);
    AST_T* value = Visitor_Visit(visitor, args[2]);

    if (table == NULL || table->type != AST_TABLE_DEFINITION) {
        Iris_Throw(visitor, table, "function 'table_set_index'", "type of argument 1 expects a table but did not receive one");
    };
    if (table->variable_const) {
        Iris_Throw(visitor, table, "function 'table_set_index'", "table is a constant");
    };
    if (index== NULL || index->type != AST_INTEGER) {
        Iris_Throw(visitor, index, "function 'table_set_index'", "type of argument 2 expects an integer but did not receive one");
    }
    if (value == NULL || value->type == AST_NOOP) {
        Iris_Throw(visitor, value, "function 'table_set_index'", "type of argument 3 expects any value but did not receive one");
    }
    if ((int)index->integer_value <= 0){
        Iris_Throw(visitor, index, "function 'table_set_index'", "index is out of bounds (0)");
    };
    if ((size_t)index->number_value > table->table_size){
        size_t new_size = (size_t)index->integer_value;
        table->table_definition_value = realloc(
            table->table_definition_value,
            new_size * sizeof(struct AST_STRUCT*)
        );
        table->table_size = new_size;
    };

    table->table_definition_value[index->integer_value - 1] = value;

    return Init_AST(AST_NOOP);
};

AST_T* builtin_function_char_at(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size > 2) {
        Iris_Throw(visitor, NULL, "function 'charAt'", "argument overflow. (%d to 2)", args_size);
    } else if (args_size < 2) {
        Iris_Throw(visitor, NULL, "function 'charAt'", "argument underflow. (2 to %d)", args_size);
    }

    AST_T* stringg = Visitor_Visit(visitor, args[0]);
    AST_T* index = Visitor_Visit(visitor, args[1]);

    if (stringg->type != AST_STRING) {
        Iris_Throw(visitor, stringg, "function 'charAt'", "type of argument 1 expects a string but did not receive one");
    }
    if (index->type != AST_INTEGER) {
        Iris_Throw(visitor, index, "function 'charAt'", "type of argument 2 expects an integer but did not receive one");
    }

    AST_T* ast_var = Init_AST(AST_STRING);
    ast_var->scope = stringg->scope;

    if (!ast_var->string_value) exit(EXIT_FAILURE);

    if (index->integer_value <= 0){
        ast_var->string_value = malloc(5);
        ast_var->string_value[0] = 'N';
        ast_var->string_value[1] = 'U';
        ast_var->string_value[2] = 'L';
        ast_var->string_value[3] = 'L';
        ast_var->string_value[4] = '\0';
        return ast_var;
    };
    if ((size_t)index->integer_value > strlen(stringg->string_value)){
        ast_var->string_value = malloc(5);
        ast_var->string_value[0] = 'N';
        ast_var->string_value[1] = 'U';
        ast_var->string_value[2] = 'L';
        ast_var->string_value[3] = 'L';
        ast_var->string_value[4] = '\0'; // i have such a low sanity.
        return ast_var;
    };
    char c = stringg->string_value[index->integer_value-1];
    ast_var->string_value = malloc(2); // a char is 1 byte in size, so no need to multiply.

    ast_var->string_value[0] = c;
    ast_var->string_value[1] = '\0';

    return ast_var;
};

AST_T* builtin_function_str_edit(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size > 3) {
        Iris_Throw(visitor, NULL, "function 'str_edit'", "argument overflow. (%d to 3)", args_size);
    } else if (args_size < 3) {
        Iris_Throw(visitor, NULL, "function 'stredit'", "argument underflow. (3 to %d)", args_size);
    }

    AST_T* str = Visitor_Visit(visitor, args[0]);
    AST_T* index = Visitor_Visit(visitor, args[1]);
    AST_T* value = Visitor_Visit(visitor, args[2]);

    if (str == NULL || str->type != AST_STRING) {
        Iris_Throw(visitor, str, "function 'stredit'", "type of argument 1 expects a string but did not receive one");
    };

    if (args[0]->type == AST_VARIABLE) {
        AST_T* variable = Scope_Get_Variable_Definition(args[0]->scope, args[0]->variable_name);

        if (variable->variable_const == true) {
            Iris_Throw(visitor, str, "function 'stredit'", "type of argument 1 is a const");
        };
    };
    if (index == NULL || index->type != AST_INTEGER) {
        Iris_Throw(visitor, index, "function 'stredit'", "type of argument 2 expects an integer but did not receive one");
    }
    if (index->integer_value <= 0){
        Iris_Throw(visitor, index, "function 'stredit'", "index is out of bounds (0)");
    };
    if (value == NULL || value->type != AST_STRING) {
        Iris_Throw(visitor, value, "function 'stredit'", "type of argument 3 expects a string but did not receive one");
    };
    size_t length = strlen(value->string_value);

    if (length == 1) {
        char c = value->string_value[0];

        str->string_value[index->integer_value-1] = c;
    } else {
        printf("Failed on function 'stredit', string value is of a size greater/less than one (at line %d) - note, stredit can only edit one character at ONE index\n", value->current_line);
    };

    return Init_AST(AST_NOOP);
};

AST_T* builtin_function_for_each(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size < 2) {
        Iris_Throw(visitor, NULL, "function 'ForEach'", "argument underflow (2 to %d)", args_size);
    }

    AST_T* str = Visitor_Visit(visitor, args[0]);
    AST_T* func_name = Visitor_Visit(visitor, args[1]);

    if (str == NULL || str->type != AST_STRING) {
        Iris_Throw(visitor, str, "function 'ForEach'", "type of argument 1 expects a string but did not receive one");
    };
    if (func_name == NULL || func_name->type != AST_STRING) {
        Iris_Throw(visitor, func_name, "function 'ForEach'", "type of argument 2 expects a function name (string) but did not receive one");
    };

    AST_T* node = Init_AST(AST_FUNCTION_CALL);
    node->scope = func_name->scope;
    node->function_call_name = func_name->string_value;
    node->function_call_arguments_size = 0;

    size_t siz = 0;

    siz = strlen(str->string_value);

    for (int in=0;in<siz;in++) {
        AST_T* fdef = Scope_Get_Function_Definition(node->scope, node->function_call_name);

        if (fdef == (void*)0) {
            Iris_Throw(visitor, node, "undefined method", "'%s'", node->function_call_name);
        }
        if (fdef->function_definition_public == false) {
            Iris_Throw(visitor, func_name, "function 'ForEach'", "type of argument 2 expects a public function");
        };
        if (node->function_call_arguments_size != fdef->function_definition_args_size) {
            Iris_Throw(visitor, node, "function call", "'%s', expected %zu args, got %zu", node->function_call_name, fdef->function_definition_args_size, node->function_call_arguments_size);
        }
        visitor->call_depth++;
        Scope_T* call_scope = fdef->function_definition_body->scope;
        size_t saved_scope_size = call_scope->variable_definitions_size;

        AST_T** evaluated_args = (AST_T**) calloc(node->function_call_arguments_size, sizeof(struct AST_STRUCT*));
        for (int i=0;i<(int)node->function_call_arguments_size;i++){
            evaluated_args[i] = Visitor_Visit(visitor, node->function_call_arguments[i]);
        }
        for (int i=0;i<(int)node->function_call_arguments_size;i++){
            AST_T* ast_var = (AST_T*) fdef->function_definition_args[i];

            AST_T* ast_vardef = Init_AST(AST_VARIABLE_DEFINITION);
            ast_vardef->variable_definition_value = evaluated_args[i];
            ast_vardef->variable_definition_variable_name = (char*) calloc(strlen(ast_var->variable_name) + 1, sizeof(char));
            strcpy(ast_vardef->variable_definition_variable_name, ast_var->variable_name);
            Scope_Add_Variable_Definition(call_scope, ast_vardef);
        }
        free(evaluated_args);
        Visitor_Visit(visitor, fdef->function_definition_body);
        visitor->returning = 0;
        visitor->breaking = false;
        visitor->call_depth--;

        call_scope->variable_definitions_size = saved_scope_size;
    };
    return Init_AST(AST_NOOP);
};

AST_T* builtin_function_to_integer(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size > 1) {
        Iris_Throw(visitor, NULL, "function 'toInteger'", "argument overflow. (%d to 1)", args_size);
    } else if (args_size < 1) {
        Iris_Throw(visitor, NULL, "function 'toInteger'", "argument underflow. (1 to %d)", args_size);
    }

    AST_T* v = Visitor_Visit(visitor, args[0]);

    if (v == NULL || (v->type != AST_STRING && v->type != AST_NUMBER)) {
        Iris_Throw(visitor, v, "function 'toInteger'", "ineligible type for conversion (%d)", v->type);
    };

    if (v->type == AST_STRING) {
        if (v->string_value == NULL) {
            Iris_Throw(visitor, v, "function 'toInteger'", "got empty string");
        }
    }

    AST_T* num = Init_AST(AST_INTEGER);

    int numi = (v->type == AST_STRING) ? (int)strtof(v->string_value, NULL) : (int)v->number_value;
    num->integer_value=numi;

    return num;
};


AST_T* builtin_function_to_number(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size > 1) {
        Iris_Throw(visitor, NULL, "function 'toNumber'", "argument overflow. (%d to 1)", args_size);
    } else if (args_size < 1) {
        Iris_Throw(visitor, NULL, "function 'toNumber'", "argument underflow. (1 to %d)", args_size);
    }

    AST_T* stringg = Visitor_Visit(visitor, args[0]);

    if (stringg == NULL || stringg->type != AST_STRING) {
        Iris_Throw(visitor, stringg, "function 'toNumber'", "ineligible type for conversion (%d)", stringg->type);
    };

    if (stringg->string_value == NULL) {
        Iris_Throw(visitor, stringg, "function 'toNumber'", "got empty string");
    }

    AST_T* num = Init_AST(AST_NUMBER);

    float numf = strtod(stringg->string_value, NULL);
    num->number_value=numf;

    return num;
};

AST_T* builtin_function_to_string(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size > 1) {
        Iris_Throw(visitor, NULL, "function 'toString'", "argument overflow. (%d to 1)", args_size);
    } else if (args_size < 1) {
        Iris_Throw(visitor, NULL, "function 'toString'", "argument underflow. (1 to %d)", args_size);
    }

    AST_T* num = Visitor_Visit(visitor, args[0]);

    if (num == NULL || num->type != AST_NUMBER || num->type != AST_INTEGER) {
        Iris_Throw(visitor, num, "function 'toString'", "ineligible type for conversion (%d)", num->type);
    };

    int len = snprintf(NULL, 0, "%f", (num->type == AST_NUMBER) ? num->number_value : num->integer_value) + 1;

    AST_T* stringg = Init_AST(AST_STRING);
    stringg->string_value = calloc(len, sizeof(char));
    if (!stringg->string_value) {
        Iris_Throw(visitor, num, "function 'toString'", "memory allocation failed");
    };

    snprintf(stringg->string_value, len, "%f", num->number_value);

    return stringg;
};

AST_T* builtin_function_floor(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size > 2) {
        Iris_Throw(visitor, NULL, "function 'floor'", "argument overflow. (%d to 2)", args_size);
    } else if (args_size < 2) {
        Iris_Throw(visitor, NULL, "function 'floor'", "argument underflow. (2 to %d)", args_size);
    }

    AST_T* num = Visitor_Visit(visitor, args[0]);
    AST_T* floor_to = Visitor_Visit(visitor, args[1]);

    if (num == NULL || num->type != AST_NUMBER) {
        Iris_Throw(visitor, num, "function 'floor'", "ineligible type for conversion (%d)", num->type);
    };
    if (floor_to == NULL || floor_to->type != AST_NUMBER) {
        Iris_Throw(visitor, floor_to, "function 'floor'", "ineligible type for conversion (%d)", floor_to->type);
    };

    AST_T* rounded = Init_AST(AST_NUMBER);
    rounded->number_value=floor(num->number_value/floor_to->number_value) * floor_to->number_value;

    return rounded;
};

AST_T* builtin_function_read_file(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size > 1) {
        Iris_Throw(visitor, NULL, "function 'readFile'", "argument overflow. (%d to 1)", args_size);
    } else if (args_size < 1) {
        Iris_Throw(visitor, NULL, "function 'readFile'", "argument underflow. (1 to %d)", args_size);
    }

    AST_T* fileName = Visitor_Visit(visitor, args[0]);

    if (fileName == NULL || fileName->type != AST_STRING) {
        Iris_Throw(visitor, fileName, "function 'readFile'", "argument 1 expects a string but did not receive one");
    };
    if (fileName->string_value == NULL || strlen(fileName->string_value) <= 0) {
        Iris_Throw(visitor, fileName, "function 'readFile'", "argument 1 expects a string of size 1 or bigger");
    };

    AST_T* str = Init_AST(AST_STRING);
    str->string_value = calloc(1, sizeof(char));

    // do something here
    FILE* file = fopen(fileName->string_value, "r");

    char buffer[256];

    if (file==NULL){
        Iris_Throw(visitor, fileName, "function 'readFile'", "failed to open file '%s'", fileName->string_value);
    }

    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        str->string_value = realloc(str->string_value, strlen(str->string_value) + strlen(buffer) + 1);
        strcat(str->string_value, buffer);
    };

    fclose(file);

    return str;
};

AST_T* builtin_function_write_file(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size > 3) {
        Iris_Throw(visitor, NULL, "function 'writeFile'", "argument overflow. (%d to 3)", args_size);
    } else if (args_size < 3) {
        Iris_Throw(visitor, NULL, "function 'writeFile'", "argument underflow. (3 to %d)", args_size);
    }

    AST_T* fileName = Visitor_Visit(visitor, args[0]);
    AST_T* contents = Visitor_Visit(visitor, args[1]);
    AST_T* formatted = Visitor_Visit(visitor, args[2]);

    if (fileName == NULL || fileName->type != AST_STRING) {
        Iris_Throw(visitor, fileName, "function 'writeFile'", "argument 1 expects a string but did not receive one");
    };
    if (fileName->string_value == NULL || strlen(fileName->string_value) <= 0) {
        Iris_Throw(visitor, fileName, "function 'writeFile'", "argument 1 expects a string of size 1 or bigger");
    };
    if (contents == NULL || contents->type != AST_STRING) {
        Iris_Throw(visitor, contents, "function 'writeFile'", "argument 2 expects a string but did not receive one");
    };
    if (contents->string_value == NULL || strlen(contents->string_value) <= 0) {
        Iris_Throw(visitor, contents, "function 'writeFile'", "argument 2 expects a string of size 1 or bigger");
    };
    if (formatted == NULL || formatted->type != AST_BOOL) {
        Iris_Throw(visitor, formatted, "function 'writeFile'", "argument 3 expects a bool but did not receive one");
    };

    AST_T* success = Init_AST(AST_BOOL);
    success->bool_value = false;

    FILE* file = fopen(fileName->string_value, "w");

    if (file == NULL) {
        success->bool_value = false;
        return success;
    } else {
        if (formatted->bool_value) {
            fprintf(file, contents->string_value);
        } else {
            fputs(contents->string_value, file);
        };
        success->bool_value = true;
    };

    fclose(file);
    return success;
};

static size_t Scope_Compact_Variable_Definitions(Scope_T* scope) {
    size_t sizi = scope->variable_definitions_size;
    if (sizi == 0) return 0;

    char* keep = calloc(sizi, 1);
    if (!keep) exit(EXIT_FAILURE);

    for (size_t i = 0; i < sizi; i++) {
        const char* name_i = scope->variable_definitions[i]->variable_definition_variable_name;
        int is_last = 1;
        for (size_t j = i + 1; j < sizi; j++) {
            if (strcmp(name_i, scope->variable_definitions[j]->variable_definition_variable_name) == 0) {
                is_last = 0;
                break;
            }
        }
        keep[i] = (char)is_last;
    }

    size_t write = 0;
    size_t reclaimed = 0;
    for (size_t i = 0; i < sizi; i++) {
        if (keep[i]) {
            scope->variable_definitions[write++] = scope->variable_definitions[i];
        } else {
            free(scope->variable_definitions[i]);
            reclaimed++;
        }
    }
    free(keep);
    scope->variable_definitions_size = write;
    return reclaimed;
};

static size_t Scope_Compact_Function_Definitions(Scope_T* scope) {
    size_t sizi = scope->function_definitions_size;
    if (sizi == 0) return 0;

    char* keep = calloc(sizi, 1);
    if (!keep) exit(EXIT_FAILURE);

    for (size_t i = 0; i < sizi; i++) {
        const char* name_i = scope->function_definitions[i]->function_definition_name;
        int is_last = 1;
        for (size_t j = i + 1; j < sizi; j++) {
            if (strcmp(name_i, scope->function_definitions[j]->function_definition_name) == 0) {
                is_last = 0;
                break;
            }
        }
        keep[i] = (char)is_last;
    }

    size_t write = 0;
    for (size_t i = 0; i < sizi; i++) {
        if (keep[i]) {
            scope->function_definitions[write++] = scope->function_definitions[i];
        };
    }
    free(keep);
    size_t reclaimed = sizi - write;
    scope->function_definitions_size = write;
    return reclaimed;
};

AST_T* builtin_function_gc(Visitor_T* visitor, AST_T** args, int args_size) {
    if (args_size != 0) {
        Iris_Throw(visitor, NULL, "function 'gc'", "expected 0 arguments, got %d", args_size);
    }
    if (visitor->call_depth > 0) {
        Iris_Throw(visitor, NULL, "function 'gc'", "cannot collect while a function call is in progress (call it from the top level of your script, not from inside a function)");
    }

    extern Scope_T* g_iris_gc_root_scope;
    if (!g_iris_gc_root_scope) {
        AST_T* result = Init_AST(AST_NUMBER);
        result->number_value = 0;
        return result;
    }

    size_t reclaimed = 0;
    reclaimed += Scope_Compact_Variable_Definitions(g_iris_gc_root_scope);
    reclaimed += Scope_Compact_Function_Definitions(g_iris_gc_root_scope);

    AST_T* result = Init_AST(AST_NUMBER);
    result->number_value = (float)reclaimed;
    return result;
};

AST_T* builtin_function_gc_stats(Visitor_T* visitor, AST_T** args, int args_size) {
    extern Scope_T* g_iris_gc_root_scope;
    AST_T* result = Init_AST(AST_STRING);
    char buf[128];
    size_t var_size = g_iris_gc_root_scope ? g_iris_gc_root_scope->variable_definitions_size : 0;
    size_t fn_size = g_iris_gc_root_scope ? g_iris_gc_root_scope->function_definitions_size : 0;
    snprintf(buf, sizeof(buf), "vars=%zu funcs=%zu", var_size, fn_size);
    result->string_value = malloc(strlen(buf) + 1);
    if (!result->string_value) exit(EXIT_FAILURE);
    strcpy(result->string_value, buf);
    return result;
};

static char* read_variable_string() {
    int capacity = 10; // Start with a small buffer size
    int length = 0;
    char *buffer = malloc(capacity * sizeof(char));

    if (!buffer) return NULL;

    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
        if (length + 1 >= capacity) {
            capacity *= 2;
            char *new_buffer = realloc(buffer, capacity * sizeof(char));
            if (!new_buffer) {
                free(buffer); // Clean up if out of memory
                return NULL;
            }
            buffer = new_buffer;
        }
        buffer[length++] = ch;
    }

    buffer[length] = '\0'; // Null-terminate the string
    return buffer;
}

AST_T* builtin_function_input(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size > 2) {
        Iris_Throw(visitor, NULL, "function 'input'", "argument overflow. (%d to 2)", args_size);
    } else if (args_size < 2) {
        Iris_Throw(visitor, NULL, "function 'input'", "argument underflow. (2 to %d)", args_size);
    }

    AST_T* prompt = Visitor_Visit(visitor, args[0]);
    AST_T* input_message = Visitor_Visit(visitor, args[1]);

    if (prompt->type != AST_STRING) {
        Iris_Throw(visitor, prompt, "function 'input'", "type of argument 1 expects a string but did not receive one");
    };
    if (prompt == NULL || prompt->string_value == NULL) {
        Iris_Throw(visitor, prompt, "function 'input'", "argument 1 is null");
    };
    if (input_message->type != AST_BOOL) {
        Iris_Throw(visitor, input_message, "function 'input'", "type of argument 2 expects a bool but did not receive one");
    };

    printf("%s", prompt->string_value);
    if (input_message->bool_value == true) {
        printf("\nWaiting for input: ");
    };

    char* input = read_variable_string();
    if (!input){
        free(input); Iris_Throw(visitor, NULL, "function 'input'", "internal error");
    };

    AST_T* str = Init_AST(AST_STRING);
    str->string_value = input;

    return str;
};

// strings

AST_T* builtin_function_sub(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size > 3) {
        Iris_Throw(visitor, NULL, "function 'sub'", "argument overflow. (%d to 3)", args_size);
    } else if (args_size < 3) {
        Iris_Throw(visitor, NULL, "function 'sub'", "argument underflow. (3 to %d)", args_size);
    }

    AST_T* string = Visitor_Visit(visitor, args[0]);
    AST_T* start = Visitor_Visit(visitor, args[1]);
    AST_T* stop = Visitor_Visit(visitor, args[2]);

    if (string->type != AST_STRING) {
        Iris_Throw(visitor, string, "function 'sub'", "type of argument 1 expects a string but did not receive one");
    };
    if (string == NULL || string->string_value == NULL) {
        Iris_Throw(visitor, string, "function 'sub'", "argument 1 is null");
    };
    if (start->type != AST_INTEGER) {
        Iris_Throw(visitor, start, "function 'sub'", "type of argument 2 expects an integer but did not receive one");
    };
    if (stop->type != AST_INTEGER) {
        Iris_Throw(visitor, stop, "function 'sub'", "type of argument 3 expects an integer but did not receive one");
    };

    int chars = stop->integer_value - start->integer_value;
    if (chars == 0) {
        Iris_Throw(visitor, stop, "function 'sub'", "start and stop arguments are the same");
    }
    char* new = malloc(chars * sizeof(char) + 1);
    int v = 0;
    for (int x=start->integer_value;x<=stop->integer_value;x++){
        new[v] = string->string_value[x-1];
        v++;
    }
    new[v] = '\0';

    AST_T* new_string = Init_AST(AST_STRING);
    strcpy(new_string->string_value, new);

    return new_string;
}

static void dict_put(AST_T* d, const char* key, AST_T* value) {
    AST_T* k = Init_AST(AST_STRING);
    k->string_value = Iris_Strdup(key);
    k->scope = d->scope;

    d->dictionary_size += 1;
    d->dictionary_definition_value_name = realloc(
        d->dictionary_definition_value_name,
        d->dictionary_size * sizeof(struct AST_STRUCT*));
    d->dictionary_definition_value = realloc(
        d->dictionary_definition_value,
        d->dictionary_size * sizeof(struct AST_STRUCT*));
    d->dictionary_definition_value_name[d->dictionary_size - 1] = k;
    d->dictionary_definition_value[d->dictionary_size - 1] = value;
};

static AST_T* make_dict(Visitor_T* v, Scope_T* scope, const char* name) {
    AST_T* d = Init_AST(AST_DICTIONARY_DEFINITION);
    d->scope = scope;                        // must be set, see below
    d->dictionary_definition_name = Iris_Strdup(name);
    return d;
};

AST_T* builtin_function_split(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size > 2) {
        Iris_Throw(visitor, NULL, "function 'sub'", "argument overflow. (%d to 2)", args_size);
    } else if (args_size < 2) {
        Iris_Throw(visitor, NULL, "function 'sub'", "argument underflow. (2 to %d)", args_size);
    }

    AST_T* string = Visitor_Visit(visitor, args[0]);
    AST_T* separator = Visitor_Visit(visitor, args[1]);

    if (string->type != AST_STRING) {
        Iris_Throw(visitor, string, "function 'split'", "type of argument 1 expects a string but did not receive one");
    };
    if (string == NULL || string->string_value == NULL) {
        Iris_Throw(visitor, string, "function 'split'", "argument 1 is null");
    };
    if (separator->type != AST_STRING) {
        Iris_Throw(visitor, separator, "function 'split'", "type of argument 2 expects a string but did not receive one");
    };
    if (separator == NULL || separator->string_value == NULL) {
        Iris_Throw(visitor, separator, "function 'split'", "argument 2 is null");
    };
    if (strlen(separator->string_value) != 0) {
        Iris_Throw(visitor, separator, "function 'split'", "argument 2 is not a character");
    };

    AST_T* dict = make_dict(visitor, args[0]->scope, "IRIS_TUPLE_x13");

    int last = 0;
    int key = 0;

    for (int s=0;s<strlen(string->string_value);s++) {
        if (string->string_value[s] == separator->string_value[0]) {
            if (s == 0) {
                Iris_Throw(visitor, separator, "function 'split'", "starting character is separator");
            }

            int length = (key == 0) ? 1 : (int)log10(abs(key)) + 1;
            if (key < 0) length++;

            char n[length + 1];
            snprintf(n, sizeof(n), "%d", key);

            int substring_len = s - last;
            char* buffer = malloc(substring_len + 1);
            if (buffer == NULL) {
                printf("Ran out of memory, please close out of any high demanding memory applications or upgrade your RAM!\n");
                exit(1);
            }

            for (int o = 0; o < substring_len; o++) {
                buffer[o] = string->string_value[last + o];
            }
            buffer[substring_len] = '\0';

            const char* v = buffer;
            AST_T* str = Init_AST(AST_STRING);
            strcpy(str->string_value, v);
            str->scope = string->scope;
            dict_put(dict, n, str);
        }
    }

    return dict;
}
AST_T* builtin_function_join(Visitor_T* visitor, AST_T** args, int args_size){
    ;
}
AST_T* builtin_function_upper(Visitor_T* visitor, AST_T** args, int args_size){
    ;
}
AST_T* builtin_function_lower(Visitor_T* visitor, AST_T** args, int args_size){
    ;
}

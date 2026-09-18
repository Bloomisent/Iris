#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "window.h"

#include "scope.h"
#include "AST.h"
#include "visitor.h"

#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof((arr)[0]))

static char *removeSub(const char *str, const char *sub, char *new) {
    char *p = new;
    size_t len = strlen(sub);
    if (len > 0) {
        const char *match;
        while ((match = strstr(str, sub)) != NULL) {
            memcpy(p, str, match - str);
            p += match - str;
            str = match + len;
        }
    }
    strcpy(p, str);
    return new;
};

AST_T* builtin_function_print(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size <= 0) {
        printf("Tripped on function 'print', argument underflow. (0 to 1)\n");
        exit(1);
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
            case AST_BOOL: printf("%s", visited_ast->bool_value ? "true" : "false"); break;
            case AST_NUMBER : printf("%f ", visited_ast->number_value); break;
            case AST_CLASS : printf("%s ", visited_ast->class_name); break;
            case AST_CLASS_INSTANTIATION :
                //printf("{");
                //AST_T* class_def = Scope_Get_Class_Definition(visited_ast->scope, visited_ast->instance_class_name);
                printf(visited_ast->instance_class_name);
                break;
                /*for (size_t j = 0; j < class_def->class_size; j++) {
                    AST_T* elem = Visitor_Visit(visitor, class_def->class_definition_value[j]);
                    switch (elem->type) {
                        case AST_STRING : printf(" %s", elem->string_value); break;
                        case AST_NUMBER : printf(" %f", elem->number_value); break;
                        case AST_BOOL: printf(" %s", elem->bool_value ? "true" : "false"); break;
                        case AST_TABLE_DEFINITION : printf(" "); AST_T** new_args = calloc(1, sizeof(struct AST_STRUCT*)); new_args[0] = elem; builtin_function_print(visitor, new_args, 1); break;
                        default : printf(" %p", elem); break;
                    }
                    if (j < (visited_ast->table_size-1)){
                        printf(",");
                    };
                }
                printf("}");*/
            case AST_TABLE_DEFINITION : {
                printf("[");
                for (size_t j = 0; j < visited_ast->table_size; j++) {
                    AST_T* elem = Visitor_Visit(visitor, visited_ast->table_definition_value[j]);
                    switch (elem->type) {
                        case AST_STRING : printf(" %s", elem->string_value); break;
                        case AST_NUMBER : printf(" %f", elem->number_value); break;
                        case AST_BOOL: printf(" %s", elem->bool_value ? "true" : "false"); break;
                        case AST_TABLE_DEFINITION : printf(" "); AST_T** new_args = calloc(1, sizeof(struct AST_STRUCT*)); new_args[0] = elem; builtin_function_print(visitor, new_args, 1); break;
                        default : printf(" %p", elem); break;
                    }
                    if (j < (visited_ast->table_size-1)){
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

AST_T* builtin_function_type(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size > 1) {
        printf("Tripped on function 'type', argument overflow. (%d to 1)\n", args_size);
        exit(1);
    } else if (args_size < 1) {
        printf("Tripped on function 'type', argument underflow. (1 to %d)\n", args_size);
        exit(1);
    }

    AST_T* newvar = Init_AST(AST_NUMBER);

    if (args[0]->type == AST_TABLE) {
        newvar->type = AST_STRING;
        newvar->string_value = "TABLE";
        return newvar;
    };

    AST_T* var = Visitor_Visit(visitor, args[0]);

    switch (var->type) {
        case AST_STRING:
            newvar->type = AST_STRING;
            newvar->string_value = "STRING";
            return newvar;
            break;
        case AST_NUMBER:
            newvar->type = AST_STRING;
            newvar->string_value = "NUMBER";
            return newvar;
            break;
        case AST_TABLE:
            newvar->type = AST_STRING;
            newvar->string_value = "TABLE";
            return newvar;
            break;
        case AST_TABLE_DEFINITION:
            newvar->type = AST_STRING;
            newvar->string_value = "TABLE";
            return newvar;
            break;
        case AST_CLASS:
            newvar->type = AST_STRING;
            newvar->string_value = "CLASS";
            return newvar;
            break;
        case AST_CLASS_DEFINITION:
            newvar->type = AST_STRING;
            newvar->string_value = "CLASS";
            return newvar;
            break;
        case AST_CLASS_INSTANTIATION:
            newvar->type = AST_STRING;
            newvar->string_value = var->instance_class_name;
            return newvar;
            break;
        case AST_BOOL:
            newvar->type = AST_STRING;
            newvar->string_value = "BOOL";
            return newvar;
            break;
        default:
            printf("Tripped on function 'type', unsupported type %d\n", var->type);
            exit(1);
    };

    return Init_AST(AST_NOOP);
};

AST_T* builtin_function_dict_get_from_index(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size > 2) {
        printf("Tripped on function 'dict_get_from_index', argument overflow. (%d to 2)\n", args_size);
        exit(1);
    } else if (args_size < 2) {
        printf("Tripped on function 'dict_get_from_index', argument underflow. (2 to %d)\n", args_size);
        exit(1);
    }

    AST_T* dict = Visitor_Visit(visitor, args[0]);
    AST_T* index = Visitor_Visit(visitor, args[1]);

    if (dict->type != AST_DICTIONARY_DEFINITION) {
        printf("Tripped on function 'dict_get_from_index', type of argument 1 expects a dictionary but did not receive one\n");
        exit(1);
    };
    if (dict == NULL || dict->dictionary_definition_value == NULL || dict->dictionary_definition_value_name == NULL) {
        printf("Tripped on function 'dict_get_from_index', argument 1 is null\n");
        exit(1);
    };
    if (index->type != AST_STRING) {
        printf("Tripped on function 'dict_get_from_index', type of argument 2 expects a string but did not receive one\n");
        exit(1);
    };
    if (index->string_value == (void*)0){
        printf("Tripped on function 'dict_get_from_index', argument 2 is null\n");
        exit(1);
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
        printf("Tripped on function 'dict_get_index', argument overflow. (%d to 2)\n", args_size);
        exit(1);
    } else if (args_size < 2) {
        printf("Tripped on function 'dict_get_index', argument underflow. (2 to %d)\n", args_size);
        exit(1);
    }

    AST_T* dict = Visitor_Visit(visitor, args[0]);
    AST_T* value = Visitor_Visit(visitor, args[1]);

    if (dict == NULL || dict->type != AST_DICTIONARY_DEFINITION) {
        printf("Tripped on function 'dict_get_index', type of argument 1 expects a table but did not receive one\n");
        exit(1);
    }
    if (value == NULL || value->type == AST_NOOP) {
        printf("Tripped on function 'dict_get_index', type of argument 2 expects any value but did not receive one\n");
        exit(1);
    };

    AST_T* num = Init_AST(AST_NUMBER);

    for (int x=0;x<dict->dictionary_size;x++) {
        AST_T* d_val = Visitor_Visit(visitor, dict->dictionary_definition_value[x]);
        if (d_val->type != value->type) {
            num->number_value = -1;
        } else {
            switch (d_val->type) {
                case AST_STRING: {
                    if (strcmp(d_val->string_value, value->string_value) == 0) {
                        num->number_value = x;
                        break;
                    };
                };
                case AST_NUMBER: {
                    if (d_val->number_value == value->number_value) {
                        num->number_value = x;
                        break;
                    };
                };
                case AST_BOOL: {
                    if (d_val->bool_value == value->bool_value) {
                        num->number_value = x;
                        break;
                    };
                };
                default: num->number_value = -1; break;
            };
        };
    };

    return num;
};

AST_T* builtin_function_dict_set_index(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size > 3) {
        printf("Tripped on function 'dict_set_index', argument overflow. (%d to 3)\n", args_size);
        exit(1);
    } else if (args_size < 3) {
        printf("Tripped on function 'dict_set_index', argument underflow. (3 to %d)\n", args_size);
        exit(1);
    }

    AST_T* dict = Visitor_Visit(visitor, args[0]);
    AST_T* index = Visitor_Visit(visitor, args[1]);
    AST_T* value = Visitor_Visit(visitor, args[2]);

    if (dict == NULL || dict->type != AST_DICTIONARY_DEFINITION) {
        printf("Tripped on function 'dict_set_index', type of argument 1 expects a table but did not receive one\n");
        exit(1);
    };
    if (index == NULL || index->type != AST_STRING) {
        printf("Tripped on function 'dict_set_index', type of argument 2 expects a string but did not receive one\n");
        exit(1);
    }
    if (value == NULL || value->type == AST_NOOP) {
        printf("Tripped on function 'dict_set_index', type of argument 3 expects any value but did not receive one\n");
        exit(1);
    };

    for (int x;x<dict->dictionary_size;x++) {
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

AST_T* builtin_function_table_get_from_index(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size > 2) {
        printf("Tripped on function 'table_get_from_index', argument overflow. (%d to 2)\n", args_size);
        exit(1);
    } else if (args_size < 2) {
        printf("Tripped on function 'table_get_from_index', argument underflow. (2 to %d)\n", args_size);
        exit(1);
    }

    AST_T* table = Visitor_Visit(visitor, args[0]);
    AST_T* index = Visitor_Visit(visitor, args[1]);

    if (table->type != AST_TABLE_DEFINITION) {
        printf("Tripped on function 'table_get_from_index', type of argument 1 expects a table but did not receive one\n");
        exit(1);
    }
    if (index->type != AST_NUMBER) {
        printf("Tripped on function 'table_get_from_index', type of argument 2 expects a number but did not receive one\n");
        exit(1);
    };
    if ((size_t)index->number_value > table->table_size){
        printf("Tripped on function 'table_get_from_index', index is out of bounds (1)\n");
        exit(1);
    };
    if ((int)index->number_value <= 0){
        printf("Tripped on function 'table_get_from_index', index is out of bounds (0)\n");
        exit(1);
    };

    AST_T* elem = Visitor_Visit(visitor, table->table_definition_value[(int)index->number_value - 1]);
    AST_T* ast_var = Init_AST(AST_STRING);

    switch (elem->type) {
        case AST_STRING: ast_var->scope = table->scope; ast_var->string_value = elem->string_value; return ast_var; break;
        case AST_NUMBER: ast_var->type = AST_NUMBER; ast_var->scope = table->scope; ast_var->number_value = elem->number_value; return ast_var; break;
        case AST_BOOL: ast_var->type = AST_BOOL; ast_var->scope = table->scope; ast_var->bool_value = elem->bool_value; return ast_var; break;
        case AST_TABLE_DEFINITION : {
            ast_var->type = AST_TABLE_DEFINITION;
            ast_var->scope = table->scope;
            ast_var->table_definition_value = elem->table_definition_value;
            return ast_var;
            break;
        }
        default: printf("Tripped on function 'table_get_from_index', unsupported type %d\n", elem->type); exit(1);
    };

    return Init_AST(AST_NOOP);
};

AST_T* builtin_function_table_get_index(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size > 2) {
        printf("Tripped on function 'table_get_index', argument overflow. (%d to 2)\n", args_size);
        exit(1);
    } else if (args_size < 2) {
        printf("Tripped on function 'table_get_index', argument underflow. (2 to %d)\n", args_size);
        exit(1);
    }

    AST_T* table = Visitor_Visit(visitor, args[0]);
    AST_T* value = Visitor_Visit(visitor, args[1]);

    if (table == NULL || table->type != AST_TABLE_DEFINITION) {
        printf("Tripped on function 'table_get_index', type of argument 1 expects a table but did not receive one\n");
        exit(1);
    }
    if (value == NULL || value->type == AST_NOOP) {
        printf("Tripped on function 'table_get_index', type of argument 2 expects any value but did not receive one\n");
        exit(1);
    };

    AST_T* num = Init_AST(AST_NUMBER);

    for (int x;x<table->table_size;x++) {
        AST_T* t_val = Visitor_Visit(visitor, table->table_definition_value[x]);
        if (t_val->type != value->type) {
            num->number_value = -1;
        } else {
            switch (t_val->type) {
                case AST_TABLE_DEFINITION: {
                    if (t_val->table_definition_value == value->table_definition_value) {
                        num->number_value = x;
                        break;
                    };
                };
                case AST_STRING: {
                    if (strcmp(t_val->string_value, value->string_value) == 0) {
                        num->number_value = x;
                        break;
                    };
                };
                case AST_NUMBER: {
                    if (t_val->number_value == value->number_value) {
                        num->number_value = x;
                        break;
                    };
                };
                case AST_BOOL: {
                    if (t_val->bool_value == value->bool_value) {
                        num->number_value = x;
                        break;
                    };
                };
                default: num->number_value = -1; break;
            };
        };
    };

    return num;
};

AST_T* builtin_function_table_set_index(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size > 3) {
        printf("Tripped on function 'table_set_index', argument overflow. (%d to 3)\n", args_size);
        exit(1);
    } else if (args_size < 3) {
        printf("Tripped on function 'table_set_index', argument underflow. (3 to %d)\n", args_size);
        exit(1);
    }

    AST_T* table = Visitor_Visit(visitor, args[0]);
    AST_T* index = Visitor_Visit(visitor, args[1]);
    AST_T* value = Visitor_Visit(visitor, args[2]);

    if (table == NULL || table->type != AST_TABLE_DEFINITION) {
        printf("Tripped on function 'table_set_index', type of argument 1 expects a table but did not receive one\n");
        exit(1);
    };
    if (index== NULL || index->type != AST_NUMBER) {
        printf("Tripped on function 'table_set_index', type of argument 2 expects a number but did not receive one\n");
        exit(1);
    }
    if (value == NULL || value->type == AST_NOOP) {
        printf("Tripped on function 'table_set_index', type of argument 3 expects any value but did not receive one\n");
        exit(1);
    }
    if ((int)index->number_value <= 0){
        printf("Tripped on function 'table_set_index', index is out of bounds (0)\n");
        exit(1);
    };
    if ((size_t)index->number_value > table->table_size){
        size_t new_size = (size_t)index->number_value;
        table->table_definition_value = realloc(
            table->table_definition_value,
            new_size * sizeof(struct AST_STRUCT*)
        );
        table->table_size = new_size;
    };

    table->table_definition_value[(int)index->number_value - 1] = value;

    return Init_AST(AST_NOOP);
};

AST_T* builtin_function_char_at(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size > 2) {
        printf("Tripped on function 'charAt', argument overflow. (%d to 2)\n", args_size);
        exit(1);
    } else if (args_size < 2) {
        printf("Tripped on function 'charAt', argument underflow. (2 to %d)\n", args_size);
        exit(1);
    }

    AST_T* stringg = Visitor_Visit(visitor, args[0]);
    AST_T* index = Visitor_Visit(visitor, args[1]);

    if (index->type != AST_NUMBER) {
        printf("Tripped on function 'charAt', type of argument 2 expects a number but did not receive one\n");
        exit(1);
    }
    if (stringg->type != AST_STRING) {
        printf("Tripped on function 'charAt', type of argument 1 expects a string but did not receive one\n");
        exit(1);
    }

    AST_T* ast_var = Init_AST(AST_STRING);
    ast_var->scope = stringg->scope;
    ast_var->string_value = malloc(2);

    if (!ast_var->string_value) exit(EXIT_FAILURE);

    if ((int)index->number_value <= 0){
        printf("Failed on function 'charAt', index is out of bounds (0)\n");
        ast_var->string_value[0] = 'N';
        ast_var->string_value[1] = 'U';
        ast_var->string_value[2] = 'L';
        ast_var->string_value[3] = 'L';
        return ast_var;
    };
    if ((size_t)index->number_value > strlen(stringg->string_value)){
        printf("Failed on function 'charAt', index is out of bounds (1)\n");
        ast_var->string_value[0] = 'N';
        ast_var->string_value[1] = 'U';
        ast_var->string_value[2] = 'L';
        ast_var->string_value[3] = 'L'; // i have such a low sanity.
        return ast_var;
    };
    char c = stringg->string_value[(int)index->number_value-1];

    ast_var->string_value[0] = c;
    ast_var->string_value[1] = '\0';

    return ast_var;
};

AST_T* builtin_function_str_edit(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size > 3) {
        printf("Tripped on function 'str_edit', argument overflow. (%d to 3)\n", args_size);
        exit(1);
    } else if (args_size < 3) {
        printf("Tripped on function 'stredit', argument underflow. (3 to %d)\n", args_size);
        exit(1);
    }

    AST_T* str = Visitor_Visit(visitor, args[0]);
    AST_T* index = Visitor_Visit(visitor, args[1]);
    AST_T* value = Visitor_Visit(visitor, args[2]);

    if (str == NULL || str->type != AST_STRING) {
        printf("Tripped on function 'stredit', type of argument 1 expects a string but did not receive one\n");
        exit(1);
    }
    if (index == NULL || index->type == AST_NOOP) {
        printf("Tripped on function 'stredit', type of argument 2 expects a number but did not receive one\n");
        exit(1);
    }
    if ((int)index->number_value <= 0){
        printf("Tripped on function 'stredit', index is out of bounds (0)\n");
        exit(1);
    };
    if (value == NULL || value->type != AST_STRING) {
        printf("Tripped on function 'stredit', type of argument 1 expects a string but did not receive one\n");
        exit(1);
    };
    size_t length = strlen(value->string_value);

    if (length == 1) {
        char c = value->string_value[0];

        str->string_value[(int)index->number_value-1] = c;
    } else {
        printf("Failed on function 'stredit', string value is of a size greater/less than one.");
    };

    return Init_AST(AST_NOOP);
};

AST_T* builtin_function_for_each(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size < 2) {
        printf("Tripped on function 'ForEach', argument underflow. (2 to %d)\n", args_size);
        exit(1);
    }

    AST_T* str = Visitor_Visit(visitor, args[0]);
    AST_T* func_name = Visitor_Visit(visitor, args[1]);

    if (str == NULL || str->type != AST_STRING) {
        printf("Tripped on function 'ForEach', type of argument 1 expects a string but did not receive one\n");
        exit(1);
    };
    if (func_name == NULL || func_name->type != AST_STRING) {
        printf("Tripped on function 'ForEach', type of argument 2 expects a function name (string) but did not receive one\n");
        exit(1);
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
            printf("Tripped on undefined method '%s'\n", node->function_call_name);
            exit(1);
        }
        if (node->function_call_arguments_size != fdef->function_definition_args_size) {
            printf("Tripped on function call '%s', expected %zu args, got %zu\n",
                   node->function_call_name, fdef->function_definition_args_size, node->function_call_arguments_size);
            exit(1);
        }
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
        AST_T* result = Visitor_Visit(visitor, fdef->function_definition_body);
        visitor->returning = 0;

        call_scope->variable_definitions_size = saved_scope_size;
    };
    return Init_AST(AST_NOOP);
};

AST_T* builtin_function_to_number(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size > 1) {
        printf("Tripped on function 'toNumber', argument overflow. (%d to 1)\n", args_size);
        exit(1);
    } else if (args_size < 1) {
        printf("Tripped on function 'toNumber', argument underflow. (1 to %d)\n", args_size);
        exit(1);
    }

    AST_T* stringg = Visitor_Visit(visitor, args[0]);

    if (stringg == NULL || stringg->type != AST_STRING) {
        printf("Tripped on function 'toNumber', ineligible type for conversion (%d)\n", stringg->type);
        exit(1);
    };

    if (stringg->string_value == NULL) {
        printf("Tripped on function 'toNumber', got empty string\n");
        exit(1);
    }

    AST_T* num = Init_AST(AST_NUMBER);

    float numf = strtof(stringg->string_value, NULL);
    num->number_value=numf;

    return num;
};

AST_T* builtin_function_to_string(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size > 1) {
        printf("Tripped on function 'toString', argument overflow. (%d to 1)\n", args_size);
        exit(1);
    } else if (args_size < 1) {
        printf("Tripped on function 'toString', argument underflow. (1 to %d)\n", args_size);
        exit(1);
    }

    AST_T* num = Visitor_Visit(visitor, args[0]);

    if (num == NULL || num->type != AST_NUMBER) {
        printf("Tripped on function 'toString', ineligible type for conversion (%d)\n", num->type);
        exit(1);
    };

    AST_T* stringg = Init_AST(AST_STRING);
    stringg->string_value = calloc(1, sizeof(char));

    int len = snprintf(NULL, 0, "%f", num->number_value);

    char floats[len];
    snprintf(floats, sizeof(floats), "%f", num->number_value);
    strcpy(stringg->string_value, floats);

    return stringg;
};

AST_T* builtin_function_read_file(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size > 1) {
        printf("Tripped on function 'readFile', argument overflow. (%d to 1)\n", args_size);
        exit(1);
    } else if (args_size < 1) {
        printf("Tripped on function 'readFile', argument underflow. (1 to %d)\n", args_size);
        exit(1);
    }

    AST_T* fileName = Visitor_Visit(visitor, args[0]);

    if (fileName == NULL || fileName->type != AST_STRING) {
        printf("Tripped on function 'readFile', argument 1 expects a string but did not receive one\n");
        exit(1);
    };
    if (fileName->string_value == NULL || strlen(fileName->string_value) <= 0) {
        printf("Tripped on function 'readFile', argument 1 expects a string of size 1 or bigger\n");
        exit(1);
    };

    AST_T* str = Init_AST(AST_BOOL);
    str->string_value = calloc(1, sizeof(char));

    int len = strlen(str->string_value);

    // do something here
    FILE* file = fopen(fileName->string_value, "r");

    char buffer[256];

    if (file==NULL){
        printf("Failed to open file (%s)", str->string_value);
        exit(1);
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
        printf("Tripped on function 'writeFile', argument overflow. (%d to 3)\n", args_size);
        exit(1);
    } else if (args_size < 3) {
        printf("Tripped on function 'writeFile', argument underflow. (3 to %d)\n", args_size);
        exit(1);
    }

    AST_T* fileName = Visitor_Visit(visitor, args[0]);
    AST_T* contents = Visitor_Visit(visitor, args[1]);
    AST_T* formatted = Visitor_Visit(visitor, args[2]);

    if (fileName == NULL || fileName->type != AST_STRING) {
        printf("Tripped on function 'writeFile', argument 1 expects a string but did not receive one\n");
        exit(1);
    };
    if (fileName->string_value == NULL || strlen(fileName->string_value) <= 0) {
        printf("Tripped on function 'writeFile', argument 1 expects a string of size 1 or bigger\n");
        exit(1);
    };
    if (contents == NULL || contents->type != AST_STRING) {
        printf("Tripped on function 'writeFile', argument 2 expects a string but did not receive one\n");
        exit(1);
    };
    if (contents->string_value == NULL || strlen(contents->string_value) <= 0) {
        printf("Tripped on function 'writeFile', argument 2 expects a string of size 1 or bigger\n");
        exit(1);
    };
    if (formatted == NULL || formatted->type != AST_BOOL) {
        printf("Tripped on function 'writeFile', argument 3 expects a bool but did not receive one\n");
        exit(1);
    };

    AST_T* success = Init_AST(AST_BOOL);
    success->bool_value = false;

    FILE* file = fopen(fileName->string_value, "w");

    if (file == NULL) {
        success->bool_value = false;
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

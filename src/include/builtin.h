#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "window.h"

#include "scope.h"
#include "AST.h"
#include "visitor.h"

#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof((arr)[0]))

extern int current_line;

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
        printf("Tripped on function 'print', argument underflow. (0 to 1), at line %d\n", current_line);
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
            case AST_NUMBER : printf("%g ", visited_ast->number_value); break;
            case AST_CLASS : printf("%s ", visited_ast->class_name); break;
            case AST_CLASS_INSTANTIATION :
                printf("%s ", visited_ast->instance_class_name);
                break;
            case AST_TABLE_DEFINITION : {
                printf("[");
                for (size_t j = 0; j < visited_ast->table_size; j++) {
                    AST_T* elem = Visitor_Visit(visitor, visited_ast->table_definition_value[j]);
                    switch (elem->type) {
                        case AST_STRING : printf(" %s", elem->string_value); break;
                        case AST_NUMBER : printf(" %g", elem->number_value); break;
                        case AST_BOOL: printf(" %s", elem->bool_value ? "true" : "false"); break;
                        case AST_CLASS : printf(" %s ", elem->class_name); break;
                        case AST_CLASS_INSTANTIATION :
                            printf(" %s", elem->instance_class_name);
                            break;
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
            case AST_DICTIONARY_DEFINITION : {
                printf("[");
                for (size_t j = 0; j < visited_ast->dictionary_size; j++) {
                    AST_T* name = Visitor_Visit(visitor, visited_ast->dictionary_definition_value_name[j]);
                    AST_T* elem = Visitor_Visit(visitor, visited_ast->dictionary_definition_value[j]);
                    switch (elem->type) {
                        case AST_STRING : printf(" '%s':%s", name->string_value, elem->string_value); break;
                        case AST_NUMBER : printf(" '%s':%g", name->string_value, elem->number_value); break;
                        case AST_BOOL: printf(" '%s':%s", elem->bool_value ? "true" : "false", name->string_value); break;
                        case AST_CLASS : printf(" '%s':%s ", elem->class_name, name->string_value); break;
                        case AST_CLASS_INSTANTIATION :
                            printf(" '%s':%s", name->string_value, elem->instance_class_name);
                            break;
                        case AST_TABLE_DEFINITION : printf(" '%s':", name->string_value); AST_T** new_args = calloc(1, sizeof(struct AST_STRUCT*)); new_args[0] = elem; builtin_function_print(visitor, new_args, 1); break;
                        case AST_DICTIONARY_DEFINITION : printf(" '%s':", name->string_value); AST_T** new_args2 = calloc(1, sizeof(struct AST_STRUCT*)); new_args2[0] = elem; builtin_function_print(visitor, new_args2, 1); break;
                        default : printf(" '%s':%p", name->string_value, elem); break;
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

AST_T* builtin_function_type(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size > 1) {
        printf("Tripped on function 'type', argument overflow. (%d to 1), at line %d\n", args_size, current_line);
        exit(1);
    } else if (args_size < 1) {
        printf("Tripped on function 'type', argument underflow. (1 to %d), at line %d\n", args_size, current_line);
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
            printf("Tripped on function 'type', unsupported type (at line %d)\n", current_line);
            exit(1);
    };

    return Init_AST(AST_NOOP);
};

AST_T* builtin_function_dict_get_from_index(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size > 2) {
        printf("Tripped on function 'dict_get_from_index', argument overflow. (%d to 2), at line %d\n", args_size, current_line);
        exit(1);
    } else if (args_size < 2) {
        printf("Tripped on function 'dict_get_from_index', argument underflow. (2 to %d), at line %d\n", args_size, current_line);
        exit(1);
    }

    AST_T* dict = Visitor_Visit(visitor, args[0]);
    AST_T* index = Visitor_Visit(visitor, args[1]);

    if (dict->type != AST_DICTIONARY_DEFINITION) {
        printf("Tripped on function 'dict_get_from_index', type of argument 1 expects a dictionary but did not receive one (at line %d)\n", current_line);
        exit(1);
    };
    if (dict == NULL || dict->dictionary_definition_value == NULL || dict->dictionary_definition_value_name == NULL) {
        printf("Tripped on function 'dict_get_from_index', argument 1 is null (at line %d)\n", current_line);
        exit(1);
    };
    if (index->type != AST_STRING) {
        printf("Tripped on function 'dict_get_from_index', type of argument 2 expects a string but did not receive one (at line %d)\n", current_line);
        exit(1);
    };
    if (index->string_value == (void*)0){
        printf("Tripped on function 'dict_get_from_index', argument 2 is null (at line %d)\n", current_line);
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
        printf("Tripped on function 'dict_get_index', argument overflow. (%d to 2), at line %d\n", args_size, current_line);
        exit(1);
    } else if (args_size < 2) {
        printf("Tripped on function 'dict_get_index', argument underflow. (2 to %d), at line %d\n", args_size, current_line);
        exit(1);
    }

    AST_T* dict = Visitor_Visit(visitor, args[0]);
    AST_T* value = Visitor_Visit(visitor, args[1]);

    if (dict == NULL || dict->type != AST_DICTIONARY_DEFINITION) {
        printf("Tripped on function 'dict_get_index', type of argument 1 expects a table but did not receive one (at line %d)\n", current_line);
        exit(1);
    }
    if (value == NULL || value->type == AST_NOOP) {
        printf("Tripped on function 'dict_get_index', type of argument 2 expects any value but did not receive one (at line %d)\n", current_line);
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
        printf("Tripped on function 'dict_set_index', argument overflow. (%d to 3), at line %d\n", args_size, current_line);
        exit(1);
    } else if (args_size < 3) {
        printf("Tripped on function 'dict_set_index', argument underflow. (3 to %d), at line %d\n", args_size, current_line);
        exit(1);
    }

    AST_T* dict = Visitor_Visit(visitor, args[0]);
    AST_T* index = Visitor_Visit(visitor, args[1]);
    AST_T* value = Visitor_Visit(visitor, args[2]);

    if (dict == NULL || dict->type != AST_DICTIONARY_DEFINITION) {
        printf("Tripped on function 'dict_set_index', type of argument 1 expects a table but did not receive one (at line %d)\n", current_line);
        exit(1);
    };
    if (index == NULL || index->type != AST_STRING) {
        printf("Tripped on function 'dict_set_index', type of argument 2 expects a string but did not receive one (at line %d)\n", current_line);
        exit(1);
    }
    if (value == NULL || value->type == AST_NOOP) {
        printf("Tripped on function 'dict_set_index', type of argument 3 expects any value but did not receive one (at line %d)\n", current_line);
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
        printf("Tripped on function 'table_get_from_index', argument overflow. (%d to 2), at line %d\n", args_size, current_line);
        exit(1);
    } else if (args_size < 2) {
        printf("Tripped on function 'table_get_from_index', argument underflow. (2 to %d), at line %d\n", args_size, current_line);
        exit(1);
    }

    AST_T* table = Visitor_Visit(visitor, args[0]);
    AST_T* index = Visitor_Visit(visitor, args[1]);

    if (table->type != AST_TABLE_DEFINITION) {
        printf("Tripped on function 'table_get_from_index', type of argument 1 expects a table but did not receive one (at line %d)\n", current_line);
        exit(1);
    }
    if (index->type != AST_NUMBER) {
        printf("Tripped on function 'table_get_from_index', type of argument 2 expects a number but did not receive one (at line %d)\n", current_line);
        exit(1);
    };
    if ((int)index->number_value <= 0){
        printf("Tripped on function 'table_get_from_index', index is out of bounds (%g, smaller than 0), at line %d\n", index->number_value, current_line);
        exit(1);
    };
    if ((size_t)index->number_value-1 > table->table_size){
        printf("Tripped on function 'table_get_from_index', index is out of bounds (%g, larger than table size of %zu), at line %d\n", index->number_value, table->table_size, current_line);
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
        printf("Tripped on function 'table_get_index', argument overflow. (%d to 2), at line %d\n", args_size, current_line);
        exit(1);
    } else if (args_size < 2) {
        printf("Tripped on function 'table_get_index', argument underflow. (2 to %d), at line %d\n", args_size, current_line);
        exit(1);
    }

    AST_T* table = Visitor_Visit(visitor, args[0]);
    AST_T* value = Visitor_Visit(visitor, args[1]);

    if (table == NULL || table->type != AST_TABLE_DEFINITION) {
        printf("Tripped on function 'table_get_index', type of argument 1 expects a table but did not receive one (at line %d)\n", current_line);
        exit(1);
    }
    if (value == NULL || value->type == AST_NOOP) {
        printf("Tripped on function 'table_get_index', type of argument 2 expects any value but did not receive one (at line %d)\n", current_line);
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
        printf("Tripped on function 'table_set_index', argument overflow. (%d to 3), at line %d\n", args_size, current_line);
        exit(1);
    } else if (args_size < 3) {
        printf("Tripped on function 'table_set_index', argument underflow. (3 to %d), at line %d\n", args_size, current_line);
        exit(1);
    }

    AST_T* table = Visitor_Visit(visitor, args[0]);
    AST_T* index = Visitor_Visit(visitor, args[1]);
    AST_T* value = Visitor_Visit(visitor, args[2]);

    if (table == NULL || table->type != AST_TABLE_DEFINITION) {
        printf("Tripped on function 'table_set_index', type of argument 1 expects a table but did not receive one (at line %d)\n", current_line);
        exit(1);
    };
    if (index== NULL || index->type != AST_NUMBER) {
        printf("Tripped on function 'table_set_index', type of argument 2 expects a number but did not receive one (at line %d)\n", current_line);
        exit(1);
    }
    if (value == NULL || value->type == AST_NOOP) {
        printf("Tripped on function 'table_set_index', type of argument 3 expects any value but did not receive one (at line %d)\n", current_line);
        exit(1);
    }
    if ((int)index->number_value <= 0){
        printf("Tripped on function 'table_set_index', index is out of bounds (0), at line %d\n", current_line);
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
        printf("Tripped on function 'charAt', argument overflow. (%d to 2), at line %d\n", args_size, current_line);
        exit(1);
    } else if (args_size < 2) {
        printf("Tripped on function 'charAt', argument underflow. (2 to %d), at line %d\n", args_size, current_line);
        exit(1);
    }

    AST_T* stringg = Visitor_Visit(visitor, args[0]);
    AST_T* index = Visitor_Visit(visitor, args[1]);

    if (index->type != AST_NUMBER) {
        printf("Tripped on function 'charAt', type of argument 2 expects a number but did not receive one (at line %d)\n", current_line);
        exit(1);
    }
    if (stringg->type != AST_STRING) {
        printf("Tripped on function 'charAt', type of argument 1 expects a string but did not receive one (at line %d)\n", current_line);
        exit(1);
    }

    AST_T* ast_var = Init_AST(AST_STRING);
    ast_var->scope = stringg->scope;
    ast_var->string_value = malloc(2);

    if (!ast_var->string_value) exit(EXIT_FAILURE);

    if ((int)index->number_value <= 0){
        printf("Failed on function 'charAt', index is out of bounds (0), at line %d\n", current_line);
        ast_var->string_value[0] = 'N';
        ast_var->string_value[1] = 'U';
        ast_var->string_value[2] = 'L';
        ast_var->string_value[3] = 'L';
        return ast_var;
    };
    if ((size_t)index->number_value > strlen(stringg->string_value)){
        printf("Failed on function 'charAt', index is out of bounds (1), at line %d\n", current_line);
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
        printf("Tripped on function 'str_edit', argument overflow. (%d to 3), at line %d\n", args_size, current_line);
        exit(1);
    } else if (args_size < 3) {
        printf("Tripped on function 'stredit', argument underflow. (3 to %d), at line %d\n", args_size, current_line);
        exit(1);
    }

    AST_T* str = Visitor_Visit(visitor, args[0]);
    AST_T* index = Visitor_Visit(visitor, args[1]);
    AST_T* value = Visitor_Visit(visitor, args[2]);

    if (str == NULL || str->type != AST_STRING) {
        printf("Tripped on function 'stredit', type of argument 1 expects a string but did not receive one (at line %d)\n", current_line);
        exit(1);
    }
    if (index == NULL || index->type == AST_NOOP) {
        printf("Tripped on function 'stredit', type of argument 2 expects a number but did not receive one (at line %d)\n", current_line);
        exit(1);
    }
    if ((int)index->number_value <= 0){
        printf("Tripped on function 'stredit', index is out of bounds (0)\n");
        exit(1);
    };
    if (value == NULL || value->type != AST_STRING) {
        printf("Tripped on function 'stredit', type of argument 1 expects a string but did not receive one (at line %d)\n", current_line);
        exit(1);
    };
    size_t length = strlen(value->string_value);

    if (length == 1) {
        char c = value->string_value[0];

        str->string_value[(int)index->number_value-1] = c;
    } else {
        printf("Failed on function 'stredit', string value is of a size greater/less than one (at line %d)\n", current_line);
    };

    return Init_AST(AST_NOOP);
};

AST_T* builtin_function_for_each(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size < 2) {
        printf("Tripped on function 'ForEach', argument underflow (2 to %d), at line %d\n", args_size, current_line);
        exit(1);
    }

    AST_T* str = Visitor_Visit(visitor, args[0]);
    AST_T* func_name = Visitor_Visit(visitor, args[1]);

    if (str == NULL || str->type != AST_STRING) {
        printf("Tripped on function 'ForEach', type of argument 1 expects a string but did not receive one (at line %d)\n", current_line);
        exit(1);
    };
    if (func_name == NULL || func_name->type != AST_STRING) {
        printf("Tripped on function 'ForEach', type of argument 2 expects a function name (string) but did not receive one (at line %d)\n", current_line);
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
            printf("Tripped on undefined method '%s' (at line %d)\n", node->function_call_name, current_line);
            exit(1);
        }
        if (node->function_call_arguments_size != fdef->function_definition_args_size) {
            printf("Tripped on function call '%s', expected %zu args, got %zu (at line %d)\n",
                   node->function_call_name, fdef->function_definition_args_size, node->function_call_arguments_size, current_line);
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
        Visitor_Visit(visitor, fdef->function_definition_body);
        visitor->returning = 0;

        call_scope->variable_definitions_size = saved_scope_size;
    };
    return Init_AST(AST_NOOP);
};

AST_T* builtin_function_to_number(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size > 1) {
        printf("Tripped on function 'toNumber', argument overflow. (%d to 1), at line %d\n", args_size, current_line);
        exit(1);
    } else if (args_size < 1) {
        printf("Tripped on function 'toNumber', argument underflow. (1 to %d), at line %d\n", args_size, current_line);
        exit(1);
    }

    AST_T* stringg = Visitor_Visit(visitor, args[0]);

    if (stringg == NULL || stringg->type != AST_STRING) {
        printf("Tripped on function 'toNumber', ineligible type for conversion (%d), at line %d\n", stringg->type, current_line);
        exit(1);
    };

    if (stringg->string_value == NULL) {
        printf("Tripped on function 'toNumber', got empty string (at line %d)\n", current_line);
        exit(1);
    }

    AST_T* num = Init_AST(AST_NUMBER);

    float numf = strtof(stringg->string_value, NULL);
    num->number_value=numf;

    return num;
};

AST_T* builtin_function_to_string(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size > 1) {
        printf("Tripped on function 'toString', argument overflow. (%d to 1), at line %d\n", args_size, current_line);
        exit(1);
    } else if (args_size < 1) {
        printf("Tripped on function 'toString', argument underflow. (1 to %d), at line %d\n", args_size, current_line);
        exit(1);
    }

    AST_T* num = Visitor_Visit(visitor, args[0]);

    if (num == NULL || num->type != AST_NUMBER) {
        printf("Tripped on function 'toString', ineligible type for conversion (%d), at line %d\n", num->type, current_line);
        exit(1);
    };

    int len = snprintf(NULL, 0, "%f", num->number_value) + 1;

    AST_T* stringg = Init_AST(AST_STRING);
    stringg->string_value = calloc(len, sizeof(char));
    if (!stringg->string_value) {
        printf("Failed on function 'toString', memory allocation failed (at line %d)\n", current_line);
        exit(1);
    };

    snprintf(stringg->string_value, len, "%f", num->number_value);

    return stringg;
};

AST_T* builtin_function_floor(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size > 2) {
        printf("Tripped on function 'floor', argument overflow. (%d to 2), at line %d\n", args_size, current_line);
        exit(1);
    } else if (args_size < 2) {
        printf("Tripped on function 'floor', argument underflow. (2 to %d), at line %d\n", args_size, current_line);
        exit(1);
    }

    AST_T* num = Visitor_Visit(visitor, args[0]);
    AST_T* floor_to = Visitor_Visit(visitor, args[1]);

    if (num == NULL || num->type != AST_NUMBER) {
        printf("Tripped on function 'floor', ineligible type for conversion (%d), at line %d\n", num->type, current_line);
        exit(1);
    };
    if (floor_to == NULL || floor_to->type != AST_NUMBER) {
        printf("Tripped on function 'floor', ineligible type for conversion (%d), at line %d\n", floor_to->type, current_line);
        exit(1);
    };

    AST_T* rounded = Init_AST(AST_NUMBER);
    rounded->number_value=floor(num->number_value/floor_to->number_value) * floor_to->number_value;

    return rounded;
};

AST_T* builtin_function_read_file(Visitor_T* visitor, AST_T** args, int args_size){
    if (args_size > 1) {
        printf("Tripped on function 'readFile', argument overflow. (%d to 1), at line %d\n", args_size, current_line);
        exit(1);
    } else if (args_size < 1) {
        printf("Tripped on function 'readFile', argument underflow. (1 to %d), at line %d\n", args_size, current_line);
        exit(1);
    }

    AST_T* fileName = Visitor_Visit(visitor, args[0]);

    if (fileName == NULL || fileName->type != AST_STRING) {
        printf("Tripped on function 'readFile', argument 1 expects a string but did not receive one (at line %d)\n", current_line);
        exit(1);
    };
    if (fileName->string_value == NULL || strlen(fileName->string_value) <= 0) {
        printf("Tripped on function 'readFile', argument 1 expects a string of size 1 or bigger (at line %d)\n", current_line);
        exit(1);
    };

    AST_T* str = Init_AST(AST_STRING);
    str->string_value = calloc(1, sizeof(char));

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
        printf("Tripped on function 'writeFile', argument overflow. (%d to 3), at line %d\n", args_size, current_line);
        exit(1);
    } else if (args_size < 3) {
        printf("Tripped on function 'writeFile', argument underflow. (3 to %d), at line %d\n", args_size, current_line);
        exit(1);
    }

    AST_T* fileName = Visitor_Visit(visitor, args[0]);
    AST_T* contents = Visitor_Visit(visitor, args[1]);
    AST_T* formatted = Visitor_Visit(visitor, args[2]);

    if (fileName == NULL || fileName->type != AST_STRING) {
        printf("Tripped on function 'writeFile', argument 1 expects a string but did not receive one (at line %d)\n", current_line);
        exit(1);
    };
    if (fileName->string_value == NULL || strlen(fileName->string_value) <= 0) {
        printf("Tripped on function 'writeFile', argument 1 expects a string of size 1 or bigger (at line %d)\n", current_line);
        exit(1);
    };
    if (contents == NULL || contents->type != AST_STRING) {
        printf("Tripped on function 'writeFile', argument 2 expects a string but did not receive one (at line %d)\n", current_line);
        exit(1);
    };
    if (contents->string_value == NULL || strlen(contents->string_value) <= 0) {
        printf("Tripped on function 'writeFile', argument 2 expects a string of size 1 or bigger (at line %d)\n", current_line);
        exit(1);
    };
    if (formatted == NULL || formatted->type != AST_BOOL) {
        printf("Tripped on function 'writeFile', argument 3 expects a bool but did not receive one (at line %d)\n", current_line);
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
        printf("Tripped on function 'gc', expected 0 arguments, got %d (at line %d)\n", args_size, current_line);
        exit(1);
    }
    if (visitor->call_depth > 0) {
        printf("Tripped on function 'gc', cannot collect while a function call is in progress (call it from the top level of your script, not from inside a function) (at line %d)\n", current_line);
        exit(1);
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
        printf("Tripped on function 'input', argument overflow. (%d to 2), at line %d\n", args_size, current_line);
        exit(1);
    } else if (args_size < 2) {
        printf("Tripped on function 'input', argument underflow. (2 to %d), at line %d\n", args_size, current_line);
        exit(1);
    }

    AST_T* prompt = Visitor_Visit(visitor, args[0]);
    AST_T* input_message = Visitor_Visit(visitor, args[1]);

    if (prompt->type != AST_STRING) {
        printf("Tripped on function 'input', type of argument 1 expects a string but did not receive one (at line %d)\n", current_line);
        exit(1);
    };
    if (input_message->type != AST_BOOL) {
        printf("Tripped on function 'input', type of argument 2 expects a bool but did not receive one (at line %d)\n", current_line);
        exit(1);
    };
    if (prompt == NULL || prompt->string_value == NULL) {
        printf("Tripped on function 'input', argument 1 is null (at line %d)\n", current_line);
        exit(1);
    };

    printf("%s", prompt->string_value);
    if (input_message->bool_value == true) {
        printf("\nWaiting for input: ");
    };

    char* input = read_variable_string();
    if (!input){
        printf("Failed on function 'input', internal error\n", input);
        free(input);
        exit(1);
    };

    AST_T* str = Init_AST(AST_STRING);
    str->string_value = input;

    free(input);

    return str;
};

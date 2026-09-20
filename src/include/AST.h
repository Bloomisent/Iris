#pragma once

#include <stddef.h>
#include <stdbool.h>

typedef enum {
    AST_VARIABLE_DEFINITION,
    AST_TABLE_DEFINITION,
    AST_DICTIONARY_DEFINITION,
    AST_CLASS_DEFINITION,
    AST_FUNCTION_DEFINITION,
    AST_IF,
    AST_IF_ELSE,
    AST_FOR,
    AST_WHILE,
    AST_BINOP,
    AST_DOT,
    AST_VARIABLE,
    AST_TABLE,
    AST_DICTIONARY,
    AST_CLASS,
    AST_FUNCTION_CALL,
    AST_STRING,
    AST_NUMBER,
    AST_BOOL,
    AST_RETURN,
    AST_COMPOUND,
    AST_ASSIGNMENT,
    AST_CHECKS,
    AST_CLASS_INSTANTIATION,
    AST_INIT_CALL,
    AST_NOOP
} AST_Type;

typedef struct AST_STRUCT {

    int current_line;

    AST_Type type;
    struct SCOPE_STRUCT* scope;

    char* variable_definition_variable_name;
    struct AST_STRUCT* variable_definition_value;
    char* variable_name;

    struct AST_STRUCT* function_definition_body;
    char* function_definition_name;

    struct AST_STRUCT** function_definition_args;
    size_t function_definition_args_size;

    char* function_call_name;

    struct AST_STRUCT** function_call_arguments;
    size_t function_call_arguments_size;

    struct AST_STRUCT* if_condition;
    struct AST_STRUCT* if_body;
    struct AST_STRUCT* if_else_body;

    struct AST_STRUCT* checks_base_var; //var to be checked
    struct AST_STRUCT** checks_condition_body;
    size_t checks_condition_size;
    struct AST_STRUCT** checks_do_body;

    struct AST_STRUCT* for_variable;
    struct AST_STRUCT* for_condition;
    struct AST_STRUCT* for_does_at_end;
    struct AST_STRUCT* for_body;

    struct AST_STRUCT* while_condition;
    struct AST_STRUCT* while_body;

    struct AST_STRUCT* binop_left;
    struct AST_STRUCT* binop_right;
    int binop_op;

    struct AST_STRUCT* dot_left;
    struct AST_STRUCT* dot_right;

    struct AST_STRUCT* arrow_left;
    struct AST_STRUCT* arrow_right;

    struct AST_STRUCT* assignment_target;
    struct AST_STRUCT* assignment_value;

    char* instance_class_name;
    char* instance_variable_name;

    struct AST_STRUCT* init_call_instance;

    struct AST_STRUCT* return_value;

    char* string_value;
    float number_value;
    bool bool_value;

    char* table_definition_name;

    struct AST_STRUCT** table_definition_value;
    struct AST_STRUCT* table_value;

    size_t table_size;
    char* table_name;

    char* dictionary_definition_name;

    struct AST_STRUCT** dictionary_definition_value;
    struct AST_STRUCT** dictionary_definition_value_name;

    struct AST_STRUCT* dictionary_value;
    size_t dictionary_size;
    char* dictionary_name;

    char* class_definition_name;

    struct AST_STRUCT** class_definition_value;
    struct AST_STRUCT** class_definition_value_name;

    struct AST_STRUCT* class_value;
    size_t class_size;

    struct AST_STRUCT* init_value;

    struct AST_STRUCT** init_args;
    size_t init_args_size;

    int init_step;

    char* class_name;

    struct AST_STRUCT** compound_value;
    size_t compound_size;

} AST_T;

AST_T* Init_AST(int type);
char* Iris_Strdup(const char* s);

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

#define RED   "\033[0;31m"
#define RESET "\033[0m"

#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof((arr)[0]))

AST_T* builtin_function_print(Visitor_T* visitor, AST_T** args, int args_size);
AST_T* builtin_function_err(Visitor_T* visitor, AST_T** args, int args_size);
AST_T* builtin_function_exit(Visitor_T* visitor, AST_T** args, int args_size);
AST_T* builtin_function_type(Visitor_T* visitor, AST_T** args, int args_size);
AST_T* builtin_function_dict_get_from_index(Visitor_T* visitor, AST_T** args, int args_size);
AST_T* builtin_function_dict_get_index(Visitor_T* visitor, AST_T** args, int args_size);
AST_T* builtin_function_dict_set_index(Visitor_T* visitor, AST_T** args, int args_size);
AST_T* builtin_function_table_get_from_index(Visitor_T* visitor, AST_T** args, int args_size);
AST_T* builtin_function_table_get_index(Visitor_T* visitor, AST_T** args, int args_size);
AST_T* builtin_function_table_set_index(Visitor_T* visitor, AST_T** args, int args_size);
AST_T* builtin_function_char_at(Visitor_T* visitor, AST_T** args, int args_size);
AST_T* builtin_function_str_edit(Visitor_T* visitor, AST_T** args, int args_size);
AST_T* builtin_function_for_each(Visitor_T* visitor, AST_T** args, int args_size);
AST_T* builtin_function_to_number(Visitor_T* visitor, AST_T** args, int args_size);
AST_T* builtin_function_to_string(Visitor_T* visitor, AST_T** args, int args_size);
AST_T* builtin_function_floor(Visitor_T* visitor, AST_T** args, int args_size);
AST_T* builtin_function_read_file(Visitor_T* visitor, AST_T** args, int args_size);
AST_T* builtin_function_write_file(Visitor_T* visitor, AST_T** args, int args_size);
AST_T* builtin_function_gc(Visitor_T* visitor, AST_T** args, int args_size);
AST_T* builtin_function_gc_stats(Visitor_T* visitor, AST_T** args, int args_size);
AST_T* builtin_function_input(Visitor_T* visitor, AST_T** args, int args_size);

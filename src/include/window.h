#pragma once

#include "AST.h"
#include "visitor.h"

AST_T* builtin_function_window_create(Visitor_T* visitor, AST_T** args, int args_size);
AST_T* builtin_function_window_get_time(Visitor_T* visitor, AST_T** args, int args_size);
AST_T* builtin_function_window_should_close(Visitor_T* visitor, AST_T** args, int args_size);
AST_T* builtin_function_window_clear(Visitor_T* visitor, AST_T** args, int args_size);
AST_T* builtin_function_window_draw_rect(Visitor_T* visitor, AST_T** args, int args_size);
AST_T* builtin_function_window_draw_text(Visitor_T* visitor, AST_T** args, int args_size);
AST_T* builtin_function_window_present(Visitor_T* visitor, AST_T** args, int args_size);
AST_T* builtin_function_window_mouse_x(Visitor_T* visitor, AST_T** args, int args_size);
AST_T* builtin_function_window_mouse_y(Visitor_T* visitor, AST_T** args, int args_size);
AST_T* builtin_function_window_mouse_pressed(Visitor_T* visitor, AST_T** args, int args_size);
AST_T* builtin_function_window_close(Visitor_T* visitor, AST_T** args, int args_size);

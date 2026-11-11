#pragma once

#include <setjmp.h>
#include "AST.h"

#define MAX_TRY_DEPTH 64

typedef struct {
    jmp_buf buf;
    size_t  scope_size;   // flat scope size at try entry
    int     call_depth;
} Try_Frame;

typedef struct VISITOR_STRUCT {
    bool breaking;
    int returning;
    int call_depth;

    Try_Frame frames[MAX_TRY_DEPTH];
    int frame_count;
    char error_kind[64];
    char error_msg[512];
    int error_line;
} Visitor_T;

typedef struct FOR_TUPLE_STRUCT {
    AST_T* condition_result;
    AST_T* for_var;
    AST_T* for_var2;
    AST_T* for_end_do;
} For_Tuple;

typedef struct WHILE_TUPLE_STRUCT {
    AST_T* condition_result;
} While_Tuple;

For_Tuple* get_for_data(Visitor_T* visitor, AST_T* node);
While_Tuple* get_while_data(Visitor_T* visitor, AST_T* node);

Visitor_T* Init_Visitor();

For_Tuple* Init_For_Tuple();
While_Tuple* Init_While_Tuple();

void Iris_Throw(Visitor_T* v, AST_T* at, const char* kind, const char* fmt, ...);

AST_T* Visitor_Visit(Visitor_T* visitor, AST_T* node);
AST_T* Copy_Value(Visitor_T* v, AST_T* x);
AST_T* VV_Variable_Definition(Visitor_T* visitor, AST_T* node);
AST_T* VV_Variable(Visitor_T* visitor, AST_T* node);
AST_T* VV_Enum(Visitor_T* visitor, AST_T* node);
AST_T* VV_Ternary(Visitor_T* visitor, AST_T* node);
AST_T* VV_If(Visitor_T* visitor, AST_T* node);
AST_T* VV_Checks(Visitor_T* visitor, AST_T* node);
AST_T* VV_For(Visitor_T* visitor, AST_T* node);
AST_T* VV_While(Visitor_T* visitor, AST_T* node);
AST_T* VV_BinOp(Visitor_T* visitor, AST_T* node);
AST_T* VV_Unary_Not(Visitor_T* visitor, AST_T* node);
AST_T* VV_Arrow(Visitor_T* visitor, AST_T* node);
AST_T* VV_Dot(Visitor_T* visitor, AST_T* node);
AST_T* VV_Break(Visitor_T* visitor, AST_T* node);
AST_T* VV_Return(Visitor_T* visitor, AST_T* node);
AST_T* VV_Assignment(Visitor_T* visitor, AST_T* node);
AST_T* VV_Class_Instantiation(Visitor_T* visitor, AST_T* node);
AST_T* VV_Init_Call(Visitor_T* visitor, AST_T* node);
AST_T* VV_Function_Definition(Visitor_T* visitor, AST_T* node);
AST_T* VV_Function_Call(Visitor_T* visitor, AST_T* node);
AST_T* VV_Tuple_Definition(Visitor_T* visitor, AST_T* node);
AST_T* VV_Tuple(Visitor_T* visitor, AST_T* node);
AST_T* VV_Table_Definition(Visitor_T* visitor, AST_T* node);
AST_T* VV_Table(Visitor_T* visitor, AST_T* node);
AST_T* VV_Dict_Definition(Visitor_T* visitor, AST_T* node);
AST_T* VV_Dict(Visitor_T* visitor, AST_T* node);
AST_T* VV_Class_Definition(Visitor_T* visitor, AST_T* node);
AST_T* VV_Class(Visitor_T* visitor, AST_T* node);
AST_T* VV_Try(Visitor_T* visitor, AST_T* node);
AST_T* VV_String(Visitor_T* visitor, AST_T* node);
AST_T* VV_Number(Visitor_T* visitor, AST_T* node);
AST_T* VV_Integer(Visitor_T* visitor, AST_T* node);
AST_T* VV_Bool(Visitor_T* visitor, AST_T* node);
AST_T* VV_Null(Visitor_T* visitor, AST_T* node);
AST_T* VV_Compound(Visitor_T* visitor, AST_T* node);
void Visitor_Clean(Visitor_T* visitor, AST_T* root);

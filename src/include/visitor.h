#pragma once

#include "AST.h"

typedef struct VISITOR_STRUCT {
    int returning;
    int call_depth;
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

AST_T* Visitor_Visit(Visitor_T* visitor, AST_T* node);
AST_T* VV_Variable_Definition(Visitor_T* visitor, AST_T* node);
AST_T* VV_Variable(Visitor_T* visitor, AST_T* node);
AST_T* VV_If(Visitor_T* visitor, AST_T* node);
AST_T* VV_If_Else(Visitor_T* visitor, AST_T* node);
AST_T* VV_Checks(Visitor_T* visitor, AST_T* node);
AST_T* VV_For(Visitor_T* visitor, AST_T* node);
AST_T* VV_While(Visitor_T* visitor, AST_T* node);
AST_T* VV_BinOp(Visitor_T* visitor, AST_T* node);
AST_T* VV_Unary_Not(Visitor_T* visitor, AST_T* node);
AST_T* VV_Arrow(Visitor_T* visitor, AST_T* node);
AST_T* VV_Dot(Visitor_T* visitor, AST_T* node);
AST_T* VV_Return(Visitor_T* visitor, AST_T* node);
AST_T* VV_Assignment(Visitor_T* visitor, AST_T* node);
AST_T* VV_Class_Instantiation(Visitor_T* visitor, AST_T* node);
AST_T* VV_Init_Call(Visitor_T* visitor, AST_T* node);
AST_T* VV_Function_Definition(Visitor_T* visitor, AST_T* node);
AST_T* VV_Function_Call(Visitor_T* visitor, AST_T* node);
AST_T* VV_Table_Definition(Visitor_T* visitor, AST_T* node);
AST_T* VV_Table(Visitor_T* visitor, AST_T* node);
AST_T* VV_Dict_Definition(Visitor_T* visitor, AST_T* node);
AST_T* VV_Dict(Visitor_T* visitor, AST_T* node);
AST_T* VV_Class_Definition(Visitor_T* visitor, AST_T* node);
AST_T* VV_Class(Visitor_T* visitor, AST_T* node);
AST_T* VV_String(Visitor_T* visitor, AST_T* node);
AST_T* VV_Number(Visitor_T* visitor, AST_T* node);
AST_T* VV_Bool(Visitor_T* visitor, AST_T* node);
AST_T* VV_Compound(Visitor_T* visitor, AST_T* node);
void Visitor_Clean(Visitor_T* visitor, AST_T* root);

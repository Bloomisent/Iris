#include <string.h>
#include "include/AST.h"
#include "include/token.h"
#include "include/visitor.h"     // for Iris_Throw

#include "include/compiler.h"
#include "include/value.h"

static Chunk* chunk;             // the chunk we are currently writing into

static void emit_op(uint8_t op, int line) { emit(chunk, op, line); }

static void emit_constant(Value v, int line) {
    int k = add_constant(chunk, v);
    if (k > 0xFFFF) Iris_Throw(NULL, NULL, "compile", "too many constants in one chunk");
    emit(chunk, OP_CONST, line);
    emit(chunk, (k >> 8) & 0xFF, line);
    emit(chunk, k & 0xFF, line);
}

static void compile_expr(AST_T* n);
static void compile_stmt(AST_T* n);

// RULE: compile_expr leaves exactly ONE value on the stack.
static void compile_expr(AST_T* n) {
    switch (n->type) {
        case AST_STRING: {
            ObjString* str = allocate_string(n->string_value, strlen(n->string_value));
            Value v;
            v.t = V_OBJ;
            v.as.o = (Obj*)str;

            emit_constant(v, n->current_line);
            return;
        }
        case AST_INTEGER: {
            Value v; v.t = V_INT; v.as.i = n->integer_value;
            emit_constant(v, n->current_line);
            return;
        }
        case AST_NUMBER: {
            Value v; v.t = V_NUM; v.as.n = n->number_value;
            emit_constant(v, n->current_line);
            return;
        }
        case AST_BINOP: {
            compile_expr(n->binop_left);
            compile_expr(n->binop_right);
            switch (n->binop_op) {
                case TOKEN_PLUS:     emit_op(OP_ADD, n->current_line); return;
                case TOKEN_MINUS:    emit_op(OP_SUB, n->current_line); return;
                case TOKEN_MULTIPLY: emit_op(OP_MUL, n->current_line); return;
                case TOKEN_DIVIDE:   emit_op(OP_DIV, n->current_line); return;
                default:
                    Iris_Throw(NULL, n, "compile", "operator %d is not supported yet", n->binop_op);
            }
            return;
        }
        case AST_TABLE_DEFINITION: {
            ObjTable* table = allocate_table(n->table_value, 0, 0);
            Value v;
            v.t = V_OBJ;
            v.as.o = (Obj*)table;

            emit_constant(v, n->current_line);
            return;
        }
        default:
            Iris_Throw(NULL, n, "compile", "expression type %d is not supported yet", n->type);
    }
}

// RULE: compile_stmt leaves NOTHING on the stack.
static void compile_stmt(AST_T* n) {
    switch (n->type) {
        case AST_COMPOUND:
            for (size_t i = 0; i < n->compound_size; i++)
                compile_stmt(n->compound_value[i]);
            return;
        case AST_NOOP:
            return;
        case AST_FUNCTION_CALL:
            if (strcmp(n->function_call_name, "print") == 0) {     // temporary: print is an opcode for now
                for (size_t i = 0; i < n->function_call_arguments_size; i++) {
                    compile_expr(n->function_call_arguments[i]);
                    emit_op(OP_PRINT, n->current_line);             // PRINT pops what it prints
                }
                return;
            }
            Iris_Throw(NULL, n, "compile", "function '%s' is not supported yet", n->function_call_name);
            return;
        default:
            compile_expr(n);               // an expression used as a statement:
            emit_op(OP_POP, n->current_line);   // evaluate it, then throw the result away
    }
}

void compile_program(AST_T* root, Chunk* out) {
    chunk = out;
    compile_stmt(root);
    emit_op(OP_RETURN, root->current_line);
}

#pragma once
#include <stdint.h>
#include "value.h"

typedef enum {
    OP_CONST,    // operand: 2-byte constant index. Pushes that constant.
    OP_ADD,      // pops two, pushes a + b
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_PRINT,    // pops one and prints it (temporary, so we can see results)
    OP_POP,
    OP_RETURN    // stops the program
} OpCode;

typedef struct {
    uint8_t* code;   // the instruction bytes
    int count;       // how many bytes are used
    int cap;         // how many bytes are allocated
    Value* consts;   // constant pool (numbers, strings...)
    int nconsts;
    int constcap;
    int* lines;      // lines[i] = source line of code[i]
} Chunk;

Chunk* chunk_init(Chunk* c);
void chunk_free(Chunk* c);
void emit(Chunk* c, uint8_t byte, int line);
int  add_constant(Chunk* c, Value v);          // returns the constant's index
int  emit_jump(Chunk* c, uint8_t op, int line);
void patch_jump(Chunk* c, int at);

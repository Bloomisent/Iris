#pragma once
#include "chunk.h"

void print_value(Value v);
void disassemble_chunk(Chunk* c, const char* name);
int  disassemble_instruction(Chunk* c, int offset);   // returns the offset of the next instruction

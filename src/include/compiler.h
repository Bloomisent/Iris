#pragma once
#include "AST.h"
#include "chunk.h"

// Compiles a whole program (the root AST node) into 'out'. 'out' must already be chunk_init'ed.
void compile_program(AST_T* root, Chunk* out);

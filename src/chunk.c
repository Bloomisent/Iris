#include <stdlib.h>
#include "include/chunk.h"

Chunk* chunk_init(Chunk* c) {
    c->code = NULL;   c->count = 0;    c->cap = 0;
    c->consts = NULL; c->nconsts = 0;  c->constcap = 0;
    c->lines = NULL;
    return c;
}

void chunk_free(Chunk* c) {
    free(c->code);
    free(c->consts);
    free(c->lines);
    chunk_init(c);
}

void emit(Chunk* c, uint8_t byte, int line) {
    if (c->count + 1 > c->cap) {
        c->cap = c->cap < 8 ? 8 : c->cap * 2;
        c->code  = realloc(c->code,  c->cap * sizeof(uint8_t));
        c->lines = realloc(c->lines, c->cap * sizeof(int));
        if (!c->code || !c->lines) exit(EXIT_FAILURE);
    }
    c->code[c->count]  = byte;
    c->lines[c->count] = line;
    c->count++;
}

int add_constant(Chunk* c, Value v) {
    if (c->nconsts + 1 > c->constcap) {
        c->constcap = c->constcap < 8 ? 8 : c->constcap * 2;
        c->consts = realloc(c->consts, c->constcap * sizeof(Value));
        if (!c->consts) exit(EXIT_FAILURE);
    }
    c->consts[c->nconsts] = v;
    return c->nconsts++;
}

int emit_jump(Chunk* c, uint8_t op, int line) {
    emit(c, op, line);
    emit(c, 0xFF, line);
    emit(c, 0xFF, line);
    return c->count - 2;              // where the 2 placeholder bytes start
}

void patch_jump(Chunk* c, int at) {
    int jump = c->count - at - 2;     // distance from just after the operand to here
    if (jump > 0xFFFF) exit(EXIT_FAILURE);
    c->code[at]     = (jump >> 8) & 0xFF;
    c->code[at + 1] = jump & 0xFF;
}

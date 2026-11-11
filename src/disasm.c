#include <stdio.h>
#include "include/disasm.h"

void print_value(Value v) {
    switch (v.t) {
        case V_NULL: printf("null"); break;
        case V_BOOL: printf(v.as.b ? "true" : "false"); break;
        case V_INT:  printf("%d", v.as.i); break;
        case V_NUM:  printf("%g", v.as.n); break;
        case V_OBJ:  {
            switch (v.as.o->t){
                case O_STRING:
                    printf("%s", ((ObjString*)v.as.o)->chars);
                    break;
                case O_TABLE:
                    printf("%p", ((ObjTable*)v.as.o)->table);
                    break;
                default:
                    printf("<object>");
                    break;
            }
        }
    }
}

static int simple_instruction(const char* name, int offset) {
    printf("%s\n", name);
    return offset + 1;
}

static int constant_instruction(const char* name, Chunk* c, int offset) {
    int k = (c->code[offset + 1] << 8) | c->code[offset + 2];
    printf("%-10s %4d  (", name, k);
    print_value(c->consts[k]);
    printf(")\n");
    return offset + 3;
}

int disassemble_instruction(Chunk* c, int offset) {
    printf("%04d ", offset);
    if (offset > 0 && c->lines[offset] == c->lines[offset - 1]) printf("   | ");
    else printf("%4d ", c->lines[offset]);

    switch (c->code[offset]) {
        case OP_CONST:  return constant_instruction("CONST", c, offset);
        case OP_ADD:    return simple_instruction("ADD", offset);
        case OP_SUB:    return simple_instruction("SUB", offset);
        case OP_MUL:    return simple_instruction("MUL", offset);
        case OP_DIV:    return simple_instruction("DIV", offset);
        case OP_POP:    return simple_instruction("POP", offset);
        case OP_PRINT:  return simple_instruction("PRINT", offset);
        case OP_RETURN: return simple_instruction("RETURN", offset);
        default:
            printf("unknown opcode %d\n", c->code[offset]);
            return offset + 1;
    }
}

void disassemble_chunk(Chunk* c, const char* name) {
    printf("== %s ==\n", name);
    for (int offset = 0; offset < c->count; )
        offset = disassemble_instruction(c, offset);
}

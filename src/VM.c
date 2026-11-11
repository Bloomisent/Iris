#include <stdio.h>
#include <stdlib.h>
#include "include/VM.h"
#include "include/disasm.h" // for print_value

#define STACK_MAX 256

typedef struct {
    Chunk* chunk;
    uint8_t* ip;
    Value stack[STACK_MAX];
    Value* stack_top;
} VM;

static VM vm;

static void reset_stack(void) {
    vm.stack_top = vm.stack;
}

static void push(Value value) {
    if (vm.stack_top - vm.stack >= STACK_MAX) {
        fprintf(stderr, "Stack overflow.\n");
        exit(EXIT_FAILURE);
    }
    *vm.stack_top++ = value;
}

static Value pop(void) {
    if (vm.stack_top == vm.stack) {
        fprintf(stderr, "Stack underflow.\n");
        exit(EXIT_FAILURE);
    }
    return *--vm.stack_top;
}

static Value arith_add(Value a, Value b) {
    Value res;
    if (a.t == V_INT && b.t == V_INT) {
        res.t = V_INT;
        res.as.i = a.as.i + b.as.i;
    } else {
        res.t = V_NUM;
        double da = (a.t == V_INT) ? a.as.i : a.as.n;
        double db = (b.t == V_INT) ? b.as.i : b.as.n;
        res.as.n = da + db;
    }
    return res;
}

static Value arith_sub(Value a, Value b) {
    Value res;
    if (a.t == V_INT && b.t == V_INT) {
        res.t = V_INT;
        res.as.i = a.as.i - b.as.i;
    } else {
        res.t = V_NUM;
        double da = (a.t == V_INT) ? a.as.i : a.as.n;
        double db = (b.t == V_INT) ? b.as.i : b.as.n;
        res.as.n = da - db;
    }
    return res;
}

static Value arith_mul(Value a, Value b) {
    Value res;
    if (a.t == V_INT && b.t == V_INT) {
        res.t = V_INT;
        res.as.i = a.as.i * b.as.i;
    } else {
        res.t = V_NUM;
        double da = (a.t == V_INT) ? a.as.i : a.as.n;
        double db = (b.t == V_INT) ? b.as.i : b.as.n;
        res.as.n = da * db;
    }
    return res;
}

static Value arith_div(Value a, Value b) {
    Value res;
    res.t = V_NUM;
    double da = (a.t == V_INT) ? a.as.i : a.as.n;
    double db = (b.t == V_INT) ? b.as.i : b.as.n;
    if (db == 0.0) {
        fprintf(stderr, "Division by zero.\n");
        exit(EXIT_FAILURE);
    }
    res.as.n = da / db;
    return res;
}

static void run(void) {
    for (;;) {
        uint8_t instruction = *vm.ip++;
        switch (instruction) {
            case OP_CONST: {
                uint16_t constant_index = (vm.ip[0] << 8) | vm.ip[1];
                vm.ip += 2;
                push(vm.chunk->consts[constant_index]);
                break;
            }
            case OP_ADD: {
                Value b = pop();
                Value a = pop();
                push(arith_add(a, b));
                break;
            }
            case OP_SUB: {
                Value b = pop();
                Value a = pop();
                push(arith_sub(a, b));
                break;
            }
            case OP_MUL: {
                Value b = pop();
                Value a = pop();
                push(arith_mul(a, b));
                break;
            }
            case OP_DIV: {
                Value b = pop();
                Value a = pop();
                push(arith_div(a, b));
                break;
            }
            case OP_PRINT: {
                Value value = pop();
                print_value(value);
                printf("\n");
                break;
            }
            case OP_POP: {
                pop();
                break;
            }
            case OP_RETURN: {
                return;
            }
            default:
                fprintf(stderr, "Unknown opcode %d\n", instruction);
                return;
        }
    }
}

void interpret(Chunk* chunk) {
    vm.chunk = chunk;
    vm.ip = chunk->code;
    reset_stack();
    run();
}

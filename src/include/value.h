#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef struct Obj Obj; // forward declaration

typedef enum { V_NULL, V_BOOL, V_INT, V_NUM, V_OBJ } VType;
typedef struct {
    VType t;
    union {
        bool b;
        int i;
        double n;
        Obj* o;
    } as;
} Value;

typedef enum {
    O_STRING,
    O_TUPLE,
    O_TABLE,
    O_DICT,
    O_INSTANCE,
    O_FUNC
} OType;

struct Obj { OType t; };  // every object kind embeds this first

typedef struct {
    Obj obj;
    int length;
    char* chars;
} ObjString;

typedef struct {
    Obj obj;
    int capacity;
    int count;
    void* table;
} ObjTable;

static inline bool IS_STRING(Value v) { return v.t == V_OBJ && v.as.o->t == O_STRING; }
static inline ObjString* AS_STRING(Value v) { return (ObjString*)v.as.o; }
static inline const char* AS_CSTRING(Value v) { return AS_STRING(v)->chars; }

static inline bool IS_TABLE(Value v) { return v.t == V_OBJ && v.as.o->t == O_TABLE; }
static inline ObjTable* AS_TABLE(Value v) { return (ObjTable*)v.as.o; }
static inline const char* AS_CTABLE(Value v) { return AS_TABLE(v)->table; }

// value.c

ObjString* allocate_string(const char* chars, int length);
ObjTable* allocate_table(const void* table, int capacity, int count);
void free_object(Obj* obj);

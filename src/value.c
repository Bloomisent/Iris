#include <string.h>
#include <stdlib.h>
#include "include/value.h"

ObjString* allocate_string(const char* chars, int length) {
    ObjString* string = malloc(sizeof(ObjString));
    string->obj.t = O_STRING;
    string->length = length;

    // Allocate and copy characters (+1 for null terminator)
    string->chars = malloc(length + 1);
    memcpy(string->chars, chars, length);
    string->chars[length] = '\0';

    return string;
}

ObjTable* allocate_table(const void* table, int capacity, int count) {
    ObjTable* tble = malloc(sizeof(ObjTable));
    tble->obj.t = O_TABLE;

    // Allocate and copy characters (+1 for null terminator)
    tble->capacity = capacity;
    tble->count = count;
    tble->table = NULL;
    return tble;
}

void free_object(Obj* obj) {
    switch (obj->t) {
        case O_STRING: {
            ObjString* string = (ObjString*)obj;
            free(string->chars);
            free(string);
            break;
        }
        case O_TABLE: {
            ObjTable* table = (ObjTable*)obj;
            free(table->table);
            free(table);
            break;
        }

        // other object types soon
    }
}

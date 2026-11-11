#pragma once

#include <stdint.h>
#include "chunk.h"

// Initializes and executes a chunk in the VM
void interpret(Chunk* chunk);

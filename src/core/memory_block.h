#ifndef CORE_MEMORY_BLOCK_H
#define CORE_MEMORY_BLOCK_H

#include <stddef.h>

typedef struct {
    char *memory;
    size_t size;
} memory_block;

int core_memory_block_init(memory_block *block, size_t initial_size);

int core_memory_block_ensure_size(memory_block *block, size_t size);

void core_memory_block_free(memory_block *block);

#endif // CORE_MEMORY_BLOCK_H

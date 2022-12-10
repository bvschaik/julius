#include "memory_block.h"

#include <stdlib.h>

int core_memory_block_init(memory_block *block, size_t initial_size)
{
    block->memory = calloc(sizeof(char), initial_size);
    if (!block->memory) {
        block->size = 0;
        return 0;
    }
    block->size = sizeof(char) * initial_size;
    return 1;
}

int core_memory_block_ensure_size(memory_block *block, size_t size)
{
    if (size <= block->size) {
        return 1;
    }
    char *new_mem = realloc(block->memory, sizeof(char) * size);
    if (new_mem == NULL) {
        return 0;
    }
    block->memory = new_mem;
    block->size = sizeof(char) * size;
    return 1;
}

void core_memory_block_free(memory_block *block)
{
    free(block->memory);
    block->size = 0;
}

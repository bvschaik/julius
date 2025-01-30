#include "array.h"

int array_add_blocks(void ***data, unsigned int *blocks, unsigned int items_per_block, unsigned int item_size, unsigned int num_blocks)
{
    if (num_blocks == 0) {
        return 1;
    }
    void **new_block_pointer = realloc(*data, sizeof(void *) * (*blocks + num_blocks));
    if (!new_block_pointer) {
        return 0;
    }
    *data = new_block_pointer;
    for (unsigned int i = 0; i < num_blocks; i++) {
        void *new_block = malloc((size_t) item_size * items_per_block);
        if (!new_block) {
            return 0;
        }
        new_block_pointer[*blocks] = new_block;
        (*blocks)++;
    }
    return 1;
}

void array_free(void **data, unsigned int blocks)
{
    for (unsigned int i = 0; i < blocks; i++) {
        free(data[i]);
    }
    free(data);
}

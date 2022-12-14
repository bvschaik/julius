#include "array.h"

int array_add_blocks(void ***data, int *blocks, int items_per_block, int item_size, int num_blocks)
{
    if (num_blocks <= 0) {
        return 1;
    }
    void **new_block_pointer = realloc(*data, sizeof(void *) * (*blocks + num_blocks));
    if (!new_block_pointer) {
        return 0;
    }
    *data = new_block_pointer;
    void *new_blocks = malloc((size_t) item_size * items_per_block * num_blocks);
    if (!new_blocks) {
        return 0;
    }
    for (int i = 0; i < num_blocks; i++) {
        new_block_pointer[*blocks] = new_blocks + (size_t) item_size * items_per_block * i;
        (*blocks)++;
    }
    return 1;
}

void array_free(void **data, int blocks)
{
    for (int i = 0; i < blocks; i++) {
        free(data[i]);
    }
    free(data);
}

#include "array.h"

int array_increase_capacity(char **data, int *capacity, int item_size, int new_capacity)
{
    if (new_capacity <= 0) {
        new_capacity = 10;
    }
    if (new_capacity <= *capacity) {
        return 1;
    }
    void *new_data = realloc(*data, item_size * new_capacity);
    if (!new_data) {
        return 0;
    }
    *data = new_data;
    *capacity = new_capacity;
    return 1;
}

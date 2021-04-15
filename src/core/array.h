#ifndef CORE_ARRAY_H
#define CORE_ARRAY_H

#include <stdlib.h>
#include <string.h>

/**
 * Creates an array structure
 * @param T The type of item that the array holds
 */
#define array(T) \
struct { \
    T *items; \
    int size; \
    int capacity; \
    void (*constructor)(T *, int); \
    int (*in_use)(const T *); \
}

/**
 * Initiates an array
 * @param a The array structure
 * @param length The amount of items to preallocate memory to
 * @param new_item_callback A function to run when a new item is added to an array. Can be null.
 *        Items are are always zeroed before the function is called.
 *        The function should have the following signature:
 *        void(<item> *, int position). <item> is the new array item, position is its index.
 * @param in_use_callback A function to check if the current position has a valid item. Can be null.
 *        If null, it is assumed that every item in the array is valid.
 *        The function should have the following signature:
 *        int(<item> *). <item> is the current array item. Should return 1 if the item is being used.
 * @return Whether memory was properly allocated.
 */
#define array_init(a, length, new_item_callback, in_use_callback) \
( \
    free((a).items), \
    memset(&(a), 0, sizeof(a)), \
    (a).constructor = new_item_callback, \
    (a).in_use = in_use_callback, \
    (length) ? array_reserve(a, length) : 1 \
)

/**
 * Creates a new item for the array, either by finding an available empty item or by expanding the array.
 * @param a The array structure
 * @param index The index upon which to start searching for a free slot.
 * @param ptr A pointer that will get the new item. Will be null if there was a memory allocation error.
 */
#define array_new_item(a, index, ptr) \
{ \
    ptr = 0; \
    if ((a).in_use) { \
        for (int i = index; i < (a).size; i++) { \
            if (!(a).in_use(&(a).items[i])) { \
                ptr = &(a).items[i]; \
                memset(ptr, 0, sizeof(*(a).items)); \
                if ((a).constructor) { \
                    (a).constructor(ptr, i); \
                } \
                break; \
            } \
        } \
    } \
    if (!ptr) { \
        ptr = array_advance(a); \
    } \
}

/**
 * Advances an array, creating a new item, incrementing size and increasing the memory buffer if needed
 * @param a The array structure
 * @return A pointer to the newest item of the array, or 0 if there was a memory allocation error
 */
#define array_advance(a) \
( \
    (a).size < (a).capacity || \
    array_reserve(a, (a).capacity << 1) ? \
    array_next(a) : 0 \
)

/**
 * Returns the first item of the array, or 0 if the array has no items
 * @param a The array structure
 * @return A pointer to the first item of the array, or 0 if the array has no items
 */
#define array_first(a) \
    ( (a).size > 0 ? (a).items : 0 )

/**
 * Returns the last item of the array, or 0 if the array has no items
 * @param a The array structure
 * @return A pointer to the last item of the array, or 0 if the array has no items
 */
#define array_last(a) \
    ( (a).size > 0 ? &(a).items[(a).size - 1] : 0 )

/**
 * Iterates through an array
 * @param a The array structure
 * @param item A pointer to the array item that will be used to traverse the structure
 */
#define array_foreach(a, item) \
    for(int i = 0; i < (a).size && ((item) = &(a).items[i]); i++)

/**
 * Trims an array, removing its latest items that satisfy condition until the first one that doesn't.
 * The first item of the array is always kept.
 * @param a The array structure
 */
#define array_trim(a) \
{ \
    if ((a).size > 1 && (a).in_use) { \
        while ((a).size - 1 && !(a).in_use(&(a).items[(a).size - 1])) { \
            (a).size--; \
        } \
    } \
}

/**
 * Gets the next item of the array without checking for memory bounds.
 * ONLY use when you're SURE the array memory bounds won't be exceeded!
 * @param a The array structure
 * @return A pointer to the newest item of the array, or 0 if there was a memory allocation error
 */
#define array_next(a) \
( \
    memset(&(a).items[(a).size], 0, sizeof(*(a).items)), \
    (a).constructor ? (a).constructor(&(a).items[(a).size], (a).size) : 0, \
    &(a).items[(a).size++] \
)

/**
 * This definition is private and should not be used
 */
#define array_reserve(a, length) \
( \
    array_increase_capacity((char **)&(a).items, &(a).capacity, sizeof(*(a).items), length) \
)

/**
 * This function is private and should not be used
 */
int array_increase_capacity(char **data, int *capacity, int item_size, int new_capacity);

#endif // CORE_ARRAY_H

#include "building/model.h"

#include "core/debug.h"
#include "core/io.h"
#include "core/string.h"

#include <stdlib.h>
#include <string.h>

#define TMP_BUFFER_SIZE 100000

#define NUM_BUILDINGS 130
#define NUM_HOUSES 20

static const uint8_t ALL_BUILDINGS[] = {'A', 'L', 'L', ' ', 'B', 'U', 'I', 'L', 'D', 'I', 'N', 'G', 'S', 0};
static const uint8_t ALL_HOUSES[] = {'A', 'L', 'L', ' ', 'H', 'O', 'U', 'S', 'E', 'S', 0};

static model_building buildings[NUM_BUILDINGS];
static model_house houses[NUM_HOUSES];

static int strings_equal(const uint8_t *a, const uint8_t *b, size_t len)
{
    for (int i = 0; i < len; i++, a++, b++) {
        if (*a != *b) {
            return 0;
        }
    }
    return 1;
}

static size_t string_length(const uint8_t *str)
{
    size_t len = 0;
    while (*str) {
        len++;
        str++;
    }
    return len;
}

static int index_of_string(const uint8_t *haystack, const uint8_t *needle, int haystack_length)
{
    size_t needle_length = string_length(needle);
    for (int i = 0; i < haystack_length; i++) {
        if (haystack[i] == needle[0] && strings_equal(&haystack[i], needle, needle_length)) {
            return i + 1;
        }
    }
    return 0;
}

static int index_of(const uint8_t *haystack, uint8_t needle, int haystack_length)
{
    for (int i = 0; i < haystack_length; i++) {
        if (haystack[i] == needle) {
            return i + 1;
        }
    }
    return 0;
}

static const uint8_t *skip_non_digits(const uint8_t *str)
{
    int safeguard = 0;
    while (1) {
        if (++safeguard >= 1000) {
            break;
        }
        if ((*str >= '0' && *str <= '9') || *str == '-') {
            break;
        }
        str++;
    }
    return str;
}


static const uint8_t *get_value(const uint8_t *ptr, int filesize, int *value)
{
    ptr = skip_non_digits(ptr);
    *value = string_to_int(ptr);
    ptr += index_of(ptr, ',', filesize);
    return ptr;
}

int model_load()
{
    uint8_t *buffer = (uint8_t *) malloc(TMP_BUFFER_SIZE);
    if (!buffer) {
        debug_log("ERR:no spare memory for model", 0, 0);
        return 0;
    }
    memset(buffer, 0, TMP_BUFFER_SIZE);
    int filesize = io_read_file_into_buffer("c3_model.txt", buffer, TMP_BUFFER_SIZE);
    if (filesize == 0) {
        debug_log("ERR:no c3_model.txt file", 0, 0);
        free(buffer);
        return 0;
    }

    int num_lines = 0;
    int guard = 200;
    int brace_index;
    const uint8_t *ptr = &buffer[index_of_string(buffer, ALL_BUILDINGS, filesize)];
    do {
        guard--;
        brace_index = index_of(ptr, '{', filesize);
        if (brace_index) {
            ptr += brace_index;
            num_lines++;
        }
    } while (brace_index && guard > 0);

    if (num_lines != NUM_BUILDINGS + NUM_HOUSES) {
        debug_log("ERR:model has incorrect no of lines ", 0, num_lines + 1);
        free(buffer);
        return 0;
    }

    int dummy;
    ptr = &buffer[index_of_string(buffer, ALL_BUILDINGS, filesize)];
    for (int i = 0; i < NUM_BUILDINGS; i++) {
        ptr += index_of(ptr, '{', filesize);

        ptr = get_value(ptr, filesize, &buildings[i].cost);
        ptr = get_value(ptr, filesize, &buildings[i].desirability_value);
        ptr = get_value(ptr, filesize, &buildings[i].desirability_step);
        ptr = get_value(ptr, filesize, &buildings[i].desirability_step_size);
        ptr = get_value(ptr, filesize, &buildings[i].desirability_range);
        ptr = get_value(ptr, filesize, &buildings[i].laborers);
        ptr = get_value(ptr, filesize, &dummy);
        ptr = get_value(ptr, filesize, &dummy);
    }

    ptr = &buffer[index_of_string(buffer, ALL_HOUSES, filesize)];

    for (int i = 0; i < NUM_HOUSES; i++) {
        ptr += index_of(ptr, '{', filesize);

        ptr = get_value(ptr, filesize, &houses[i].devolve_desirability);
        ptr = get_value(ptr, filesize, &houses[i].evolve_desirability);
        ptr = get_value(ptr, filesize, &houses[i].entertainment);
        ptr = get_value(ptr, filesize, &houses[i].water);
        ptr = get_value(ptr, filesize, &houses[i].religion);
        ptr = get_value(ptr, filesize, &houses[i].education);
        ptr = get_value(ptr, filesize, &houses[i].food);
        ptr = get_value(ptr, filesize, &houses[i].barber);
        ptr = get_value(ptr, filesize, &houses[i].bathhouse);
        ptr = get_value(ptr, filesize, &houses[i].health);
        ptr = get_value(ptr, filesize, &houses[i].food_types);
        ptr = get_value(ptr, filesize, &houses[i].pottery);
        ptr = get_value(ptr, filesize, &houses[i].oil);
        ptr = get_value(ptr, filesize, &houses[i].furniture);
        ptr = get_value(ptr, filesize, &houses[i].wine);
        ptr = get_value(ptr, filesize, &dummy);
        ptr = get_value(ptr, filesize, &dummy);
        ptr = get_value(ptr, filesize, &houses[i].prosperity);
        ptr = get_value(ptr, filesize, &houses[i].max_people);
        ptr = get_value(ptr, filesize, &houses[i].tax_multiplier);
    }

    debug_log(" OK: model loaded", 0, 0);
    free(buffer);
    return 1;
}

const model_building *model_get_building(building_type type)
{
    return &buildings[type];
}

const model_house *model_get_house(house_level level)
{
    return &houses[level];
}

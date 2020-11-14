#include "building/model.h"

#include "core/io.h"
#include "core/log.h"
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

static int strings_equal(const uint8_t *a, const uint8_t *b, int len)
{
    for (int i = 0; i < len; i++, a++, b++) {
        if (*a != *b) {
            return 0;
        }
    }
    return 1;
}

static int index_of_string(const uint8_t *haystack, const uint8_t *needle, int haystack_length)
{
    int needle_length = string_length(needle);
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

static const uint8_t *get_value(const uint8_t *ptr, const uint8_t *end_ptr, int *value)
{
    ptr = skip_non_digits(ptr);
    *value = string_to_int(ptr);
    ptr += index_of(ptr, ',', (int) (end_ptr - ptr));
    return ptr;
}

int model_load(void)
{
    uint8_t *buffer = (uint8_t *) malloc(TMP_BUFFER_SIZE);
    if (!buffer) {
        log_error("No memory for model", 0, 0);
        return 0;
    }
    memset(buffer, 0, TMP_BUFFER_SIZE);
    int filesize = io_read_file_into_buffer("c3_model.txt", NOT_LOCALIZED, buffer, TMP_BUFFER_SIZE);
    if (filesize == 0) {
        log_error("No c3_model.txt file", 0, 0);
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
        log_error("Model has incorrect no of lines ", 0, num_lines + 1);
        free(buffer);
        return 0;
    }

    int dummy;
    ptr = &buffer[index_of_string(buffer, ALL_BUILDINGS, filesize)];
    const uint8_t *end_ptr = &buffer[filesize];
    for (int i = 0; i < NUM_BUILDINGS; i++) {
        ptr += index_of(ptr, '{', filesize);

        ptr = get_value(ptr, end_ptr, &buildings[i].cost);
        ptr = get_value(ptr, end_ptr, &buildings[i].desirability_value);
        ptr = get_value(ptr, end_ptr, &buildings[i].desirability_step);
        ptr = get_value(ptr, end_ptr, &buildings[i].desirability_step_size);
        ptr = get_value(ptr, end_ptr, &buildings[i].desirability_range);
        ptr = get_value(ptr, end_ptr, &buildings[i].laborers);
        ptr = get_value(ptr, end_ptr, &dummy);
        ptr = get_value(ptr, end_ptr, &dummy);
    }

    ptr = &buffer[index_of_string(buffer, ALL_HOUSES, filesize)];

    for (int i = 0; i < NUM_HOUSES; i++) {
        ptr += index_of(ptr, '{', filesize);

        ptr = get_value(ptr, end_ptr, &houses[i].devolve_desirability);
        ptr = get_value(ptr, end_ptr, &houses[i].evolve_desirability);
        ptr = get_value(ptr, end_ptr, &houses[i].entertainment);
        ptr = get_value(ptr, end_ptr, &houses[i].water);
        ptr = get_value(ptr, end_ptr, &houses[i].religion);
        ptr = get_value(ptr, end_ptr, &houses[i].education);
        ptr = get_value(ptr, end_ptr, &houses[i].food);
        ptr = get_value(ptr, end_ptr, &houses[i].barber);
        ptr = get_value(ptr, end_ptr, &houses[i].bathhouse);
        ptr = get_value(ptr, end_ptr, &houses[i].health);
        ptr = get_value(ptr, end_ptr, &houses[i].food_types);
        ptr = get_value(ptr, end_ptr, &houses[i].pottery);
        ptr = get_value(ptr, end_ptr, &houses[i].oil);
        ptr = get_value(ptr, end_ptr, &houses[i].furniture);
        ptr = get_value(ptr, end_ptr, &houses[i].wine);
        ptr = get_value(ptr, end_ptr, &dummy);
        ptr = get_value(ptr, end_ptr, &dummy);
        ptr = get_value(ptr, end_ptr, &houses[i].prosperity);
        ptr = get_value(ptr, end_ptr, &houses[i].max_people);
        ptr = get_value(ptr, end_ptr, &houses[i].tax_multiplier);
    }

    log_info("Model loaded", 0, 0);
    free(buffer);
    return 1;
}

const model_building MODEL_ROADBLOCK = {40,0,0,0,0};
const model_building MODEL_WORK_CAMP = { 150,-10,2,3,4,25 };
const model_building MODEL_ENGINEER_GUILD = { 150,-8,1,2,4,25 };
const model_building MODEL_GRAND_TEMPLE_CERES = { 2500,20,2,-4,5,50 };
const model_building MODEL_GRAND_TEMPLE_NEPTUNE = { 2500,20,2,-4,5,50 };
const model_building MODEL_GRAND_TEMPLE_MERCURY = { 2500,20,2,-4,5,50 };
const model_building MODEL_GRAND_TEMPLE_MARS = { 2500,20,2,-4,5,50 };
const model_building MODEL_GRAND_TEMPLE_VENUS = { 2500,20,2,-4,5,50 };
const model_building MODEL_PANTHEON = { 3500,20,2,-4,5,50 };
const model_building MODEL_LIGHTHOUSE = { 2000,6,1,-1,4,30 };
const model_building MODEL_MESS_HALL = { 100,-8,1,2,4,10 };
const model_building MODEL_TAVERN = { 25,-3,1,2,4,8 };
const model_building MODEL_GRAND_GARDEN = { 400,0,0,0,0,0 };
const model_building MODEL_NULL = { 0,0,0,0,0 };

const model_building *model_get_building(building_type type)
{
    switch (type) {
        case BUILDING_ROADBLOCK:
            return &MODEL_ROADBLOCK;
        case BUILDING_WORKCAMP:
            return &MODEL_WORK_CAMP;
        case BUILDING_ENGINEER_GUILD:
            return &MODEL_ENGINEER_GUILD;
        case BUILDING_GRAND_TEMPLE_CERES:
            return &MODEL_GRAND_TEMPLE_CERES;
        case BUILDING_GRAND_TEMPLE_NEPTUNE:
            return &MODEL_GRAND_TEMPLE_NEPTUNE;
        case BUILDING_GRAND_TEMPLE_MERCURY:
            return &MODEL_GRAND_TEMPLE_MERCURY;
        case BUILDING_GRAND_TEMPLE_MARS:
            return &MODEL_GRAND_TEMPLE_MARS;
        case BUILDING_GRAND_TEMPLE_VENUS:
            return &MODEL_GRAND_TEMPLE_VENUS;
        case BUILDING_PANTHEON:
            return &MODEL_PANTHEON;
        case BUILDING_MESS_HALL:
            return &MODEL_MESS_HALL;
        case BUILDING_LIGHTHOUSE:
            return &MODEL_LIGHTHOUSE;
        case BUILDING_TAVERN:
            return &MODEL_TAVERN;
        case BUILDING_GRAND_GARDEN:
            return &MODEL_GRAND_GARDEN;
        default:
            break;
    }

    if (type >= BUILDING_PINE_TREE && type <= BUILDING_SMALL_STATUE_ALT_B) {
        return &buildings[41];
    }

    if(type == BUILDING_SMALL_POND || type==BUILDING_OBELISK) {
        return &buildings[42];
    }

    if(type == BUILDING_LARGE_POND) {
        return &buildings[43];
    }

    if (type > 129) {
        return &MODEL_NULL;
    }
    else {
        return &buildings[type];
    }
}

const model_house *model_get_house(house_level level)
{
    return &houses[level];
}

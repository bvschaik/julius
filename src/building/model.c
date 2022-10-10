#include "building/model.h"

#include "city/resource.h"
#include "core/io.h"
#include "core/log.h"
#include "core/string.h"

#include <stdlib.h>
#include <string.h>

#define TMP_BUFFER_SIZE 100000

#define NUM_BUILDINGS 130
#define NUM_HOUSES 20

static const uint8_t ALL_BUILDINGS[] = { 'A', 'L', 'L', ' ', 'B', 'U', 'I', 'L', 'D', 'I', 'N', 'G', 'S', 0 };
static const uint8_t ALL_HOUSES[] = { 'A', 'L', 'L', ' ', 'H', 'O', 'U', 'S', 'E', 'S', 0 };

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

static void override_model_data(void)
{
    buildings[BUILDING_LARGE_TEMPLE_CERES].desirability_value = 14;
    buildings[BUILDING_LARGE_TEMPLE_CERES].desirability_step = 2;
    buildings[BUILDING_LARGE_TEMPLE_CERES].desirability_step_size = -2;
    buildings[BUILDING_LARGE_TEMPLE_CERES].desirability_range = 5;

    buildings[BUILDING_LARGE_TEMPLE_NEPTUNE].desirability_value = 14;
    buildings[BUILDING_LARGE_TEMPLE_NEPTUNE].desirability_step = 2;
    buildings[BUILDING_LARGE_TEMPLE_NEPTUNE].desirability_step_size = -2;
    buildings[BUILDING_LARGE_TEMPLE_NEPTUNE].desirability_range = 5;

    buildings[BUILDING_LARGE_TEMPLE_MERCURY].desirability_value = 14;
    buildings[BUILDING_LARGE_TEMPLE_MERCURY].desirability_step = 2;
    buildings[BUILDING_LARGE_TEMPLE_MERCURY].desirability_step_size = -2;
    buildings[BUILDING_LARGE_TEMPLE_MERCURY].desirability_range = 5;

    buildings[BUILDING_LARGE_TEMPLE_MARS].desirability_value = 14;
    buildings[BUILDING_LARGE_TEMPLE_MARS].desirability_step = 2;
    buildings[BUILDING_LARGE_TEMPLE_MARS].desirability_step_size = -2;
    buildings[BUILDING_LARGE_TEMPLE_MARS].desirability_range = 5;

    buildings[BUILDING_LARGE_TEMPLE_VENUS].desirability_value = 14;
    buildings[BUILDING_LARGE_TEMPLE_VENUS].desirability_step = 2;
    buildings[BUILDING_LARGE_TEMPLE_VENUS].desirability_step_size = -2;
    buildings[BUILDING_LARGE_TEMPLE_VENUS].desirability_range = 5;

    buildings[BUILDING_WELL].laborers = 0;
    buildings[BUILDING_GATEHOUSE].laborers = 0;
    buildings[BUILDING_FORT_JAVELIN].laborers = 0;
    buildings[BUILDING_FORT_LEGIONARIES].laborers = 0;
    buildings[BUILDING_FORT_MOUNTED].laborers = 0;
    buildings[BUILDING_FORT].laborers = 0;
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
        ptr = get_value(ptr, end_ptr, &dummy);
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

    override_model_data();

    log_info("Model loaded", 0, 0);
    free(buffer);
    return 1;
}

const model_building MODEL_ROADBLOCK = { 12,0,0,0,0 };
const model_building MODEL_WORK_CAMP = { 150,-10,2,3,4,20 };
const model_building MODEL_ARCHITECT_GUILD = { 200,-8,1,2,4,12 };
const model_building MODEL_GRAND_TEMPLE_CERES = { 2500,20,2,-4,5,50 };
const model_building MODEL_GRAND_TEMPLE_NEPTUNE = { 2500,20,2,-4,5,50 };
const model_building MODEL_GRAND_TEMPLE_MERCURY = { 2500,20,2,-4,5,50 };
const model_building MODEL_GRAND_TEMPLE_MARS = { 2500,20,2,-4,5,50 };
const model_building MODEL_GRAND_TEMPLE_VENUS = { 2500,20,2,-4,5,50 };
const model_building MODEL_PANTHEON = { 3500,20,2,-4,5,50 };
const model_building MODEL_LIGHTHOUSE = { 1250,6,1,-1,4,20 };
const model_building MODEL_MESS_HALL = { 100,-8,1,2,4,10 };
const model_building MODEL_TAVERN = { 40,-2,1,1,6,8 };
const model_building MODEL_GRAND_GARDEN = { 400,0,0,0,0,0 };
const model_building MODEL_ARENA = { 500,-3,1,1,3,25 };
const model_building MODEL_COLOSSEUM = { 1500,-3,1,1,3,100 };
const model_building MODEL_HIPPODROME = { 3500,-3,1,1,3,150 };
const model_building MODEL_NULL = { 0,0,0,0,0 };
const model_building MODEL_LARARIUM = { 45, 4, 1, -1, 3, 0 };
const model_building MODEL_NYMPHAEUM = { 250,12,2,-1,6,0 };
const model_building MODEL_SMALL_MAUSOLEUM = { 300,-8,1,3,5,0 };
const model_building MODEL_LARGE_MAUSOLEUM = { 750,-10,1,3,6,0 };
const model_building MODEL_WATCHTOWER = { 100,-6,1,2,3,8, };
const model_building MODEL_CARAVANSERAI = { 500,-10,2,3,4,20 };
const model_building MODEL_PALISADE = { 6,0,0,0,0,0 };
const model_building MODEL_HIGHWAY = { 100,-4,1,2,3,0 };

const model_building *model_get_building(building_type type)
{
    switch (type) {
        case BUILDING_ROADBLOCK:
        case BUILDING_GARDEN_WALL_GATE:
        case BUILDING_HEDGE_GATE_DARK:
        case BUILDING_HEDGE_GATE_LIGHT:
            return &MODEL_ROADBLOCK;
        case BUILDING_WORKCAMP:
            return &MODEL_WORK_CAMP;
        case BUILDING_ARCHITECT_GUILD:
            return &MODEL_ARCHITECT_GUILD;
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
        case BUILDING_ARENA:
            return &MODEL_ARENA;
        case BUILDING_COLOSSEUM:
            return &MODEL_COLOSSEUM;
        case BUILDING_HIPPODROME:
            return &MODEL_HIPPODROME;
        case BUILDING_LARARIUM:
            return &MODEL_LARARIUM;
        case BUILDING_NYMPHAEUM:
            return &MODEL_NYMPHAEUM;
        case BUILDING_WATCHTOWER:
            return &MODEL_WATCHTOWER;
        case BUILDING_SMALL_MAUSOLEUM:
            return &MODEL_SMALL_MAUSOLEUM;
        case BUILDING_LARGE_MAUSOLEUM:
            return &MODEL_LARGE_MAUSOLEUM;
        case BUILDING_CARAVANSERAI:
            return &MODEL_CARAVANSERAI;
        case BUILDING_PALISADE:
        case BUILDING_PALISADE_GATE:
            return &MODEL_PALISADE;
        case BUILDING_HIGHWAY:
            return &MODEL_HIGHWAY;
        default:
            break;
    }

    if ((type >= BUILDING_PINE_TREE && type <= BUILDING_SMALL_STATUE_ALT_B) ||
        type == BUILDING_HEDGE_DARK || type == BUILDING_HEDGE_LIGHT ||
        type == BUILDING_DECORATIVE_COLUMN || type == BUILDING_GARDEN_WALL ||
        type == BUILDING_COLONNADE || type == BUILDING_GARDEN_WALL || 
        type == BUILDING_ROOFED_GARDEN_WALL || type == BUILDING_GARDEN_PATH ||
        type == BUILDING_GLADIATOR_STATUE) {
        return &buildings[41];
    }

    if (type == BUILDING_SMALL_POND || type == BUILDING_OBELISK ||
        type == BUILDING_LEGION_STATUE || type == BUILDING_DOLPHIN_FOUNTAIN) {
        return &buildings[42];
    }

    if (type == BUILDING_LARGE_POND || type == BUILDING_HORSE_STATUE) {
        return &buildings[43];
    }

    if (type > 129 || type < 0) {
        return &MODEL_NULL;
    } else {
        return &buildings[type];
    }
}

const model_house *model_get_house(house_level level)
{
    return &houses[level];
}

int model_house_uses_inventory(house_level level, inventory_type inventory)
{
    const model_house *house = model_get_house(level);
    switch (inventory) {
        case INVENTORY_WINE:
            return house->wine;
        case INVENTORY_OIL:
            return house->oil;
        case INVENTORY_FURNITURE:
            return house->furniture;
        case INVENTORY_POTTERY:
            return house->pottery;
        default:
            return 0;
    }
}

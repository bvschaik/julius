#include "warning.h"

#include "city/view.h"
#include "core/lang.h"
#include "core/string.h"
#include "core/time.h"
#include "game/settings.h"
#include "graphics/window.h"
#include "translation/translation.h"

#define MAX_WARNINGS 5
#define MAX_TEXT 100
#define TIMEOUT_MS 15000
#define TIMEOUT_FLASH 30

typedef struct {
    int in_use;
    time_millis time;
    int id;
    int flashing;
    uint8_t text[MAX_TEXT];
} warning;

static warning warnings[MAX_WARNINGS];
static int current_id;

static const uint8_t *get_text_for_warning(warning_type type)
{
    switch (type) {
        case WARNING_ORIENTATION:
            return lang_get_string(17, city_view_orientation());
        case WARNING_NO_MESS_HALL:
            return translation_for(TR_WARNING_NO_MESS_HALL);
        case WARNING_MAX_GRAND_TEMPLES:
            return translation_for(TR_WARNING_MAX_GRAND_TEMPLES);
        case WARNING_WATER_NEEDED_FOR_LIGHTHOUSE:
            return translation_for(TR_WARNING_WATER_NEEDED_FOR_LIGHTHOUSE);
        case WARNING_RESOURCES_NOT_AVAILABLE:
            return translation_for(TR_WARNING_RESOURCES_NOT_AVAILABLE);
        case WARNING_VARIANT_TOGGLE:
            return translation_for(TR_WARNING_VARIANT_TOGGLE);
        case WARNING_SECESSION:
            return translation_for(TR_CITY_WARNING_SECESSION);
        case WARNING_WAREHOUSE_BREAKIN:
            return translation_for(TR_CITY_WARNING_WAREHOUSE_BREAKIN);
        case WARNING_GRANARY_BREAKIN:
            return translation_for(TR_CITY_WARNING_GRANARY_BREAKIN);
        case WARNING_THEFT:
            return translation_for(TR_CITY_WARNING_THEFT);
        case WARNING_WOLF_NEARBY:
            return translation_for(TR_WARNING_NEARBY_WOLF);
        case WARNING_BET_VICTORY:
            return translation_for(TR_WARNING_BET_VICTORY);
        case WARNING_BET_DEFEAT:
            return translation_for(TR_WARNING_BET_DEFEAT);
        case WARNING_DATA_COPY_SUCCESS:
            return translation_for(TR_CITY_WARNING_DATA_COPY_SUCCESS);
        case WARNING_DATA_COPY_NOT_SUPPORTED:
            return translation_for(TR_CITY_WARNING_DATA_COPY_NOT_SUPPORTED);
        case WARNING_DATA_PASTE_FAILURE:
            return translation_for(TR_CITY_WARNING_DATA_PASTE_NOT_SUPPORTED);
        case WARNING_DATA_PASTE_SUCCESS:
            return translation_for(TR_CITY_WARNING_DATA_PASTE_SUCCESS);
        case WARNING_DATA_MOTHBALL_ON:
            return translation_for(TR_CITY_WARNING_DATA_MOTHBALL_ON);
        case WARNING_DATA_MOTHBALL_OFF:
            return translation_for(TR_CITY_WARNING_DATA_MOTHBALL_OFF);
        case WARNING_GOLD_NEEDED:
            return translation_for(TR_CITY_WARNING_GOLD_NEEDED);
        case WARNING_BUILD_GOLD_MINE:
            return translation_for(TR_CITY_WARNING_BUILD_GOLD_MINE);
        case WARNING_SENATE_NEEDED:
            return translation_for(TR_CITY_WARNING_SENATE_NEEDED);
        case WARNING_BUILD_SENATE:
            return translation_for(TR_CITY_WARNING_BUILD_SENATE);
        default:
           return lang_get_string(19, type - 2);
    }
}

int city_warning_show(warning_type type, int id)
{
    return city_warning_show_custom(get_text_for_warning(type), id);
}

static warning *get_warning_slot(int id, const uint8_t *text)
{
    if (id != 0) {
        for (int i = 0; i < MAX_WARNINGS; i++) {
            if (warnings[i].id == id && warnings[i].in_use) {
                return &warnings[i];
            }
        }
    }
    for (int i = 0; i < MAX_WARNINGS; i++) {
        if (warnings[i].in_use && string_equals(warnings[i].text, text)) {
            if (warnings[i].time != time_get_millis()) {
                warnings[i].flashing = 1;
            }
            return &warnings[i];
        }
    }
    for (int i = 0; i < MAX_WARNINGS; i++) {
        if (!warnings[i].in_use) {
            warnings[i].id = ++current_id;
            warnings[i].flashing = 0;
            return &warnings[i];
        }
    }
    return 0;
}

int city_warning_show_custom(const uint8_t *text, int id)
{
    if (!setting_warnings()) {
        return 0;
    }
    warning *w = get_warning_slot(id, text);
    if (!w) {
        return 0;
    }
    w->in_use = 1;
    w->time = time_get_millis();
    if (!w->flashing) {
        string_copy(text, w->text, MAX_TEXT);
    }
    return w->id;
}

int city_has_warnings(void)
{
    for (int i = 0; i < MAX_WARNINGS; i++) {
        if (warnings[i].in_use) {
            return 1;
        }
    }
    return 0;
}

const uint8_t *city_warning_get(int position)
{
    if (warnings[position].in_use) {
        if (warnings[position].flashing) {
            if (time_get_millis() - warnings[position].time > TIMEOUT_FLASH) {
                warnings[position].flashing = 0;
                window_request_refresh();
            }
        } else {
            return warnings[position].text;
        }
    }
    return 0;
}

void city_warning_clear_id(int id)
{
    for (int i = 0; i < MAX_WARNINGS; i++) {
        if (warnings[i].id == id) {
            warnings[i].in_use = 0;
            return;
        }
    }
}

void city_warning_clear_all(void)
{
    for (int i = 0; i < MAX_WARNINGS; i++) {
        warnings[i].in_use = 0;
    }
}

void city_warning_clear_outdated(void)
{
    for (int i = 0; i < MAX_WARNINGS; i++) {
        if (warnings[i].in_use && time_get_millis() - warnings[i].time > TIMEOUT_MS) {
            warnings[i].in_use = 0;
            window_request_refresh();
        }
    }
}

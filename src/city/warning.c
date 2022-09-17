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

int city_warning_show(warning_type type, int id)
{
    const uint8_t *text;
    if (type == WARNING_ORIENTATION) {
        text = lang_get_string(17, city_view_orientation());
    } else if (type == WARNING_NO_MESS_HALL) {
        text = translation_for(TR_WARNING_NO_MESS_HALL);
    } else if (type == WARNING_MAX_GRAND_TEMPLES) {
        text = translation_for(TR_WARNING_MAX_GRAND_TEMPLES);
    } else if (type == WARNING_WATER_NEEDED_FOR_LIGHTHOUSE) {
        text = translation_for(TR_WARNING_WATER_NEEDED_FOR_LIGHTHOUSE);
    } else if (type == WARNING_RESOURCES_NOT_AVAILABLE) {
        text = translation_for(TR_WARNING_RESOURCES_NOT_AVAILABLE);
    } else if (type == WARNING_VARIANT_TOGGLE) {
        text = translation_for(TR_WARNING_VARIANT_TOGGLE);
    } else if (type == WARNING_SECESSION) {
        text = translation_for(TR_CITY_WARNING_SECESSION);
    } else if (type == WARNING_WAREHOUSE_BREAKIN) {
        text = translation_for(TR_CITY_WARNING_WAREHOUSE_BREAKIN);
    } else if (type == WARNING_GRANARY_BREAKIN) {
        text = translation_for(TR_CITY_WARNING_GRANARY_BREAKIN);
    } else if (type == WARNING_THEFT) {
        text = translation_for(TR_CITY_WARNING_THEFT);
    } else if (type == WARNING_WOLF_NEARBY) {
        text = translation_for(TR_WARNING_NEARBY_WOLF);
    } else if (type == WARNING_BET_VICTORY) {
        text = translation_for(TR_WARNING_BET_VICTORY);
    } else if (type == WARNING_BET_DEFEAT) {
        text = translation_for(TR_WARNING_BET_DEFEAT);
    } else if (type == WARNING_DATA_COPY_SUCCESS) {
        text = translation_for(TR_CITY_WARNING_DATA_COPY_SUCCESS);
    } else if (type == WARNING_DATA_COPY_NOT_SUPPORTED) {
        text = translation_for(TR_CITY_WARNING_DATA_COPY_NOT_SUPPORTED);
    } else if (type == WARNING_DATA_PASTE_FAILURE) {
        text = translation_for(TR_CITY_WARNING_DATA_PASTE_NOT_SUPPORTED);
    } else if (type == WARNING_DATA_PASTE_SUCCESS) {
        text = translation_for(TR_CITY_WARNING_DATA_PASTE_SUCCESS);
    } else if (type == WARNING_DATA_MOTHBALL_ON) {
        text = translation_for(TR_CITY_WARNING_DATA_MOTHBALL_ON);
    } else if (type == WARNING_DATA_MOTHBALL_OFF) {
        text = translation_for(TR_CITY_WARNING_DATA_MOTHBALL_OFF);
    } else {
        text = lang_get_string(19, type - 2);
    }
    return city_warning_show_custom(text, id);
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

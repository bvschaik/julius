#include "warning.h"

#include "city/view.h"
#include "core/lang.h"
#include "core/string.h"
#include "core/time.h"
#include "game/settings.h"
#include "graphics/window.h"

#define MAX_WARNINGS 5
#define MAX_TEXT 100
#define TIMEOUT_MS 15000

struct warning {
    int in_use;
    time_millis time;
    uint8_t text[MAX_TEXT];
};

static struct warning warnings[MAX_WARNINGS];

static struct warning *new_warning(void)
{
    for (int i = 0; i < MAX_WARNINGS; i++) {
        if (!warnings[i].in_use) {
            return &warnings[i];
        }
    }
    return 0;
}

void city_warning_show(warning_type type)
{
    if (!setting_warnings()) {
        return;
    }
    struct warning *w = new_warning();
    if (!w) {
        return;
    }
    w->in_use = 1;
    w->time = time_get_millis();
    const uint8_t *text;
    if (type == WARNING_ORIENTATION) {
        text = lang_get_string(17, city_view_orientation());
    } else {
        text = lang_get_string(19, type - 2);
    }
    string_copy(text, w->text, MAX_TEXT);
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

const uint8_t *city_warning_get(int id)
{
    if (warnings[id].in_use) {
        return warnings[id].text;
    }
    return 0;
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

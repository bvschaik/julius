#include "editor/editor.h"

#include "core/file.h"

#define MAX_EDITOR_FILES 9

static const char EDITOR_FILES[MAX_EDITOR_FILES][32] = {
    "c3_map.eng",
    "c3_map_mm.eng",
    "c3map.sg2",
    "c3map.555",
    "c3map_north.sg2",
    "c3map_north.555",
    "c3map_south.sg2",
    "c3map_south.555",
    "map_panels.555"
};

static int is_active;

int editor_is_present(void)
{
    for (int i = 0; i < MAX_EDITOR_FILES; i++) {
        if (!file_exists(EDITOR_FILES[i], MAY_BE_LOCALIZED)) {
            return 0;
        }
    }
    return 1;
}

void editor_set_active(int active)
{
    is_active = active;
}

int editor_is_active(void)
{
    return is_active;
}

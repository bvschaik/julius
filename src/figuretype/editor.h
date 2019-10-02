#ifndef FIGURETYPE_EDITOR_H
#define FIGURETYPE_EDITOR_H

#include "figure/figure.h"

enum {
    MAP_FLAG_EARTHQUAKE = 1,
    MAP_FLAG_ENTRY = 2,
    MAP_FLAG_EXIT = 3,
    MAP_FLAG_RIVER_ENTRY = 12,
    MAP_FLAG_RIVER_EXIT = 13,
    MAP_FLAG_INVASION_MIN = 4,
    MAP_FLAG_INVASION_MAX = 12,
    MAP_FLAG_FISHING_MIN = 14,
    MAP_FLAG_FISHING_MAX = 22,
    MAP_FLAG_HERD_MIN = 22,
    MAP_FLAG_HERD_MAX = 26,

    MAP_FLAG_MIN = 1,
    MAP_FLAG_MAX = 26,
};

void figure_create_editor_flags(void);

void figure_editor_flag_action(figure *f);

#endif // FIGURETYPE_EDITOR_H

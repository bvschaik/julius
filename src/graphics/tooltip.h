#ifndef GRAPHICS_TOOLTIP_H
#define GRAPHICS_TOOLTIP_H

#include "input/mouse.h"

#include <stdint.h>

#define TOOLTIP_MAX_EXTRA_VALUES 5

typedef enum {
    TOOLTIP_NONE = 0,
    TOOLTIP_BUTTON = 1,
    TOOLTIP_OVERLAY = 2,
    TOOLTIP_SENATE = 3,
    TOOLTIP_TILES = 4
} tooltip_type;

typedef enum {
    TOOLTIP_EXTRA_TEXT_COMMA_SEPARATED = 0,
    TOOLTIP_EXTRA_TEXT_JOINED_BY_SPACE = 1
} tooltip_extra_text_type;

typedef struct {
    const int mouse_x;
    const int mouse_y;
    tooltip_type type;
    int high_priority;
    int text_group;
    int text_id;
    int has_numeric_prefix;
    int numeric_prefix;
    int num_extra_values;
    int extra_value_text_groups[TOOLTIP_MAX_EXTRA_VALUES];
    int extra_value_text_ids[TOOLTIP_MAX_EXTRA_VALUES];
    int translation_key;
    int num_extra_texts;
    tooltip_extra_text_type extra_text_type;
    int extra_text_groups[TOOLTIP_MAX_EXTRA_VALUES];
    int extra_text_ids[TOOLTIP_MAX_EXTRA_VALUES];
    const uint8_t *precomposed_text;
} tooltip_context;

void tooltip_invalidate(void);
void tooltip_handle(const mouse *m, void (*func)(tooltip_context *));

#endif // GRAPHICS_TOOLTIP_H

#ifndef GRAPHICS_TOOLTIP_H
#define GRAPHICS_TOOLTIP_H

#include "input/mouse.h"

typedef enum {
    TOOLTIP_NONE = 0,
    TOOLTIP_BUTTON = 1,
    TOOLTIP_OVERLAY = 2,
    TOOLTIP_SENATE = 3
} tooltip_type;

typedef struct {
    const int mouse_x;
    const int mouse_y;
    tooltip_type type;
    int high_priority;
    int text_group;
    int text_id;
    int has_numeric_prefix;
    int numeric_prefix;
} tooltip_context;

void tooltip_invalidate(void);
void tooltip_handle(const mouse *m, void (*func)(tooltip_context *));

#endif // GRAPHICS_TOOLTIP_H

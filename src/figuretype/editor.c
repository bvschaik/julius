#include "editor.h"

#include "figure/figure.h"

void figure_create_editor_flags(void)
{
    for (int id = 1; id <= 25; id++) {
        figure_create(FIGURE_MAP_FLAG, -1, -1, 0)->resource_id = id;
    }
}

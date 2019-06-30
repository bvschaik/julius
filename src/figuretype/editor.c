#include "editor.h"

#include "figure/figure.h"

static void create_flag(int id)
{
    figure *f = figure_create(FIGURE_MAP_FLAG, -1, -1, 0);
    if (f) {
        f->resource_id = id;
    }
}

void figure_create_editor_flags(void)
{
    for (int id = 1; id <= 25; id++) {
        create_flag(id);
    }
}

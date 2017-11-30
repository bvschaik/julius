#ifndef FIGURE_FIGURE_H
#define FIGURE_FIGURE_H

#include "core/buffer.h"
#include "core/direction.h"
#include "figure/type.h"

#include "Data/Figure.h"

figure *figure_get(int id);

/**
 * Creates a figure
 * @param type Figure type
 * @param x X position
 * @param y Y position
 * @param dir Direction the figure faces
 * @return Always a figure. If figure->id is zero, it is an invalid one.
 */
figure *figure_create(figure_type type, int x, int y, direction dir);

void figure_init_scenario();

void figure_save_state(buffer *list, buffer *seq);

void figure_load_state(buffer *list, buffer *seq);

#endif // FIGURE_FIGURE_H

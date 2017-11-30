#ifndef FIGURE_FIGURE_H
#define FIGURE_FIGURE_H

#include "core/buffer.h"

#include "Data/Figure.h"

figure *figure_get(int id);

void figure_init_scenario();

void figure_save_state(buffer *list, buffer *seq);

void figure_load_state(buffer *list, buffer *seq);

#endif // FIGURE_FIGURE_H

#ifndef FIGURE_FIGURE_H
#define FIGURE_FIGURE_H

#include "core/buffer.h"

#include "Data/Figure.h"

struct Data_Figure *figure_get(int id);

void figure_save_state(buffer *list, buffer *seq);

void figure_load_state(buffer *list, buffer *seq);

#endif // FIGURE_FIGURE_H

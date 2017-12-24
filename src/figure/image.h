#ifndef FIGURE_IMAGE_H
#define FIGURE_IMAGE_H

#include "figure/figure.h"

void figure_image_update(figure *f, int image_base);

void figure_image_increase_offset(figure *f, int max);

int figure_image_corpse_offset(figure *f);

int figure_image_direction(figure *f);

#endif // FIGURE_IMAGE_H

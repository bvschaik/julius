#include "image.h"

#include "../FigureAction_private.h"

static const int CORPSE_IMAGE_OFFSETS[128] = {
    0, 1, 2, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
};

void figure_image_update(figure *f, int image_base)
{
    if (f->actionState == FigureActionState_149_Corpse) {
        f->graphicId = image_base + CORPSE_IMAGE_OFFSETS[f->waitTicks / 2] + 96;
    } else {
        f->graphicId = image_base + FigureActionDirection(f) + 8 * f->graphicOffset;
    }
}

void figure_image_increase_offset(figure *f, int max)
{
    f->graphicOffset++;
    if (f->graphicOffset >= (max)) {
        f->graphicOffset = 0;
    }
}

int figure_image_corpse_offset(figure *f)
{
    return CORPSE_IMAGE_OFFSETS[f->waitTicks / 2];
}


#ifndef WINDOW_ADVISOR_RATINGS_H
#define WINDOW_ADVISOR_RATINGS_H

#include "window/advisors.h"

const advisor_window_type *window_advisor_ratings(void);

void draw_rating_column(int x_offset, int y_offset, int value, int has_reached);


#endif // WINDOW_ADVISOR_RATINGS_H

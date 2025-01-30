#ifndef WINDOW_NUMERIC_INPUT_H
#define WINDOW_NUMERIC_INPUT_H

#include "graphics/generic_button.h"

void window_numeric_input_show(int x, int y, const generic_button *button, int max_digits,
    int max_value, void (*callback)(int));

void window_numeric_input_bound_show(int x, int y, const generic_button *button, int max_digits,
    int min_value, int max_value, void (*callback)(int));

#endif // WINDOW_NUMERIC_INPUT_H

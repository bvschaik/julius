#ifndef WINDOW_NUMERIC_INPUT_H
#define WINDOW_NUMERIC_INPUT_H

void window_numeric_input_show(int x, int y, int max_digits, int max_value, void (*callback)(int));

void window_numeric_input_bound_show(int x, int y, int max_digits, int min_value, int max_value, void (*callback)(int));

void window_numeric_input_accept(void);

#endif // WINDOW_NUMERIC_INPUT_H

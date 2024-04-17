#ifndef WINDOW_TEXT_INPUT_H
#define WINDOW_TEXT_INPUT_H

#include <stdint.h>

void window_text_input_show(const uint8_t *title, const uint8_t *placeholder, const uint8_t *text, int max_length,
    void (*callback)(const uint8_t *));

#endif // WINDOW_TEXT_INPUT_H

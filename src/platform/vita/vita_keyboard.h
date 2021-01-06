#ifndef VITA_KEYBOARD_H
#define VITA_KEYBOARD_H

#include <stdint.h>

const uint8_t *vita_keyboard_get(const uint8_t *initial_text, int max_length);

#endif /* VITA_KEYBOARD_H */

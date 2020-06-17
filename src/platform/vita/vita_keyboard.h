#ifndef VITA_KEYBOARD_H
#define VITA_KEYBOARD_H

#include <stdint.h>

char *vita_keyboard_get(const uint8_t *title, const uint8_t *initial_text, int max_len);

#endif /* VITA_KEYBOARD_H */

#ifndef WINDOW_PLAIN_MESSAGE_DIALOG_H
#define WINDOW_PLAIN_MESSAGE_DIALOG_H

#include "translation/translation.h"

void window_plain_message_dialog_show(translation_key title, translation_key message, int should_draw_underlying_window);
void window_plain_message_dialog_show_with_extra(translation_key title, translation_key message, const uint8_t *extra, const uint8_t *extra2);

#endif // WINDOW_PLAIN_MESSAGE_DIALOG_H

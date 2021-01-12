#ifndef WINDOW_PLAIN_MESSAGE_DIALOG_H
#define WINDOW_PLAIN_MESSAGE_DIALOG_H

#include "translation/translation.h"

void window_plain_message_dialog_show(translation_key title, translation_key message);
void window_plain_message_dialog_show_with_extra(translation_key title, translation_key message, const uint8_t *extra);

#endif // WINDOW_PLAIN_MESSAGE_DIALOG_H

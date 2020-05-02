#ifndef WINDOW_HOTKEY_EDITOR_H
#define WINDOW_HOTKEY_EDITOR_H

#include "core/hotkey_config.h"
#include "input/keys.h"

void window_hotkey_editor_show(hotkey_action action, int index,
    void (*callback)(hotkey_action, int, key_type, key_modifier_type));

void window_hotkey_editor_key_pressed(key_type key, key_modifier_type modifiers);
void window_hotkey_editor_key_released(key_type key, key_modifier_type modifiers);

#endif // WINDOW_HOTKEY_EDITOR_H

#include "input/hotkey.h"
#include "input/keys.h"
#include "input/mouse.h"
#include "input/scroll.h"

void hotkey_install_mapping(hotkey_mapping *mappings, int num_mappings)
{
}

const char *key_combination_name(key_type key, key_modifier_type modifiers)
{
    return "";
}

int key_combination_from_name(const char *name, key_type *key, key_modifier_type *modifiers)
{
    return 0;
}

key_type system_keyboard_key_for_symbol(const char *name)
{
    return KEY_NONE;
}

void mouse_reset_up_state(void)
{
}

int scroll_in_progress(void)
{
    return 0;
}

int scroll_is_smooth(void)
{
    return 0;
}

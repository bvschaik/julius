#include "keys.h"

#include <string.h>

static const char *key_names[KEY_MAX_ITEMS] = {
    "", "A", "B", "C", "D", "E", "F", "G", "H", "I",
    "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S",
    "T", "U", "V", "W", "X", "Y", "Z", "1", "2", "3",
    "4", "5", "6", "7", "8", "9", "0", "-", "=", "Enter",
    "Esc", "Backspace", "Tab", "Space", "[", "]", "\\", ";", "'", "`",
    ",", ".", "/", "F1", "F2", "F3", "F4", "F5", "F6", "F7",
    "F8", "F9", "F10", "F11", "F12", "Insert", "Delete", "Home", "End", "PageUp",
    "PageDown", "Right", "Left", "Down", "Up",
};

typedef struct {
    key_modifier_type modifier;
    const char *name;
} modifier_name;

static const modifier_name modifier_names[] = {
    { KEY_MOD_CTRL, "Ctrl" },
    { KEY_MOD_ALT, "Alt" },
    { KEY_MOD_GUI, "Gui" },
    { KEY_MOD_SHIFT, "Shift" },
    { KEY_MOD_NONE }
};

const char *key_combination_name(key_type key, key_modifier_type modifiers)
{
    static char name[100];
    name[0] = 0;
    for (const modifier_name *modname = modifier_names; modname->modifier; modname++) {
        if (modifiers & modname->modifier) {
            strcat(name, modname->name);
            strcat(name, " ");
        }
    }
    strcat(name, key_names[key]);
    return name;
}

static key_modifier_type parse_modifier(const char *name)
{
    for (const modifier_name *modname = modifier_names; modname->modifier; modname++) {
        if (strcmp(modname->name, name) == 0) {
            return modname->modifier;
        }
    }
    return KEY_MOD_NONE;
}

static key_type parse_key(const char *name)
{
    for (int i = 1; i < KEY_MAX_ITEMS; i++) {
        if (strcmp(key_names[i], name) == 0) {
            return i;
        }
    }
    return KEY_NONE;
}

int key_combination_from_name(const char *name, key_type *key, key_modifier_type *modifiers)
{
    char editable_name[100] = {0};
    strncpy(editable_name, name, 99);

    *key = KEY_NONE;
    *modifiers = KEY_MOD_NONE;

    char *token = strtok(editable_name, " ");
    while (token) {
        if (token[0]) {
            key_modifier_type mod = parse_modifier(token);
            if (mod != KEY_MOD_NONE) {
                *modifiers |= mod;
            } else {
                *key = parse_key(token);
                if (*key == KEY_NONE) {
                    return 0;
                }
            }
        }
        token = strtok(0, " ");
    }
    if (*key == KEY_NONE) {
        return 0;
    }
    return 1;
}

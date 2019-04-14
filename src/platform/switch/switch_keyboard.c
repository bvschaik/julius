#include <string.h>
#include <stdbool.h>

#include <switch.h>
#include "switch_keyboard.h"

void switch_keyboard_get(char *title, const char *initial_text, int max_len, int multiline, char *buf)
{
    Result rc = 0;

    SwkbdConfig kbd;

    rc = swkbdCreate(&kbd, 0);

    if (R_SUCCEEDED(rc)) {
        swkbdConfigMakePresetDefault(&kbd);
        swkbdConfigSetInitialText(&kbd, initial_text);
        rc = swkbdShow(&kbd, buf, max_len);
        swkbdClose(&kbd);
    }
}

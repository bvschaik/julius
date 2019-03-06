#include <vitasdk.h>
#include "input/keyboard.h"
#include "input/mouse.h"
#include "input/hotkey.h"


#define TOUCH_HEIGHT 1087
#define TOUCH_WIDTH 1919

#define SCREEN_HEIGHT 544
#define SCREEN_WIDTH 960

#define BUTTON_PRESSED(x) (buttons.buttons & x)
#define BUTTON_OLD_PRESSED(x) (old_buttons.buttons & x)

SceCtrlData old_buttons = {0};

void vita_handle_input() {
    struct SceTouchData touch = {0};
    sceTouchPeek(SCE_TOUCH_PORT_FRONT, &touch, 1);

    struct SceCtrlData buttons = {0};
    sceCtrlPeekBufferPositive(0, &buttons, 1);

    if (BUTTON_PRESSED(SCE_CTRL_CROSS)) {
        if (!BUTTON_OLD_PRESSED(SCE_CTRL_CROSS)) {
            mouse_set_left_down(1);
        }
    } else if (BUTTON_OLD_PRESSED(SCE_CTRL_CROSS)) {
        mouse_set_left_down(0);
    }

    if (BUTTON_PRESSED(SCE_CTRL_CIRCLE)) {
        if (!BUTTON_OLD_PRESSED(SCE_CTRL_CIRCLE)) {
            mouse_set_right_down(1);
        }
    } else if (BUTTON_OLD_PRESSED(SCE_CTRL_CIRCLE)) {
        mouse_set_right_down(0);
    }

    if (BUTTON_PRESSED(SCE_CTRL_LEFT)) {
        keyboard_left();
        hotkey_left();
    }

    if (BUTTON_PRESSED(SCE_CTRL_RIGHT)) {
        keyboard_right();
        hotkey_right();
    }

    if (BUTTON_PRESSED(SCE_CTRL_UP)) {
        keyboard_left();
        hotkey_up();
    }

    if (BUTTON_PRESSED(SCE_CTRL_DOWN)) {
        keyboard_right();
        hotkey_down();
    }

    if (BUTTON_PRESSED(SCE_CTRL_LTRIGGER)) {
        hotkey_page_down();
    }

    if (BUTTON_PRESSED(SCE_CTRL_RTRIGGER)) {
        hotkey_page_up();
    }

    if (touch.reportNum > 0) {
        int x = touch.report[0].x*SCREEN_WIDTH/TOUCH_WIDTH;
        int y = touch.report[0].y*SCREEN_HEIGHT/TOUCH_HEIGHT;
        mouse_set_position(x, y);
    }

    memcpy(&old_buttons, &buttons, sizeof(struct SceCtrlData));
}
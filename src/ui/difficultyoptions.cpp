
#include "window.h"
#include "widget.h"
#include "graphics.h"

#include <data>
#include <ui>

#include "game/settings.h"

#include "graphics/mouse.h"

static void arrowButtonDifficulty(int param1, int param2);
static void arrowButtonGods(int param1, int param2);

static ArrowButton arrowButtons[] =
{
    {0, 54, 15, 24, arrowButtonDifficulty, 0, 0},
    {24, 54, 17, 24, arrowButtonDifficulty, 1, 0},
    {0, 102, 21, 24, arrowButtonGods, 2, 0}
};

void UI_DifficultyOptions_drawForeground()
{
    int baseOffsetX = Data_Screen.offset640x480.x;
    int baseOffsetY = Data_Screen.offset640x480.y;

    Widget::Panel::drawOuterPanel(
        baseOffsetX + 48, baseOffsetY + 80,
        24, 12
    );

    Widget_GameText_drawCentered(153, 0,
                                 baseOffsetX + 48, baseOffsetY + 94,
                                 384, FONT_LARGE_BLACK
                                );

    Widget_GameText_drawCentered(153, setting_difficulty() + 1,
                                 baseOffsetX + 80, baseOffsetY + 142,
                                 224, FONT_NORMAL_BLACK
                                );
    Widget_GameText_drawCentered(
        153, setting_gods_enabled() ? 7 : 6,
        baseOffsetX + 80, baseOffsetY + 190,
        224, FONT_NORMAL_BLACK
    );
    Widget::Button::drawArrowButtons(
        baseOffsetX + 288, baseOffsetY + 80,
        arrowButtons, 3
    );
    Widget_GameText_drawCentered(153, 8,
                                 baseOffsetX + 48, baseOffsetY + 246,
                                 384, FONT_NORMAL_BLACK
                                );
}

void UI_DifficultyOptions_handleMouse(const mouse *m)
{
    if (m->right.went_up)
    {
        // cancel dialog
        UI_Window_goTo(Window_City);
    }
    else
    {
        int baseOffsetX = Data_Screen.offset640x480.x;
        int baseOffsetY = Data_Screen.offset640x480.y;
        Widget::Button::handleArrowButtons(
            baseOffsetX + 288, baseOffsetY + 80, arrowButtons, 4);
    }
}

static void arrowButtonDifficulty(int param1, int param2)
{
    if (param1)
    {
        setting_decrease_difficulty();
    }
    else
    {
        setting_increase_difficulty();
    }
    UI_Window_requestRefresh();
}

static void arrowButtonGods(int param1, int param2)
{
    setting_toggle_gods_enabled();
    UI_Window_requestRefresh();
}

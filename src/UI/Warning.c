#include "Warning.h"
#include "Window.h"

#include "../Data/Screen.h"

#include "../Graphics.h"

#include "city/warning.h"
#include "game/state.h"
#include "graphics/panel.h"
#include "graphics/text.h"

static const int TOP_OFFSETS[] = {30, 55, 80, 105, 130};

static int determine_width(const uint8_t *text)
{
    int width = Widget_Text_getWidth(text, FONT_NORMAL_BLACK);
    if (width <= 100) {
        return 200;
    } else if (width <= 200) {
        return 300;
    } else if (width <= 300) {
        return 400;
    } else {
        return 460;
    }
}

void UI_Warning_draw()
{
    if (UI_Window_getId() != Window_City) {
        city_warning_clear_all();
        return;
    }

    int center = (Data_Screen.width - 180) / 2;
    for (int i = 0; i < 5; i++) {
        const uint8_t *text = city_warning_get(i);
        if (!text) {
            continue;
        }
        int top_offset = TOP_OFFSETS[i];
        if (game_state_is_paused()) {
            top_offset += 70;
        }
        int box_width = determine_width(text);
        label_draw(
            center - box_width / 2 + 1, top_offset,
            box_width / 16 + 1, 1);
        if (box_width < 460) {
            // ornaments at the side
            Graphics_drawImage(image_group(GROUP_CONTEXT_ICONS) + 15,
                center - box_width / 2 + 2, top_offset + 2);
            Graphics_drawImage(image_group(GROUP_CONTEXT_ICONS) + 15,
                center + box_width / 2 - 30, top_offset + 2);
        }
        Widget_Text_drawCentered(text,
            center - box_width / 2 + 1, top_offset + 4,
            box_width, FONT_NORMAL_WHITE, 0);
    }
}

#include "AllWindows.h"

#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "sound/music.h"

void UI_Logo_init()
{
	sound_music_play_intro();
}

void UI_Logo_drawBackground()
{
	graphics_clear_screen();

    graphics_in_dialog();
	image_draw(image_group(GROUP_LOGO), 0, 0);
	lang_text_draw_centered_colored(13, 7, 160, 462, 320, FONT_NORMAL_PLAIN, COLOR_WHITE);
    graphics_reset_dialog();
}

void UI_Logo_handleMouse(const mouse *m)
{
	if (m->left.went_up || m->right.went_up) {
		UI_Window_goTo(Window_MainMenu);
	}
}

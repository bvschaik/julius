#include "PopupDialog.h"

#include "graphics/graphics.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/window.h"

#define GROUP 5

static void buttonOk(int param1, int param2);
static void buttonCancel(int param1, int param2);

static image_button buttons[] = {
	{192, 100, 34, 34, IB_NORMAL, 96, 0, buttonOk, button_none, 1, 0, 1},
	{256, 100, 34, 34, IB_NORMAL, 96, 4, buttonCancel, button_none, 0, 0, 1},
};

static struct {
	int msgId;
	int okClicked;
	void (*closeFunc)(int accepted);
	int hasButtons;
} data;

void UI_PopupDialog_show(int msgId, void (*closeFunc)(int accepted), int hasOkCancelButtons)
{
	if (window_is(Window_PopupDialog)) {
		// don't show popup over popup
		return;
	}
	data.msgId = msgId;
	data.okClicked = 0;
	data.closeFunc = closeFunc;
	data.hasButtons = hasOkCancelButtons;
	UI_Window_goTo(Window_PopupDialog);
}

void UI_PopupDialog_drawBackground()
{
    graphics_in_dialog();
	outer_panel_draw(80, 80, 30, 10);
	lang_text_draw_centered(GROUP, data.msgId, 80, 100, 480, FONT_LARGE_BLACK);
	if (lang_text_get_width(GROUP, data.msgId + 1, FONT_NORMAL_BLACK) >= 420) {
		lang_text_draw_multiline(GROUP, data.msgId + 1, 110, 140, 420, FONT_NORMAL_BLACK);
	} else {
		lang_text_draw_centered(GROUP, data.msgId + 1, 80, 140, 480, FONT_NORMAL_BLACK);
	}
	graphics_reset_dialog();
}

void UI_PopupDialog_drawForeground()
{
    graphics_in_dialog();
	if (data.hasButtons) {
		image_buttons_draw(80, 80, buttons, 2);
	} else {
		lang_text_draw_centered(13, 1, 80, 208, 480, FONT_NORMAL_BLACK);
	}
	graphics_reset_dialog();
}

void UI_PopupDialog_handleMouse(const mouse *m)
{
	if (data.hasButtons) {
		image_buttons_handle_mouse(mouse_in_dialog(m), 80, 80, buttons, 2, 0);
	} else if (m->right.went_up) {
		data.closeFunc(0);
		UI_Window_goBack();
	}
}

void buttonOk(int param1, int param2)
{
	UI_Window_goBack();
	data.closeFunc(1);
}

void buttonCancel(int param1, int param2)
{
	UI_Window_goBack();
	data.closeFunc(0);
}

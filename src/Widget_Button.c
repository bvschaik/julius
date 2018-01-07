#include "Widget.h"
#include "Graphics.h"

#include "sound/effect.h"

#define PRESSED_EFFECT_MILLIS 100
#define PRESSED_REPEAT_INITIAL_MILLIS 300
#define PRESSED_REPEAT_MILLIS 50

void Widget_Button_doNothing(int param1, int param2)
{
}

static void imageButtonFadePressedEffect(ImageButton *buttons, int numButtons)
{
	time_millis currentTime = time_get_millis();
	for (int i = 0; i < numButtons; i++) {
		ImageButton *btn = &buttons[i];
		if (btn->pressed) {
			if (btn->buttonType == ImageButton_Normal) {
				if (btn->pressedSince + PRESSED_EFFECT_MILLIS < currentTime) {
					btn->pressed = 0;
				}
			} else if (btn->buttonType == ImageButton_Scroll) {
				if (btn->pressedSince + PRESSED_EFFECT_MILLIS < currentTime && !mouse_get()->left.is_down) {
					btn->pressed = 0;
				}
			}
		}
	}
}

static void imageButtonRemovePressedEffectBuild(ImageButton *buttons, int numButtons)
{
	for (int i = 0; i < numButtons; i++) {
		ImageButton *btn = &buttons[i];
		if (btn->pressed && btn->buttonType == ImageButton_Build) {
			btn->pressed = 0;
		}
	}
}

void Widget_Button_drawImageButtons(int xOffset, int yOffset, ImageButton *buttons, int numButtons)
{
	imageButtonFadePressedEffect(buttons, numButtons);
	for (int i = 0; i < numButtons; i++) {
		ImageButton *btn = &buttons[i];
		int graphicId = image_group(btn->graphicCollection) + btn->graphicIdOffset;
		if (btn->enabled) {
			if (btn->pressed) {
				graphicId += 2;
			} else if (btn->focused) {
				graphicId += 1;
			}
		} else {
			graphicId += 3;
		}
		Graphics_drawImage(graphicId, xOffset + btn->xOffset, yOffset + btn->yOffset);
	}
}

int Widget_Button_handleImageButtons(const mouse *m, ImageButton *buttons, int numButtons, int *focusButtonId)
{
	imageButtonFadePressedEffect(buttons, numButtons);
	imageButtonRemovePressedEffectBuild(buttons, numButtons);
	ImageButton *hitButton = 0;
	if (focusButtonId) {
		*focusButtonId = 0;
	}
	for (int i = 0; i < numButtons; i++) {
		ImageButton *btn = &buttons[i];
		if (btn->focused) {
			btn->focused--;
		}
		if (btn->xOffset <= m->x &&
			btn->xOffset + btn->width > m->x &&
			btn->yOffset <= m->y &&
			btn->yOffset + btn->height > m->y) {
			if (focusButtonId) {
				*focusButtonId = i + 1;
			}
			if (btn->enabled) {
				btn->focused = 2;
				hitButton = btn;
			}
		}
	}
	if (!hitButton) {
		return 0;
	}
	if (hitButton->buttonType == ImageButton_Scroll) {
		if (!m->left.went_down && !m->left.is_down) {
			return 0;
		}
	} else if (hitButton->buttonType == ImageButton_Build || hitButton->buttonType == ImageButton_Normal) {
		if (!m->left.went_down && !m->right.went_down) {
			return 0;
		}
	}
	if (m->left.went_down) {
		sound_effect_play(SOUND_EFFECT_ICON);
		hitButton->pressed = 1;
		hitButton->pressedSince = time_get_millis();
		hitButton->leftClickHandler(hitButton->parameter1, hitButton->parameter2);
	} else if (m->right.went_up) {
		hitButton->pressed = 1;
		hitButton->pressedSince = time_get_millis();
		hitButton->rightClickHandler(hitButton->parameter1, hitButton->parameter2);
	} else if (hitButton->buttonType == ImageButton_Scroll && m->left.is_down) {
		time_millis delay = hitButton->pressed == 2 ? PRESSED_REPEAT_MILLIS : PRESSED_REPEAT_INITIAL_MILLIS;
		if (hitButton->pressedSince + delay <= time_get_millis()) {
			hitButton->pressed = 2;
			hitButton->pressedSince = time_get_millis();
			hitButton->leftClickHandler(hitButton->parameter1, hitButton->parameter2);
 		}
	}
	return 1;
}

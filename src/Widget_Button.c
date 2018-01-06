#include "Widget.h"
#include "Graphics.h"

#include "sound/effect.h"

#define PRESSED_EFFECT_MILLIS 100
#define PRESSED_REPEAT_INITIAL_MILLIS 300
#define PRESSED_REPEAT_MILLIS 50

static int getArrowButton(const mouse *m, ArrowButton *buttons, int numButtons);
static int getCustomButton(const mouse *m, int xOffset, int yOffset, CustomButton *buttons, int numButtons);


void Widget_Button_doNothing(int param1, int param2)
{
}

void Widget_Button_drawArrowButtons(int xOffset, int yOffset, ArrowButton *buttons, int numButtons)
{
	for (int i = 0; i < numButtons; i++) {
		int graphicId = buttons[i].graphicId;
		if (buttons[i].pressed) {
			graphicId += 1;
		}

		Graphics_drawImage(graphicId,
			xOffset + buttons[i].xOffset, yOffset + buttons[i].yOffset);
	}
}

int Widget_Button_handleArrowButtons(const mouse *m, ArrowButton *buttons, int numButtons)
{
	static int lastTime = 0;

	time_millis currTime = time_get_millis();
	int shouldRepeat = 0;
	if (currTime - lastTime >= 30) {
		shouldRepeat = 1;
		lastTime = currTime;
	}
	for (int i = 0; i < numButtons; i++) {
		ArrowButton *btn = &buttons[i];
		if (btn->pressed) {
			btn->pressed--;
			if (!btn->pressed) {
				btn->repeats = 0;
			}
		} else {
			btn->repeats = 0;
		}
	}
	int buttonId = getArrowButton(m, buttons, numButtons);
	if (!buttonId) {
		return 0;
	}
	ArrowButton *btn = &buttons[buttonId-1];
	if (m->left.went_down) {
		btn->pressed = 3;
		btn->repeats = 0;
		btn->leftClickHandler(btn->parameter1, btn->parameter2);
		return buttonId;
	}
	if (m->left.is_down) {
		btn->pressed = 3;
		if (shouldRepeat) {
			btn->repeats++;
			if (btn->repeats < 48) {
				int clickOnRepeat[] = {
					0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0,
					0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0,
					1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0
				};
				if (!clickOnRepeat[(int)btn->repeats]) {
					return 0;
				}
			} else {
				btn->repeats = 47;
			}
			btn->leftClickHandler(btn->parameter1, btn->parameter2);
		}
	}
	return buttonId;
}

static int getArrowButton(const mouse *m, ArrowButton *buttons, int numButtons)
{
	for (int i = 0; i < numButtons; i++) {
		if (buttons[i].xOffset <= m->x &&
			buttons[i].xOffset + buttons[i].size > m->x &&
			buttons[i].yOffset <= m->y &&
			buttons[i].yOffset + buttons[i].size > m->y) {
			return i + 1;
		}
	}
	return 0;
}

int Widget_Button_handleCustomButtons(int xOffset, int yOffset, CustomButton *buttons, int numButtons, int *focusButtonId)
{
    const mouse *m = mouse_get();
	int buttonId = getCustomButton(m, xOffset, yOffset, buttons, numButtons);
	if (focusButtonId) {
		*focusButtonId = buttonId;
	}
	if (!buttonId) {
		return 0;
	}
	CustomButton *button = &buttons[buttonId-1];
	if (button->buttonType == CustomButton_Immediate) {
		if (m->left.went_down) {
			button->leftClickHandler(button->parameter1, button->parameter2);
		} else if (m->right.went_down) {
			button->rightClickHandler(button->parameter1, button->parameter2);
		} else {
			return 0;
		}
	} else if (button->buttonType == CustomButton_OnMouseUp) {
		if (m->left.went_up) {
			button->leftClickHandler(button->parameter1, button->parameter2);
		} else if (m->right.went_up) {
			button->rightClickHandler(button->parameter1, button->parameter2);
		} else {
			return 0;
		}
	}
	return buttonId;
}

static int getCustomButton(const mouse *m, int xOffset, int yOffset, CustomButton *buttons, int numButtons)
{
	int mouseX = m->x;
	int mouseY = m->y;
	for (int i = 0; i < numButtons; i++) {
		if (xOffset + buttons[i].xStart <= mouseX &&
			xOffset + buttons[i].xEnd > mouseX &&
			yOffset + buttons[i].yStart <= mouseY &&
			yOffset + buttons[i].yEnd > mouseY) {
			return i + 1;
		}
	}
	return 0;
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

int Widget_Button_handleImageButtons(int xOffset, int yOffset, ImageButton *buttons, int numButtons, int *focusButtonId)
{
    const mouse *m = mouse_get();
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
		if (xOffset + btn->xOffset <= m->x &&
			xOffset + btn->xOffset + btn->width > m->x &&
			yOffset + btn->yOffset <= m->y &&
			yOffset + btn->yOffset + btn->height > m->y) {
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

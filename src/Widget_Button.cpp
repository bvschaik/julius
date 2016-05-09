#include "Widget.h"
#include "Graphics.h"
#include "Time.h"
#include "Sound.h"

#include "Data/Mouse.h"
#include "Data/Graphics.h"

#define PRESSED_EFFECT_MILLIS 100
#define PRESSED_REPEAT_INITIAL_MILLIS 300
#define PRESSED_REPEAT_MILLIS 50

static int getArrowButton(int xOffset, int yOffset, ArrowButton *buttons, int numButtons);
static int getCustomButton(int xOffset, int yOffset, CustomButton *buttons, int numButtons);


void Widget_Button_doNothing(int param1, int param2)
{
}

void Widget_Button_drawArrowButtons(int xOffset, int yOffset, ArrowButton *buttons, int numButtons)
{
	for (int i = 0; i < numButtons; i++) {
		int graphicId = buttons[i].graphicId;
		if (buttons[i].field_D) {
			graphicId += 1;
		}

		Graphics_drawImage(graphicId,
			xOffset + buttons[i].xOffset, yOffset + buttons[i].yOffset);
	}
}

int Widget_Button_handleArrowButtons(int xOffset, int yOffset, ArrowButton *buttons, int numButtons)
{
	static int lastTime = 0;

	TimeMillis currTime = Time_getMillis();
	int shouldRepeat = 0;
	if (currTime - lastTime >= 30) {
		shouldRepeat = 1;
		lastTime = currTime;
	}
	for (int i = 0; i < numButtons; i++) {
		ArrowButton *btn = &buttons[i];
		if (btn->field_D) {
			btn->field_D--;
			if (!btn->field_D) {
				btn->field_C = 1;
				btn->field_E = 0;
			}
		} else {
			btn->field_E = 0;
		}
	}
	int buttonId = getArrowButton(xOffset, yOffset, buttons, numButtons);
	if (!buttonId) {
		return 0;
	}
	ArrowButton *btn = &buttons[buttonId-1];
	if (Data_Mouse.left.wentDown) {
		btn->field_D = 3;
		btn->field_E = 0;
		btn->field_C = 1;
		btn->leftClickHandler(btn->parameter1, btn->parameter2);
		return buttonId;
	}
	if (Data_Mouse.left.isDown) {
		btn->field_D = 3;
		if (shouldRepeat) {
			btn->field_E++;
			if (btn->field_E < 48) {
				if (btn->field_E < 8) {
					return 0;
				}
				int foo[] = {
					0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0,
					0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0,
					1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0
				};
				if (!foo[(int)btn->field_E]) {
					return 0;
				}
			} else {
				btn->field_E = 47;
			}
			btn->leftClickHandler(btn->parameter1, btn->parameter2);
		}
	}
	return buttonId;
}

static int getArrowButton(int xOffset, int yOffset, ArrowButton *buttons, int numButtons)
{
	int mouseX = Data_Mouse.x;
	int mouseY = Data_Mouse.y;
	for (int i = 0; i < numButtons; i++) {
		if (xOffset + buttons[i].xOffset <= mouseX &&
			xOffset + buttons[i].xOffset + buttons[i].size > mouseX &&
			yOffset + buttons[i].yOffset <= mouseY &&
			yOffset + buttons[i].yOffset + buttons[i].size > mouseY) {
			return i + 1;
		}
	}
	return 0;
}

int Widget_Button_handleCustomButtons(int xOffset, int yOffset, CustomButton *buttons, int numButtons, int *focusButtonId)
{
	int buttonId = getCustomButton(xOffset, yOffset, buttons, numButtons);
	if (focusButtonId) {
		*focusButtonId = buttonId;
	}
	if (!buttonId) {
		return 0;
	}
	CustomButton *button = &buttons[buttonId-1];
	if (button->type == CustomButton_Immediate) {
		if (Data_Mouse.left.wentDown || Data_Mouse.left.isDown) {
			button->leftClickHandler(button->parameter1, button->parameter2);
		} else if (Data_Mouse.right.wentDown) {
			button->rightClickHandler(button->parameter1, button->parameter2);
		} else {
			return 0;
		}
	} else if (button->type == CustomButton_OnMouseUp) {
		if (Data_Mouse.left.wentUp) {
			button->leftClickHandler(button->parameter1, button->parameter2);
		} else if (Data_Mouse.right.wentUp) {
			button->rightClickHandler(button->parameter1, button->parameter2);
		} else {
			return 0;
		}
	}
	return buttonId;
}

static int getCustomButton(int xOffset, int yOffset, CustomButton *buttons, int numButtons)
{
	int mouseX = Data_Mouse.x;
	int mouseY = Data_Mouse.y;
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
	TimeMillis currentTime = Time_getMillis();
	for (int i = 0; i < numButtons; i++) {
		ImageButton *btn = &buttons[i];
		if (btn->pressed) {
			if (btn->buttonType == ImageButton_Normal) {
				if (btn->pressedSince + PRESSED_EFFECT_MILLIS < currentTime) {
					btn->pressed = 0;
				}
			} else if (btn->buttonType == ImageButton_Scroll) {
				if (btn->pressedSince + PRESSED_EFFECT_MILLIS < currentTime && !Data_Mouse.left.isDown) {
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
		int graphicId = GraphicId(btn->graphicCollection) + btn->graphicIdOffset;
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
	imageButtonFadePressedEffect(buttons, numButtons);
	imageButtonRemovePressedEffectBuild(buttons, numButtons);
	int mouseX = Data_Mouse.x;
	int mouseY = Data_Mouse.y;
	ImageButton *hitButton = 0;
	if (focusButtonId) {
		*focusButtonId = 0;
	}
	for (int i = 0; i < numButtons; i++) {
		ImageButton *btn = &buttons[i];
		if (btn->focused) {
			btn->focused--;
		}
		if (xOffset + btn->xOffset <= mouseX &&
			xOffset + btn->xOffset + btn->width > mouseX &&
			yOffset + btn->yOffset <= mouseY &&
			yOffset + btn->yOffset + btn->height > mouseY) {
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
		if (!Data_Mouse.left.wentDown && !Data_Mouse.left.isDown) {
			return 0;
		}
	} else if (hitButton->buttonType == ImageButton_Build || hitButton->buttonType == ImageButton_Normal) {
		if (!Data_Mouse.left.wentDown && !Data_Mouse.right.wentDown) {
			return 0;
		}
	}
	if (Data_Mouse.left.wentDown) {
		Sound_Effects_playChannel(SoundChannel_Icon);
		hitButton->pressed = 1;
		hitButton->pressedSince = Time_getMillis();
		hitButton->leftClickHandler(hitButton->parameter1, hitButton->parameter2);
	} else if (Data_Mouse.right.wentUp) {
		hitButton->pressed = 1;
		hitButton->pressedSince = Time_getMillis();
		hitButton->rightClickHandler(hitButton->parameter1, hitButton->parameter2);
	} else if (hitButton->buttonType == ImageButton_Scroll && Data_Mouse.left.isDown) {
		TimeMillis delay = hitButton->pressed == 2 ? PRESSED_REPEAT_MILLIS : PRESSED_REPEAT_INITIAL_MILLIS;
		if (hitButton->pressedSince + delay <= Time_getMillis()) {
			hitButton->pressed = 2;
			hitButton->pressedSince = Time_getMillis();
			hitButton->leftClickHandler(hitButton->parameter1, hitButton->parameter2);
 		}
	}
	return 1;
}

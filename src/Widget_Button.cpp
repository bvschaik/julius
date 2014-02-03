#include "Widget.h"
#include "Graphics.h"

#include "Data/Mouse.h"
#include "Data/Graphics.h"

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
		if (buttons[i].field_D) {
			// TODO request refresh
		}
	}
}

int Widget_Button_handleArrowButtons(int xOffset, int yOffset, ArrowButton *buttons, int numButtons)
{
	/*
	static int lastTime = Time_getTime();
	int now = Time_getTime();

	int shouldRepeat = 0;
	if (now - lastTime >= 30) {
		shouldRepeat = 1;
		lastTime = now;
	}
	*/
	for (int i = 0; i < numButtons; i++) {
		if (buttons[i].field_D) {
			buttons[i].field_D--;
			if (!buttons[i].field_D) {
				buttons[i].field_C = 1;
				buttons[i].field_E = 0;
			}
		} else {
			buttons[i].field_E = 0;
		}
	}
	int buttonId = getArrowButton(xOffset, yOffset, buttons, numButtons);
	if (!buttonId) {
		return 0;
	}

	// TODO simple implementation
	ArrowButton *button = &buttons[buttonId-1];
	if (Data_Mouse.isLeftClick) {
		button->leftClickHandler(button->parameter1, button->parameter2);
	}
	return buttonId;
/*
  if ( byte_660B94[0] || mouseInfo_buttonIsDown )
  {
    buttons->field_D = 3;
    buttons->field_E = 0;
    buttons->field_C = 1;
    currentButton_parameter = buttons->parameter;
    currentButton_parameter2 = buttons->parameter2;
    dword_6ADD18 = 0;
    buttons->mouseClickHandler();
    return j + 1;
  }
  if ( !mouse_isLeftClick )
    return 0;
  buttons->field_D = 3;
  if ( shouldRepeat )
  {
    ++buttons->field_E;
    if ( buttons->field_E < 48 )
    {
      if ( buttons->field_E < 8 )
        return 0;
      if ( !byte_5F1098[buttons->field_E] )
        return 0;
    }
    else
    {
      buttons->field_E = 47;
    }
    currentButton_parameter = buttons->parameter;
    currentButton_parameter2 = buttons->parameter2;
    dword_6ADD18 = buttons->field_E;
    buttons->mouseClickHandler();
    result = j + 1;
  }
  else
  {
    result = 0;
  }

	}
	*/
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
	if (button->type == 1) {
		// TODO
	} else if (button->type == 2) {
		// TODO
	} else if (button->type == 3) {
		// TODO
	} else if (button->type == 4) {
		// TODO
	}

	// TODO simple implementation
	if (Data_Mouse.isLeftClick) {
		button->leftClickHandler(button->parameter1, button->parameter2);
	} else if (Data_Mouse.isRightClick) {
		button->rightClickHandler(button->parameter1, button->parameter2);
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

void Widget_Button_drawImageButtons(int xOffset, int yOffset, ImageButton *buttons, int numButtons)
{
	for (int i = 0; i < numButtons; i++) {
		ImageButton *btn = &buttons[i];
		int graphicId = GraphicId(btn->graphicCollection) + btn->graphicIdOffset;
		if (btn->enabled) {
			if (btn->hasClickEffect) {
				graphicId += 2;
			} else if (btn->hasFocus) {
				graphicId += 1;
			}
		} else {
			graphicId += 3;
		}
		Graphics_drawImage(graphicId, xOffset + btn->xOffset, yOffset + btn->yOffset);
	}
}

int Widget_Button_handleImageButtons(int xOffset, int yOffset, ImageButton *buttons, int numButtons)
{
	// TODO field_19 manipulation
	for (int i = 0; i < numButtons; i++) {
		ImageButton *btn = &buttons[i];
		if (btn->hasClickEffect) {
			if (btn->field_8 == 4 || btn->field_8 == 6) {
				btn->hasClickEffect--;
				if (btn->hasClickEffect <= 0) {
					btn->hasClickEffect = 0;
				}
			}
		}
	}
	// end field_19 manipulation
	int mouseX = Data_Mouse.x;
	int mouseY = Data_Mouse.y;
	int change = 0;
	ImageButton *hitButton = 0;
	int hitIndex = 0;
	for (int i = 0; i < numButtons; i++) {
		ImageButton *btn = &buttons[i];
		if (btn->hasFocus) {
			btn->hasFocus--;
			if (!btn->hasFocus) {
				change = 1;
			}
		}
		if (btn->enabled) {
			if (xOffset + btn->xOffset <= mouseX &&
				xOffset + btn->xOffset + btn->width > mouseX &&
				yOffset + btn->yOffset <= mouseY &&
				yOffset + btn->yOffset + btn->height > mouseY) {
				// TODO button_x = Data_Mouse.x; button_y = Data_Mouse.y
				if (!btn->hasFocus) {
					change = 1;
				}
				btn->hasFocus = 2;
				hitButton = btn;
				hitIndex = i + 1;
			}
		}
	}
	if (!hitButton) {
		return 0;
	}
	// TODO field_19 manipulation
	if (hitButton->field_8 == 2) {
		for (int i = 0; i < numButtons; i++) {
			ImageButton *btn = &buttons[i];
			if (btn->hasClickEffect) {
				if (btn->field_8 == 2) {
					btn->hasClickEffect = 0;
				}
			}
		}
	}
	// TODO sound playing?
	if (Data_Mouse.isLeftClick) {
		// TODO play sound
		hitButton->hasClickEffect = 10;
		hitButton->leftClickHandler(hitButton->parameter1, hitButton->parameter2);
	} else if (Data_Mouse.isRightClick) {
		hitButton->hasClickEffect = 10;
		hitButton->rightClickHandler(hitButton->parameter1, hitButton->parameter2);
	}
	return hitIndex;
}

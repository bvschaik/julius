#ifndef WIDGET_BUTTON_H
#define WIDGET_BUTTON_H

struct ArrowButton;
struct ImageButton;
struct CustomButton;

namespace Widget
{

struct Button
{
    static void drawArrowButtons(int xOffset, int yOffset, ArrowButton *buttons, int numButtons);
    static int handleArrowButtons(int xOffset, int yOffset, ArrowButton *buttons, int numButtons);
    static void drawImageButtons(int xOffset, int yOffset, ImageButton *buttons, int numButtons);
    static int handleImageButtons(int xOffset, int yOffset, ImageButton *buttons, int numButtons, int *focusButtonId);
    static int handleCustomButtons(int xOffset, int yOffset, CustomButton *buttons, int numButtons, int *focusButtonId);
    static void doNothing(int param1, int param2);
};

}

#endif

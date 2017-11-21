#ifndef DATA_BUTTONS_H
#define DATA_BUTTONS_H

#include "core/time.h"

enum
{
    ImageButton_Normal = 4,
    ImageButton_Scroll = 6,
    ImageButton_Build = 2
};

struct ImageButton
{
    short xOffset;
    short yOffset;
    short width;
    short height;
    short buttonType;
    short graphicCollection;
    short graphicIdOffset;
    void (*leftClickHandler)(int param1, int param2);
    void (*rightClickHandler)(int param1, int param2);
    int parameter1;
    int parameter2;
    char enabled;
    // state
    char pressed;
    char focused;
    time_millis pressedSince;
};

enum
{
    CustomButton_Immediate = 1,
    CustomButton_OnMouseUp = 3,
};

struct CustomButton
{
    short xStart;
    short yStart;
    short xEnd;
    short yEnd;
    int buttonType;
    void (*leftClickHandler)(int param1, int param2);
    void (*rightClickHandler)(int param1, int param2);
    int parameter1;
    int parameter2;
};

struct ArrowButton
{
    short xOffset;
    short yOffset;
    short graphicId;
    short size;
    void (*leftClickHandler)(int param1, int param2);
    int parameter1;
    int parameter2;
    // state
    char pressed;
    char repeats;
} ;

typedef struct
{
    short yStart;
    short textNumber;
    void (*leftClickHandler)(int param);
    int parameter;
} MenuItem;

struct MenuBarItem
{
    short xStart;
    short xEnd;
    short yStart;
    short textGroup;
    MenuItem *items;
    int numItems;
} ;

#endif

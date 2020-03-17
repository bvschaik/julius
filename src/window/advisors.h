#ifndef WINDOW_ADVISORS_H
#define WINDOW_ADVISORS_H

#include "city/constants.h"
#include "input/mouse.h"

typedef struct {
    /**
     * @return height of the advisor in blocks of 16px
     */
    int (*draw_background)(void);
    void (*draw_foreground)(void);
    int (*handle_mouse)(const mouse *m);
    int (*get_tooltip_text)(void);
} advisor_window_type;

advisor_type window_advisors_get_advisor(void);

void window_advisors_draw_dialog_background(void);

void window_advisors_show(void);

void window_advisors_show_checked(void);

int window_advisors_show_advisor(advisor_type advisor);

#endif // WINDOW_ADVISORS_H

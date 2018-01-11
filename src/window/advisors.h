#ifndef WINDOW_ADVISORS_H
#define WINDOW_ADVISORS_H

#include "city/constants.h"

advisor_type window_advisors_get_advisor();

void window_advisors_draw_dialog_background();

void window_advisors_show();

void window_advisors_show_checked();

void window_advisors_show_advisor(advisor_type advisor);

#endif // WINDOW_ADVISORS_H

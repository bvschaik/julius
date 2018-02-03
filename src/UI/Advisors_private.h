#ifndef UI_ADVISORS_PRIVATE_H
#define UI_ADVISORS_PRIVATE_H

#include "../Data/CityInfo.h"

#include "core/image.h"
#include "input/mouse.h"
#include "window/advisors.h"

void UI_Advisor_Labor_drawBackground(int *advisorHeight);
void UI_Advisor_Labor_drawForeground();
void UI_Advisor_Labor_handleMouse(const mouse *m);
int UI_Advisor_Labor_getTooltip();

void UI_Advisor_Trade_drawBackground(int *advisorHeight);
void UI_Advisor_Trade_drawForeground();
void UI_Advisor_Trade_handleMouse(const mouse *m);
int UI_Advisor_Trade_getTooltip();

void UI_Advisor_Population_drawBackground(int *advisorHeight);
void UI_Advisor_Population_drawForeground();
void UI_Advisor_Population_handleMouse(const mouse *m);
int UI_Advisor_Population_getTooltip();

#endif

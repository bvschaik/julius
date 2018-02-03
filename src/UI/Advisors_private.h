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

void UI_Advisor_Military_drawBackground(int *advisorHeight);
void UI_Advisor_Military_drawForeground();
void UI_Advisor_Military_handleMouse(const mouse *m);

void UI_Advisor_Imperial_drawBackground(int *advisorHeight);
void UI_Advisor_Imperial_drawForeground();
void UI_Advisor_Imperial_handleMouse(const mouse *m);
int UI_Advisor_Imperial_getTooltip();

void UI_Advisor_Ratings_drawBackground(int *advisorHeight);
void UI_Advisor_Ratings_drawForeground();
void UI_Advisor_Ratings_handleMouse(const mouse *m);
int UI_Advisor_Ratings_getTooltip();

void UI_Advisor_Trade_drawBackground(int *advisorHeight);
void UI_Advisor_Trade_drawForeground();
void UI_Advisor_Trade_handleMouse(const mouse *m);
int UI_Advisor_Trade_getTooltip();

void UI_Advisor_Population_drawBackground(int *advisorHeight);
void UI_Advisor_Population_drawForeground();
void UI_Advisor_Population_handleMouse(const mouse *m);
int UI_Advisor_Population_getTooltip();

void UI_Advisor_Health_drawBackground(int *advisorHeight);

void UI_Advisor_Education_drawBackground(int *advisorHeight);

void UI_Advisor_Entertainment_init();
void UI_Advisor_Entertainment_drawBackground(int *advisorHeight);
void UI_Advisor_Entertainment_drawForeground();
void UI_Advisor_Entertainment_handleMouse(const mouse *m);
int UI_Advisor_Entertainment_getTooltip();

void UI_Advisor_Religion_drawBackground(int *advisorHeight);

void UI_Advisor_Financial_drawBackground(int *advisorHeight);
void UI_Advisor_Financial_drawForeground();
void UI_Advisor_Financial_handleMouse(const mouse *m);
int UI_Advisor_Financial_getTooltip();

#endif

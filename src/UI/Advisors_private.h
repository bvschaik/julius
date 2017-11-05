#ifndef UI_ADVISORS_PRIVATE_H
#define UI_ADVISORS_PRIVATE_H

#include "../Widget.h"
#include "../Graphics.h"

#include "data/screen.hpp"
#include "data/cityinfo.hpp"
#include "data/constants.hpp"

#include "graphics/image.h"

void UI_Advisor_drawGeneralBackground();

void UI_Advisor_Labor_drawBackground(int *advisorHeight);
void UI_Advisor_Labor_drawForeground();
void UI_Advisor_Labor_handleMouse();
int UI_Advisor_Labor_getTooltip();

void UI_Advisor_Military_drawBackground(int *advisorHeight);
void UI_Advisor_Military_drawForeground();
void UI_Advisor_Military_handleMouse();

void UI_Advisor_Imperial_drawBackground(int *advisorHeight);
void UI_Advisor_Imperial_drawForeground();
void UI_Advisor_Imperial_handleMouse();
int UI_Advisor_Imperial_getTooltip();

void UI_Advisor_Ratings_drawBackground(int *advisorHeight);
void UI_Advisor_Ratings_drawForeground();
void UI_Advisor_Ratings_handleMouse();
int UI_Advisor_Ratings_getTooltip();

void UI_Advisor_Trade_drawBackground(int *advisorHeight);
void UI_Advisor_Trade_drawForeground();
void UI_Advisor_Trade_handleMouse();
int UI_Advisor_Trade_getTooltip();

void UI_Advisor_Population_drawBackground(int *advisorHeight);
void UI_Advisor_Population_drawForeground();
void UI_Advisor_Population_handleMouse();
int UI_Advisor_Population_getTooltip();

void UI_Advisor_Health_drawBackground(int *advisorHeight);

void UI_Advisor_Education_drawBackground(int *advisorHeight);

void UI_Advisor_Entertainment_init();
void UI_Advisor_Entertainment_drawBackground(int *advisorHeight);
void UI_Advisor_Entertainment_drawForeground();
void UI_Advisor_Entertainment_handleMouse();
int UI_Advisor_Entertainment_getTooltip();

void UI_Advisor_Religion_drawBackground(int *advisorHeight);

void UI_Advisor_Financial_drawBackground(int *advisorHeight);
void UI_Advisor_Financial_drawForeground();
void UI_Advisor_Financial_handleMouse();
int UI_Advisor_Financial_getTooltip();

void UI_Advisor_Chief_drawBackground(int *advisorHeight);

#endif

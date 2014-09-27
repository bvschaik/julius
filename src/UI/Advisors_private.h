#ifndef UI_ADVISORS_PRIVATE_H
#define UI_ADVISORS_PRIVATE_H

#include "../Widget.h"
#include "../Graphics.h"

#include "../Data/Screen.h"
#include "../Data/CityInfo.h"
#include "../Data/Constants.h"
#include "../Data/Graphics.h"

void UI_Advisor_drawGeneralBackground();

void UI_Advisor_Labor_drawBackground(int *advisorHeight);
void UI_Advisor_Labor_drawForeground();
void UI_Advisor_Labor_handleMouse();

void UI_Advisor_Military_drawBackground(int *advisorHeight);
void UI_Advisor_Military_drawForeground();
void UI_Advisor_Military_handleMouse();

void UI_Advisor_Imperial_drawBackground(int *advisorHeight);
void UI_Advisor_Imperial_drawForeground();
void UI_Advisor_Imperial_handleMouse();

void UI_Advisor_Ratings_drawBackground(int *advisorHeight);
void UI_Advisor_Ratings_drawForeground();
void UI_Advisor_Ratings_handleMouse();

void UI_Advisor_Trade_drawBackground(int *advisorHeight);
void UI_Advisor_Trade_drawForeground();
void UI_Advisor_Trade_handleMouse();

void UI_Advisor_Population_drawBackground(int *advisorHeight);
void UI_Advisor_Population_drawForeground();
void UI_Advisor_Population_handleMouse();

void UI_Advisor_Health_drawBackground(int *advisorHeight);

void UI_Advisor_Education_drawBackground(int *advisorHeight);

void UI_Advisor_Entertainment_init();
void UI_Advisor_Entertainment_drawBackground(int *advisorHeight);
void UI_Advisor_Entertainment_drawForeground();
void UI_Advisor_Entertainment_handleMouse();

void UI_Advisor_Religion_drawBackground(int *advisorHeight);

void UI_Advisor_Financial_drawBackground(int *advisorHeight);
void UI_Advisor_Financial_drawForeground();
void UI_Advisor_Financial_handleMouse();

void UI_Advisor_Chief_drawBackground(int *advisorHeight);

#endif

#ifndef UI_ADVISORS_PRIVATE_H
#define UI_ADVISORS_PRIVATE_H

#include "../Widget.h"
#include "../Graphics.h"

#include "../Data/Screen.h"
#include "../Data/CityInfo.h"
#include "../Data/Constants.h"
#include "../Data/Graphics.h"

enum {
	Advisor_None = 0,
	Advisor_Labor = 1,
	Advisor_Military = 2,
	Advisor_Imperial = 3,
	Advisor_Ratings = 4,
	Advisor_Trade = 5,
	Advisor_Population = 6,
	Advisor_Health = 7,
	Advisor_Education = 8,
	Advisor_Entertainment = 9,
	Advisor_Religion = 10,
	Advisor_Financial = 11,
	Advisor_Chief = 12
};

void UI_Advisor_Labor_drawBackground();
void UI_Advisor_Labor_drawForeground();
void UI_Advisor_Labor_handleMouse();

void UI_Advisor_Ratings_drawBackground();
void UI_Advisor_Ratings_drawForeground();
void UI_Advisor_Ratings_handleMouse();

void UI_Advisor_Trade_drawBackground();

void UI_Advisor_Population_drawBackground();
void UI_Advisor_Population_drawForeground();
void UI_Advisor_Population_handleMouse();

void UI_Advisor_Health_drawBackground();

void UI_Advisor_Education_drawBackground();

void UI_Advisor_Entertainment_drawBackground();
void UI_Advisor_Entertainment_drawForeground();
void UI_Advisor_Entertainment_handleMouse();


void UI_Advisor_Religion_drawBackground();


#endif

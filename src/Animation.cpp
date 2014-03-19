#include "Animation.h"
#include "Data/Graphics.h"
#include "Time.h"

static unsigned int lastUpdate[51];
static int shouldUpdate[51];

void Animation_resetTimers()
{
	for (int i = 0; i < 51; i++) {
		lastUpdate[i] = 0;
		shouldUpdate[i] = 0;
	}
}

void Animation_updateTimers()
{
	TimeMillis currentTimeMillis = Time_getMillis();
	for (int i = 0; i < 51; i++) {
		shouldUpdate[i] = 0;
	}
	int delayMillis = 0;
	for (int i = 0; i < 51; i++) {
		if (currentTimeMillis >= delayMillis + lastUpdate[i]) {
			shouldUpdate[i] = 1;
			lastUpdate[i] = currentTimeMillis;
		}
		delayMillis += 20;
	}
}

int Animation_getIndexForCityBuilding(int graphicId, int gridOffset)
{
	// TODO
	return 1;
}

int Animation_getIndexForEmpireMap(int graphicId, int currentIndex)
{
	if (currentIndex <= 0) {
		currentIndex = 1;
	}
	int animationSpeed = Data_Graphics_Main.index[graphicId].animationSpeedId;
	if (!shouldUpdate[animationSpeed]) {
		return currentIndex;
	}
	// assumption: the 'unused26' field of graphics index isn't set
	currentIndex++;
	if (currentIndex > Data_Graphics_Main.index[graphicId].numAnimationSprites) {
		currentIndex = 1;
	}
	return currentIndex;
}

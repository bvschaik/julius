#ifndef ANIMATION_H
#define ANIMATION_H

struct Animation
{
    static void resetTimers();
    static void updateTimers();
    static int getIndexForCityBuilding(int graphicId, int gridOffset);
    static int getIndexForEmpireMap(int graphicId, int currentIndex);
};

#endif

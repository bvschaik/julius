#ifndef BUILDINGPLACEMENT_H
#define BUILDINGPLACEMENT_H

int BuildingPlacement_getCost();
void BuildingPlacement_update(int xStart, int yStart, int xEnd, int yEnd, int type);
void BuildingPlacement_place(int orientation, int xStart, int yStart, int xEnd, int yEnd, int type);

#endif

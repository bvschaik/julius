#ifndef UNDO_H
#define UNDO_H

int Undo_recordBeforeBuild();

void Undo_restoreTerrainGraphics();
void Undo_restoreBuildings();

void Undo_addBuildingToList(int buildingId);
int Undo_isBuildingInList(int buildingId);

void Undo_recordBuild(int cost);

void Undo_perform();

void Undo_updateAvailable();

#endif

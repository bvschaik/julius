#ifndef TERRAIN_H
#define TERRAIN_H

void Terrain_addRoadsForGatehouse(int x, int y, int orientation);
void Terrain_addRoadsForTriumphalArch(int x, int y, int orientation);

void Terrain_updateEntryExitFlags(int remove);

#endif

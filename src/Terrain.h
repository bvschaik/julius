#ifndef TERRAIN_H
#define TERRAIN_H

void Terrain_setWithRadius(int x, int y, int size, int radius, unsigned short typeToAdd);
void Terrain_clearWithRadius(int x, int y, int size, int radius, unsigned short typeToKeep);

int Terrain_existsTileWithinRadiusWithType(int x, int y, int size, int radius, unsigned short type);
int Terrain_existsClearTileWithinRadius(int x, int y, int size, int radius, int exceptGridOffset);
int Terrain_allTilesWithinRadiusHaveType(int x, int y, int size, int radius, unsigned short type);

void Terrain_markNativeLand(int x, int y, int size, int radius);
int Terrain_hasBuildingOnNativeLand();

#endif

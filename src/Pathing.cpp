//#include "Pathing.h"

#include "Data/Building.h"
#include "Data/Constants.h"
#include "Data/Graphics.h"
#include "Data/Grid.h"
#include "Data/Settings.h"

#include <string.h>

void Pathing_determineLandCitizen()
{
	memset(Data_Grid_pathingLandCitizen, -1, GRID_SIZE * GRID_SIZE);
	int gridOffset = Data_Settings_Map.gridStartOffset;
	for (int y = 0; y < Data_Settings_Map.height; y++, gridOffset += Data_Settings_Map.gridBorderSize) {
		for (int x = 0; x < Data_Settings_Map.width; x++, gridOffset++) {
			if (Data_Grid_terrain[gridOffset] & Terrain_Road) {
				Data_Grid_pathingLandCitizen[gridOffset] = 0;
			} else if (Data_Grid_terrain[gridOffset] & 0x1420) { // rubble, access ramp, garden
				Data_Grid_pathingLandCitizen[gridOffset] = 2;
			} else if (Data_Grid_terrain[gridOffset] & (Terrain_Building | Terrain_Gatehouse)) {
				// TODO
				int buildingId = Data_Grid_buildingIds[gridOffset];
				if (!buildingId) {
					// shouldn't happen - correct
					Data_Grid_pathingLandNonCitizen[gridOffset] = 4; // BUGFIX - should be citizen?
					Data_Grid_terrain[gridOffset] &= 0xfff7; // remove 8 = building
					Data_Grid_graphicIds[gridOffset] = (Data_Grid_random[gridOffset] & 7) + GraphicId(ID_Graphic_TerrainGrass1);
					Data_Grid_edge[gridOffset] = Edge_LeftmostTile;
					Data_Grid_bitfields[gridOffset] &= 0xf0; // remove sizes
					continue;
				}
				int land = -1;
				switch (Data_Buildings[buildingId].type) {
					case Building_Warehouse:
					case Building_Gatehouse:
						land = 0;
						break;
					case Building_FortGround:
						land = 2;
						break;
					case Building_TriumphalArch:
						if (Data_Buildings[buildingId].subtype.orientation == 3) {
							switch (Data_Grid_edge[gridOffset] & Edge_MaskXY) {
								case Edge_X0Y1:
								case Edge_X1Y1:
								case Edge_X2Y1:
									land = 0;
									break;
							}
						} else {
							switch (Data_Grid_edge[gridOffset] & Edge_MaskXY) {
								case Edge_X1Y0:
								case Edge_X1Y1:
								case Edge_X1Y2:
									land = 0;
									break;
							}
						}
						break;
					case Building_Granary:
						switch (Data_Grid_edge[gridOffset] & Edge_MaskXY) {
							case Edge_X1Y0:
							case Edge_X0Y1:
							case Edge_X1Y1:
							case Edge_X2Y1:
							case Edge_X1Y2:
								land = 0;
								break;
						}
						break;
					case Building_Reservoir:
						switch (Data_Grid_edge[gridOffset] & Edge_MaskXY) {
							case Edge_X1Y0:
							case Edge_X0Y1:
							case Edge_X2Y1:
							case Edge_X1Y2:
								land = -4; // aqueduct connect points
								break;
						}
						break;
				}
				Data_Grid_pathingLandCitizen[gridOffset] = land;
			} else if (Data_Grid_terrain[gridOffset] & Terrain_Aqueduct) {
				int graphicId = Data_Grid_graphicIds[gridOffset] - GraphicId(ID_Graphic_Aqueduct);
				int land;
				if (graphicId <= 3) {
					land = -3;
				} else if (graphicId <= 7) {
					land = -1;
				} else if (graphicId <= 9) {
					land = -3;
				} else if (graphicId <= 14) {
					land = -1;
				} else if (graphicId <= 18) {
					land = -3;
				} else if (graphicId <= 22) {
					land = -1;
				} else if (graphicId <= 24) {
					land = -3;
				} else {
					land = -1;
				}
				Data_Grid_pathingLandCitizen[gridOffset] = land;
			} else if (Data_Grid_terrain[gridOffset] & Terrain_NotClear) {
				Data_Grid_pathingLandCitizen[gridOffset] = -1;
			} else {
				Data_Grid_pathingLandCitizen[gridOffset] = 4;
			}
		}
	}
}

void Pathing_determineLandNonCitizen()
{
	memset(Data_Grid_pathingLandNonCitizen, -1, GRID_SIZE * GRID_SIZE);
	int gridOffset = Data_Settings_Map.gridStartOffset;
	for (int y = 0; y < Data_Settings_Map.height; y++, gridOffset += Data_Settings_Map.gridBorderSize) {
		for (int x = 0; x < Data_Settings_Map.width; x++, gridOffset++) {
			int terrain = Data_Grid_terrain[gridOffset] & Terrain_NotClear;
			if (Data_Grid_terrain[gridOffset] & Terrain_Gatehouse) {
				Data_Grid_pathingLandNonCitizen[gridOffset] = 4;
			} else if (terrain & Terrain_Road) {
				Data_Grid_pathingLandNonCitizen[gridOffset] = 0;
			} else if (terrain & 0x1420) {
				Data_Grid_pathingLandNonCitizen[gridOffset] = 2;
			} else if (terrain & Terrain_Building) {
				int land = 1;
				switch (Data_Buildings[Data_Grid_buildingIds[gridOffset]].type) {
					case Building_Warehouse:
					case Building_FortGround:
						land = 0;
						break;
					case Building_BurningRuin:
					case Building_NativeHut:
					case Building_NativeMeeting:
					case Building_NativeCrops:
						land = -1;
						break;
					case Building_FortGround__:
						land = 5;
						break;
					case Building_Granary:
						switch (Data_Grid_edge[gridOffset] & Edge_MaskXY) {
							case Edge_X1Y0:
							case Edge_X0Y1:
							case Edge_X1Y1:
							case Edge_X2Y1:
							case Edge_X1Y2:
								land = 0;
								break;
						}
						break;
				}
				Data_Grid_pathingLandNonCitizen[gridOffset] = land;
			} else if (Data_Grid_terrain[gridOffset] & Terrain_Aqueduct) {
				Data_Grid_pathingLandNonCitizen[gridOffset] = 2;
			} else if (Data_Grid_terrain[gridOffset] & Terrain_Wall) {
				Data_Grid_pathingLandNonCitizen[gridOffset] = 3;
			} else if (Data_Grid_terrain[gridOffset] & Terrain_NotClear) {
				Data_Grid_pathingLandNonCitizen[gridOffset] = -1;
			} else {
				Data_Grid_pathingLandNonCitizen[gridOffset] = 0;
			}
		}
	}
}

void Pathing_determineWater()
{
	memset(Data_Grid_pathingWalls, -1, GRID_SIZE * GRID_SIZE);
	int gridOffset = Data_Settings_Map.gridStartOffset;
	for (int y = 0; y < Data_Settings_Map.height; y++, gridOffset += Data_Settings_Map.gridBorderSize) {
		for (int x = 0; x < Data_Settings_Map.width; x++, gridOffset++) {
			if (Data_Grid_terrain[gridOffset] & Terrain_Water) {
				if ((Data_Grid_terrain[gridOffset - 162] & Terrain_Water) &&
					(Data_Grid_terrain[gridOffset - 1] & Terrain_Water) &&
					(Data_Grid_terrain[gridOffset + 1] & Terrain_Water) &&
					(Data_Grid_terrain[gridOffset + 162] & Terrain_Water)) {
					if (x > 0 && x < Data_Settings_Map.width - 1 &&
						y > 0 && y < Data_Settings_Map.height - 1) {
						switch (Data_Grid_spriteOffsets[gridOffset]) {
							case 5:
							case 6:
								Data_Grid_pathingWater[gridOffset] = -3;
								break;
							case 13:
								Data_Grid_pathingWater[gridOffset] = -1;
								break;
							default:
								Data_Grid_pathingWater[gridOffset] = 0;
								break;
						}
					} else {
						Data_Grid_pathingWater[gridOffset] = -2;
					}
				} else {
					Data_Grid_pathingWater[gridOffset] = -1;
				}
			} else {
				Data_Grid_pathingWater[gridOffset] = -1;
			}
		}
	}
}

#define WALL_GATE 0xc000
void Pathing_determineWalls()
{
	memset(Data_Grid_pathingWalls, -1, GRID_SIZE * GRID_SIZE);
	int gridOffset = Data_Settings_Map.gridStartOffset;
	for (int y = 0; y < Data_Settings_Map.height; y++, gridOffset += Data_Settings_Map.gridBorderSize) {
		for (int x = 0; x < Data_Settings_Map.width; x++, gridOffset++) {
			if (Data_Grid_terrain[gridOffset] & Terrain_Wall) {
				int adjacent = 0;
				switch (Data_Settings_Map.orientation) {
					case Direction_Top:
						if (Data_Grid_terrain[gridOffset + 162] & WALL_GATE) {
							adjacent++;
						}
						if (Data_Grid_terrain[gridOffset + 163] & WALL_GATE) {
							adjacent++;
						}
						if (Data_Grid_terrain[gridOffset + 1] & WALL_GATE) {
							adjacent++;
						}
						break;
					case Direction_Right:
						if (Data_Grid_terrain[gridOffset + 162] & WALL_GATE) {
							adjacent++;
						}
						if (Data_Grid_terrain[gridOffset + 161] & WALL_GATE) {
							adjacent++;
						}
						if (Data_Grid_terrain[gridOffset - 1] & WALL_GATE) {
							adjacent++;
						}
						break;
					case Direction_Bottom:
						if (Data_Grid_terrain[gridOffset - 162] & WALL_GATE) {
							adjacent++;
						}
						if (Data_Grid_terrain[gridOffset - 163] & WALL_GATE) {
							adjacent++;
						}
						if (Data_Grid_terrain[gridOffset - 1] & WALL_GATE) {
							adjacent++;
						}
						break;
					case Direction_Left:
						if (Data_Grid_terrain[gridOffset - 162] & WALL_GATE) {
							adjacent++;
						}
						if (Data_Grid_terrain[gridOffset - 161] & WALL_GATE) {
							adjacent++;
						}
						if (Data_Grid_terrain[gridOffset + 1] & WALL_GATE) {
							adjacent++;
						}
						break;
				}
				if (adjacent == 3) {
					Data_Grid_pathingWalls[gridOffset] = 0;
				} else {
					Data_Grid_pathingWalls[gridOffset] = 0;
				}
			} else if (Data_Grid_terrain[gridOffset] & Terrain_Gatehouse) {
				Data_Grid_pathingWalls[gridOffset] = 0;
			} else {
				Data_Grid_pathingWalls[gridOffset] = -1;
			}
		}
	}
}

void Pathing_clearLandTypeCitizen()
{
	int gridOffset = 0;
	for (int y = 0; y < GRID_SIZE; y++) {
		for (int x = 0; x < GRID_SIZE; x++) {
			Data_Grid_pathingLandCitizen[gridOffset++] = -1;
		}
	}
}

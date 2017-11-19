#include "TerrainBridge.h"

#include "Routing.h"
#include "Terrain.h"

#include "Data/Constants.h"
#include "Data/Grid.h"
#include "Data/State.h"

struct TerrainBridge {
	int startGridOffset;
	int endGridOffset;
	int length;
	int directionGridOffset;
	int direction;
} bridge;

#define IS_BRIDGE(g) ((Data_Grid_terrain[(g)] & Terrain_Water) && Data_Grid_spriteOffsets[(g)])

static int getDirectionYBridgeTiles(int gridOffset)
{
	int dirY = 0;
	// Y direction
	if (IS_BRIDGE(gridOffset-162)) {
		dirY++;
	}
	if (IS_BRIDGE(gridOffset-324)) {
		dirY++;
	}
	if (IS_BRIDGE(gridOffset+162)) {
		dirY++;
	}
	if (IS_BRIDGE(gridOffset+324)) {
		dirY++;
	}
	return dirY;
}

static int getDirectionXBridgeTiles(int gridOffset)
{
	int dirX = 0;
	// X direction
	if (IS_BRIDGE(gridOffset-1)) {
		dirX++;
	}
	if (IS_BRIDGE(gridOffset-2)) {
		dirX++;
	}
	if (IS_BRIDGE(gridOffset+1)) {
		dirX++;
	}
	if (IS_BRIDGE(gridOffset+2)) {
		dirX++;
	}
	return dirX;
}

int TerrainBridge_determineLengthAndDirection(int x, int y, int isShipBridge, int *length, int *direction)
{
	int gridOffset = GridOffset(x, y);
	bridge.startGridOffset = gridOffset;
	bridge.endGridOffset = 0;
	bridge.length = *length = 0;
	bridge.directionGridOffset = 0;
	bridge.direction = *direction = 0;
	int terrain = Data_Grid_terrain[gridOffset];
	if (!(terrain & Terrain_Water)) {
		return 0;
	}
	if (terrain & Terrain_Road || terrain & Terrain_Building) {
		return 0;
	}
	if (Terrain_countTerrainTypeDirectlyAdjacentTo(gridOffset, Terrain_Water) != 3) {
		return 0;
	}
	if (!(Data_Grid_terrain[GridOffset(x, y-1)] & Terrain_Water)) {
		bridge.directionGridOffset = 162;
		bridge.direction = Dir_4_Bottom;
	} else if (!(Data_Grid_terrain[GridOffset(x+1, y)] & Terrain_Water)) {
		bridge.directionGridOffset = -1;
		bridge.direction = Dir_6_Left;
	} else if (!(Data_Grid_terrain[GridOffset(x, y+1)] & Terrain_Water)) {
		bridge.directionGridOffset = -162;
		bridge.direction = Dir_0_Top;
	} else if (!(Data_Grid_terrain[GridOffset(x-1, y)] & Terrain_Water)) {
		bridge.directionGridOffset = 1;
		bridge.direction = Dir_2_Right;
	} else {
		return 0;
	}
	*direction = bridge.direction;
	bridge.length = 1;
	for (int i = 0; i < 40; i++) {
		gridOffset += bridge.directionGridOffset;
		bridge.length++;
		int nextTerrain = Data_Grid_terrain[gridOffset + bridge.directionGridOffset];
		if (nextTerrain & Terrain_Tree) {
			break;
		}
		if (!(nextTerrain & Terrain_Water)) {
			bridge.endGridOffset = gridOffset;
			if (Terrain_countTerrainTypeDirectlyAdjacentTo(gridOffset, Terrain_Water) != 3) {
				bridge.endGridOffset = 0;
			}
			*length = bridge.length;
			return bridge.endGridOffset;
		}
		if (nextTerrain & Terrain_Road || nextTerrain & Terrain_Building) {
			break;
		}
		if (Terrain_countTerrainTypeDirectlyAdjacentTo(gridOffset, Terrain_Water) != 4) {
			break;
		}
	}
	// invalid bridge
	*length = bridge.length;
	return 0;
}

int TerrainBridge_addToSpriteGrid(int x, int y, int isShipBridge)
{
	if (bridge.endGridOffset <= 0 || bridge.length < 2 ||
			(isShipBridge && bridge.length < 5)) {
		bridge.length = 0;
		return bridge.length;
	}
	
	int pillarDistance = 0;
	bridge.direction -= Data_State.map.orientation;
	if (bridge.direction < 0) bridge.direction += 8;
	if (isShipBridge == 1) {
		switch (bridge.length) {
			case  9: pillarDistance = 4; break;
			case 10: pillarDistance = 4; break;
			case 11: pillarDistance = 5; break;
			case 12: pillarDistance = 5; break;
			case 13: pillarDistance = 6; break;
			case 14: pillarDistance = 6; break;
			case 15: pillarDistance = 7; break;
			case 16: pillarDistance = 7; break;
			default: pillarDistance = 8; break;
		}
	}
	
	int gridOffset = GridOffset(x, y);
	for (int i = 0; i < bridge.length; i++) {
		Data_Grid_terrain[gridOffset] |= Terrain_Road;
		int value = 0;
		if (isShipBridge) {
			if (i == 1 || i == bridge.length - 2) {
				// platform after ramp
				value = 13;
			} else if (i == 0) {
				// ramp at start
				switch (bridge.direction) {
					case 0: value = 7; break;
					case 2: value = 8; break;
					case 4: value = 9; break;
					case 6: value = 10; break;
				}
			} else if (i == bridge.length - 1) {
				// ramp at end
				switch (bridge.direction) {
					case 0: value = 9; break;
					case 2: value = 10; break;
					case 4: value = 7; break;
					case 6: value = 8; break;
				}
			} else if (i == pillarDistance) {
				switch (bridge.direction) {
					case 0: value = 14; break;
					case 2: value = 15; break;
					case 4: value = 14; break;
					case 6: value = 15; break;
				}
			} else {
				// middle of the bridge
				switch (bridge.direction) {
					case 0: value = 11; break;
					case 2: value = 12; break;
					case 4: value = 11; break;
					case 6: value = 12; break;
				}
			}
		} else {
			if (i == 0) {
				// ramp at start
				switch (bridge.direction) {
					case 0: value = 1; break;
					case 2: value = 2; break;
					case 4: value = 3; break;
					case 6: value = 4; break;
				}
			} else if (i == bridge.length - 1) {
				// ramp at end
				switch (bridge.direction) {
					case 0: value = 3; break;
					case 2: value = 4; break;
					case 4: value = 1; break;
					case 6: value = 2; break;
				}
			} else {
				// middle part
				switch (bridge.direction) {
					case 0: value = 5; break;
					case 2: value = 6; break;
					case 4: value = 5; break;
					case 6: value = 6; break;
				}
			}
		}
		Data_Grid_spriteOffsets[gridOffset] = value;
		gridOffset += bridge.directionGridOffset;
	}
	
	Routing_determineLandCitizen();
	Routing_determineLandNonCitizen();
	Routing_determineWater();

	return bridge.length;
}

void TerrainBridge_removeFromSpriteGrid(int gridOffset, int onlyMarkDeleted)
{
	if (!(Data_Grid_terrain[gridOffset] & Terrain_Water) ||
			Data_Grid_spriteOffsets[gridOffset] <= 0) {
		return;
	}

	int dirX = getDirectionXBridgeTiles(gridOffset);
	int dirY = getDirectionYBridgeTiles(gridOffset);

	int offsetUp = dirX > dirY ? 1 : 162;
	// find lower end of the bridge
	while ((Data_Grid_terrain[gridOffset - offsetUp] & Terrain_Water) &&
			Data_Grid_spriteOffsets[gridOffset - offsetUp]) {
		gridOffset -= offsetUp;
	}
	
	if (onlyMarkDeleted) {
		Data_Grid_bitfields[gridOffset] |= Bitfield_Deleted;
	} else {
		Data_Grid_spriteOffsets[gridOffset] = 0;
		Data_Grid_terrain[gridOffset] &= ~Terrain_Road;
	}
	while ((Data_Grid_terrain[gridOffset + offsetUp] & Terrain_Water) &&
			Data_Grid_spriteOffsets[gridOffset + offsetUp]) {
		gridOffset += offsetUp;
		if (onlyMarkDeleted) {
			Data_Grid_bitfields[gridOffset] |= Bitfield_Deleted;
		} else {
			Data_Grid_spriteOffsets[gridOffset] = 0;
			Data_Grid_terrain[gridOffset] &= ~Terrain_Road;
		}
	}
}

int TerrainBridge_countFiguresOnBridge(int gridOffset)
{
	if (!(Data_Grid_terrain[gridOffset] & Terrain_Water) ||
		Data_Grid_spriteOffsets[gridOffset] <= 0) {
		return 0;
	}
	int dirX = getDirectionXBridgeTiles(gridOffset);
	int dirY = getDirectionYBridgeTiles(gridOffset);
	
	int offsetUp = dirX > dirY ? 1 : 162;
	// find lower end of the bridge
	while ((Data_Grid_terrain[gridOffset - offsetUp] & Terrain_Water) &&
			Data_Grid_spriteOffsets[gridOffset - offsetUp]) {
		gridOffset -= offsetUp;
	}
	
	int figures = 0;
	if (Data_Grid_figureIds[gridOffset]) {
		figures = 1;
	}
	Data_Grid_bitfields[gridOffset] &= Bitfield_NoDeleted;
	while ((Data_Grid_terrain[gridOffset + offsetUp] & Terrain_Water) &&
			Data_Grid_spriteOffsets[gridOffset + offsetUp]) {
		gridOffset += offsetUp;
		Data_Grid_bitfields[gridOffset] &= Bitfield_NoDeleted;
		if (Data_Grid_figureIds[gridOffset]) {
			figures++;
		}
	}
	return figures;
}

void TerrainBridge_updateSpriteIdsOnMapRotate(int ccw)
{
	for (int y = 0; y < Data_State.map.height; y++) {
		for (int x = 0; x < Data_State.map.width; x++) {
			int gridOffset = GridOffset(x, y);
			if ((Data_Grid_terrain[gridOffset] & Terrain_Water) && Data_Grid_spriteOffsets[gridOffset]) {
				int newValue;
				switch (Data_Grid_spriteOffsets[gridOffset]) {
					case 1: newValue = ccw ? 2 : 4; break;
					case 2: newValue = ccw ? 3 : 1; break;
					case 3: newValue = ccw ? 4 : 2; break;
					case 4: newValue = ccw ? 1 : 3; break;
					case 5: newValue = 6; break;
					case 6: newValue = 5; break;
					case 7: newValue = ccw ? 8 : 10; break;
					case 8: newValue = ccw ? 9 : 7; break;
					case 9: newValue = ccw ? 10 : 8; break;
					case 10: newValue = ccw ? 7 : 9; break;
					case 11: newValue = 12; break;
					case 12: newValue = 11; break;
					case 13: newValue = 13; break;
					case 14: newValue = 15; break;
					case 15: newValue = 14; break;
					default: newValue = Data_Grid_spriteOffsets[gridOffset];
				}
				Data_Grid_spriteOffsets[gridOffset] = newValue;
			}
		}
	}
}

int TerrainBridge_getLength()
{
	return bridge.length;
}

void TerrainBridge_resetLength()
{
	bridge.length = 0;
}

#include "Building.h"
#include "Formation.h"
#include "PlayerWarning.h"
#include "Terrain.h"
#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Grid.h"
#include "Data/Settings.h"

#include <string.h>

struct BuildingProperties {
	int size;
	int unknown;
	int graphicCategory;
	int graphicOffset;
} buildingProperties[] = {
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{1, 0, 0x70, 0},
	{1, 0, 0x18, 0x1A},
	{1, 0, 0, 0},
	{1, 0, 0x13, 2},
	{0, 0, 0, 0},
	{1, 0, 0, 0},
	{1, 0, 0, 0},
	{1, 0, 0, 0},
	{1, 0, 0, 0},
	{1, 0, 0, 0},
	{1, 0, 0, 0},
	{1, 0, 0, 0},
	{1, 0, 0, 0},
	{1, 0, 0, 0},
	{1, 0, 0, 0},
	{2, 0, 0, 0},
	{2, 0, 0, 0},
	{2, 0, 0, 0},
	{2, 0, 0, 0},
	{3, 0, 0, 0},
	{3, 0, 0, 0},
	{3, 0, 0, 0},
	{3, 0, 0, 0},
	{4, 0, 0, 0},
	{4, 0, 0, 0},
	{3, 0, 0x2D, 0},
	{2, 0, 0x2E, 0},
	{5, 0, 0x0D5, 0},
	{5, 0, 0x30, 0},
	{3, 0, 0x31, 0},
	{3, 0, 0x32, 0},
	{3, 0, 0x33, 0},
	{3, 0, 0x34, 0},
	{1, 1, 0x3A, 0},
	{1, 1, 0x3B, 0},
	{3, 1, 0x42, 0},
	{1, 1, 0x3D, 0},
	{2, 1, 0x3D, 1},
	{3, 1, 0x3D, 2},
	{3, 1, 0x42, 0},
	{3, 1, 0x42, 0},
	{1, 0, 0x44, 0},
	{3, 0, 0x46, 0},
	{2, 0, 0x0B9, 0},
	{1, 0, 0x43, 0},
	{3, 0, 0x42, 0},
	{2, 0, 0x29, 0},
	{3, 0, 0x2B, 0},
	{2, 0, 0x2A, 0},
	{4, 1, 0x42, 1},
	{1, 0, 0x40, 0},
	{3, 1, 0x0CD, 0},
	{3, 1, 0x42, 0},
	{2, 1, 0x11, 1},
	{2, 1, 0x11, 0},
	{2, 0, 0x47, 0},
	{2, 0, 0x48, 0},
	{2, 0, 0x49, 0},
	{2, 0, 0x4A, 0},
	{2, 0, 0x4B, 0},
	{3, 0, 0x47, 1},
	{3, 0, 0x48, 1},
	{3, 0, 0x49, 1},
	{3, 0, 0x4A, 1},
	{3, 0, 0x4B, 1},
	{2, 0, 0x16, 0},
	{3, 0, 0x63, 0},
	{1, 1, 0x52, 0},
	{1, 1, 0x52, 0},
	{2, 0, 0x4D, 0},
	{3, 0, 0x4E, 0},
	{2, 0, 0x4F, 0},
	{3, 0, 0x55, 0},
	{4, 0, 0x56, 0},
	{5, 0, 0x57, 0},
	{2, 1, 0x0B8, 0},
	{1, 1, 0x51, 0},
	{1, 1, 0, 0},
	{1, 1, 0, 0},
	{0, 0, 0, 0},
	{5, 0, 0x3E, 0},
	{2, 0, 0x3F, 0},
	{0, 0, 0, 0},
	{1, 1, 0x0B7, 0},
	{2, 1, 0x0B7, 2},
	{3, 1, 0x19, 0},
	{1, 1, 0x36, 0},
	{1, 1, 0x17, 0},
	{1, 1, 0x64, 0},
	{3, 0, 0x0C9, 0},
	{3, 0, 0x0A6, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{2, 0, 0x4C, 0},
	{1, 1, 0, 0},
	{3, 0, 0x25, 0},
	{3, 0, 0x25, 0},
	{3, 0, 0x25, 0},
	{3, 0, 0x25, 0},
	{3, 0, 0x25, 0},
	{3, 0, 0x25, 0},
	{2, 0, 0x26, 0},
	{2, 0, 0x27, 0},
	{2, 0, 0x41, 0},
	{2, 0, 0x28, 0},
	{2, 0, 0x2C, 0},
	{2, 0, 0x7A, 0},
	{2, 0, 0x7B, 0},
	{2, 0, 0x7C, 0},
	{2, 0, 0x7D, 0},
	{0, 0, 0, 0},
	{1, 1, 0, 0},
	{1, 1, 0, 0},
	{1, 1, 0, 0},
	{1, 1, 0, 0},
	{1, 1, 0, 0},
	{1, 1, 0, 0},
	{1, 1, 0, 0},
	{1, 1, 0, 0},
	{1, 1, 0, 0},
	{1, 1, 0, 0},
	{1, 1, 0, 0},
	{1, 1, 0, 0},
	{1, 1, 0, 0},
	{2, 1, 0x0D8, 0},
	{1, 1, 0, 0},
	{1, 1, 0, 0},
	{1, 1, 0, 0},
	{1, 1, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0}
};

void Building_clearList()
{
	memset(Data_Buildings, MAX_BUILDINGS * sizeof(struct Data_Building), 0);
	Data_Buildings_Extra.highestBuildingIdSeen = 0;
	Data_Buildings_Extra.placedSequence = 0;
}

static int isBuildingOnUndoList(int buildingId)
{
	// TODO move to undo stuff?
	return 0;
}

int Building_create(int type, int x, int y)
{
	int buildingId = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (!Data_Buildings[i].inUse && !isBuildingOnUndoList(i)) {
			buildingId = i;
			break;
		}
	}
	if (!buildingId) {
		PlayerWarning_show(PlayerWarning_DataLimitReached);
		return 0;
	}
	
	struct Data_Building *b = &Data_Buildings[buildingId];
	
	b->inUse = 3;
	b->__unknown_01 = 1;
	b->__unknown_02 = Data_CityInfo.__unknown_00a5; // TODO ??
	b->type = type;
	b->size = buildingProperties[type].size;
	b->placedSequence = Data_Buildings_Extra.placedSequence++;
	b->sentiment.houseHappiness = 50;
	b->__unknown_1a = 0;
	
	// house size
	b->houseSize = 0;
	if (type >= Building_HouseSmallTent && type <= Building_HouseMediumInsula) {
		b->houseSize = 1;
	} else if (type >= Building_HouseLargeInsula && type <= Building_HouseMediumVilla) {
		b->houseSize = 2;
	} else if (type >= Building_HouseLargeVilla && type <= Building_HouseMediumPalace) {
		b->houseSize = 3;
	} else if (type >= Building_HouseLargePalace && type <= Building_HouseLuxuryPalace) {
		b->houseSize = 4;
	}
	
	// subtype
	if (type >= Building_HouseSmallTent && type <= Building_HouseLuxuryPalace) {
		b->subtype.houseLevel = type - 10;
	} else {
		b->subtype.houseLevel = 0;
	}
	
	// input/output resources
	switch (type) {
		case Building_WheatFarm:
			b->outputResourceId = Resource_Wheat;
			break;
		case Building_VegetableFarm:
			b->outputResourceId = Resource_Vegetables;
			break;
		case Building_FruitFarm:
			b->outputResourceId = Resource_Fruit;
			break;
		case Building_OliveFarm:
			b->outputResourceId = Resource_Olives;
			break;
		case Building_VinesFarm:
			b->outputResourceId = Resource_Vines;
			break;
		case Building_PigFarm:
			b->outputResourceId = Resource_Meat;
			break;
		case Building_MarbleQuarry:
			b->outputResourceId = Resource_Marble;
			break;
		case Building_IronMine:
			b->outputResourceId = Resource_Iron;
			break;
		case Building_TimberYard:
			b->outputResourceId = Resource_Timber;
			break;
		case Building_ClayPit:
			b->outputResourceId = Resource_Clay;
			break;
		case Building_WineWorkshop:
			b->outputResourceId = Resource_Wine;
			b->subtype.warehouseResourceId = 2;
			break;
		case Building_OilWorkshop:
			b->outputResourceId = Resource_Oil;
			b->subtype.warehouseResourceId = 1;
			break;
		case Building_WeaponsWorkshop:
			b->outputResourceId = Resource_Weapons;
			b->subtype.warehouseResourceId = 3;
			break;
		case Building_FurnitureWorkshop:
			b->outputResourceId = Resource_Furniture;
			b->subtype.warehouseResourceId = 4;
			break;
		case Building_PotteryWorkshop:
			b->outputResourceId = Resource_Pottery;
			b->subtype.warehouseResourceId = 5;
			break;
		default:
			b->outputResourceId = Resource_None;
			break;
	}
	
	if (type == Building_Granary) {
		b->data.granary.spaceLeft = 2400;
	}
	
	b->x = x;
	b->y = y;
	b->gridOffset = 162 * y + x + Data_Settings_Map.gridStartOffset;
	b->__unknown_2c = b->__unknown_45 = Data_Grid_random[b->gridOffset] & 0x7f;
	b->__unknown_44 = buildingProperties[type].unknown;
	b->isAdjacentToWater = Terrain_isAdjacentToWater(x, y, b->size);

	return buildingId;
}

void Building_delete(int buildingId)
{
	Building_deleteData(buildingId);
	memset(&Data_Buildings[buildingId], sizeof(struct Data_Building), 0);
}

void Building_deleteData(int buildingId)
{
	struct Data_Building *b = &Data_Buildings[buildingId];
	if (b->storageId) {
		Data_Building_Storages[b->storageId].inUse = 0;
	}
	if (b->type == Building_SenateUpgraded && b->gridOffset == Data_CityInfo.buildingSenateGridOffset) {
		Data_CityInfo.buildingSenateGridOffset = 0;
		Data_CityInfo.buildingSenateX = 0;
		Data_CityInfo.buildingSenateY = 0;
		Data_CityInfo.buildingSenatePlaced = 0;
	}
	if (b->type == Building_Dock) {
		--Data_CityInfo.numWorkingDocks;
	}
	if (b->type == Building_Barracks && b->gridOffset == Data_CityInfo.buildingBarracksGridOffset) {
		Data_CityInfo.buildingBarracksGridOffset = 0;
		Data_CityInfo.buildingBarracksX = 0;
		Data_CityInfo.buildingBarracksY = 0;
		Data_CityInfo.buildingBarracksPlaced = 0;
	}
	if (b->type == Building_DistributionCenter_Unused && b->gridOffset == Data_CityInfo.buildingDistributionCenterGridOffset) {
		Data_CityInfo.buildingDistributionCenterGridOffset = 0;
		Data_CityInfo.buildingDistributionCenterX = 0;
		Data_CityInfo.buildingDistributionCenterY = 0;
		Data_CityInfo.buildingDistributionCenterPlaced = 0;
	}
	if (b->type == Building_FortGround__) {
		Formation_deleteFortAndBanner(b->formationId);
	}
	if (b->type == Building_Hippodrome) {
		Data_CityInfo.buildingHippodromePlaced = 0;
	}
}

void BuildingStorage_clearList()
{
	memset(Data_Building_Storages, MAX_STORAGES * sizeof(struct Data_Building_Storage), 0);
}

int BuildingStorage_create()
{
	for (int i = 1; i < MAX_STORAGES; i++) {
		if (!Data_Building_Storages[i].inUse) {
			memset(&Data_Building_Storages[i], sizeof(struct Data_Building_Storage), 0);
			Data_Building_Storages[i].inUse = 1;
			return i;
		}
	}
	return 0;
}

void BuildingStorage_resetBuildingIds()
{
	for (int i = 1; i < MAX_STORAGES; i++) {
		Data_Building_Storages[i].buildingId = 0;
	}
	
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (!Data_Buildings[i].inUse) {
			continue;
		}
		if (Data_Buildings[i].type == Building_Granary || Data_Buildings[i].type == Building_Warehouse) {
			if (Data_Buildings[i].storageId) {
				int storageId = Data_Buildings[i].storageId;
				if (Data_Building_Storages[storageId].buildingId) {
					// is already connected to a building, create new
					// FIXME: not connected to building id
					Data_Buildings[i].storageId = BuildingStorage_create();
				} else {
					Data_Building_Storages[storageId].buildingId = i;
				}
			}
		}
	}
}

int Building_Market_getMaxFoodStock(int buildingId)
{
	int maxStock = 0;
	if (buildingId > 0 && Data_Buildings[buildingId].type == Building_Market) {
		for (int i = 0; i < 4; i++) {
			if (Data_Buildings[buildingId].data.market.food[i] > maxStock) {
				maxStock = Data_Buildings[buildingId].data.market.food[i];
			}
		}
	}
	return maxStock;
}

int Building_Market_getMaxGoodsStock(int buildingId)
{
	int maxStock = 0;
	if (buildingId > 0 && Data_Buildings[buildingId].type == Building_Market) {
		if (Data_Buildings[buildingId].data.market.pottery > maxStock) {
			maxStock = Data_Buildings[buildingId].data.market.pottery;
		}
		if (Data_Buildings[buildingId].data.market.furniture > maxStock) {
			maxStock = Data_Buildings[buildingId].data.market.furniture;
		}
		if (Data_Buildings[buildingId].data.market.oil > maxStock) {
			maxStock = Data_Buildings[buildingId].data.market.oil;
		}
		if (Data_Buildings[buildingId].data.market.wine > maxStock) {
			maxStock = Data_Buildings[buildingId].data.market.wine;
		}
	}
	return maxStock;
}

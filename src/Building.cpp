#include "Building.h"
#include "Data/Building.h"

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

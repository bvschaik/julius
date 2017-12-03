#include "Security.h"

#include "Building.h"
#include "core/calc.h"
#include "CityInfo.h"
#include "Figure.h"
#include "FigureAction.h"
#include "Formation.h"
#include "SidebarMenu.h"
#include "Terrain.h"
#include "TerrainGraphics.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/State.h"

#include "building/list.h"
#include "city/message.h"
#include "core/random.h"
#include "figure/figure.h"
#include "figure/type.h"
#include "game/tutorial.h"
#include "map/building.h"
#include "map/grid.h"
#include "map/random.h"
#include "map/routing_terrain.h"
#include "scenario/property.h"
#include "sound/effect.h"

static int burningRuinSpreadDirection;

void Security_Tick_updateFireSpreadDirection()
{
	burningRuinSpreadDirection = random_byte() & 7;
}

void Security_Tick_updateBurningRuins()
{
    scenario_climate climate = scenario_property_climate();
	int recalculateTerrain = 0;
	building_list_burning_clear();
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		struct Data_Building *b = &Data_Buildings[i];
		if (!BuildingIsInUse(i) || b->type != BUILDING_BURNING_RUIN) {
			continue;
		}
		if (b->fireDuration < 0) {
			b->fireDuration = 0;
		}
		b->fireDuration++;
		if (b->fireDuration > 32) {
			Data_State.undoAvailable = 0;
			b->state = BuildingState_Rubble;
			TerrainGraphics_setBuildingAreaRubble(i, b->x, b->y, b->size);
			recalculateTerrain = 1;
			continue;
		}
		if (b->ruinHasPlague) {
			continue;
		}
		building_list_burning_add(i);
		if (climate == CLIMATE_DESERT) {
			if (b->fireDuration & 3) { // check spread every 4 ticks
				continue;
			}
		} else {
			if (b->fireDuration & 7) { // check spread every 8 ticks
				continue;
			}
		}
		if ((b->houseGenerationDelay & 3) != (random_byte() & 3)) {
			continue;
		}
		int dir1 = burningRuinSpreadDirection - 1;
		if (dir1 < 0) dir1 = 7;
		int dir2 = burningRuinSpreadDirection + 1;
		if (dir2 > 7) dir2 = 0;
		
		int gridOffset = b->gridOffset;
		int nextBuildingId = map_building_at(gridOffset + map_grid_direction_delta(burningRuinSpreadDirection));
		if (nextBuildingId && !Data_Buildings[nextBuildingId].fireProof) {
			Building_collapseOnFire(nextBuildingId, 0);
			Building_collapseLinked(nextBuildingId, 1);
			sound_effect_play(SOUND_EFFECT_EXPLOSION);
			recalculateTerrain = 1;
		} else {
			nextBuildingId = map_building_at(gridOffset + map_grid_direction_delta(dir1));
			if (nextBuildingId && !Data_Buildings[nextBuildingId].fireProof) {
				Building_collapseOnFire(nextBuildingId, 0);
				Building_collapseLinked(nextBuildingId, 1);
				sound_effect_play(SOUND_EFFECT_EXPLOSION);
				recalculateTerrain = 1;
			} else {
				nextBuildingId = map_building_at(gridOffset + map_grid_direction_delta(dir2));
				if (nextBuildingId && !Data_Buildings[nextBuildingId].fireProof) {
					Building_collapseOnFire(nextBuildingId, 0);
					Building_collapseLinked(nextBuildingId, 1);
					sound_effect_play(SOUND_EFFECT_EXPLOSION);
					recalculateTerrain = 1;
				}
			}
		}
	}
	if (recalculateTerrain) {
		map_routing_update_land();
	}
}

int Security_Fire_getClosestBurningRuin(int x, int y, int *distance)
{
	int minFreeBuildingId = 0;
	int minOccupiedBuildingId = 0;
	int minOccupiedDist = *distance = 10000;

	const int *burning = building_list_burning_items();
	int burning_size = building_list_burning_size();
	for (int i = 0; i < burning_size; i++) {
		int buildingId = burning[i];
		struct Data_Building *b = &Data_Buildings[buildingId];
		if (BuildingIsInUse(buildingId) && b->type == BUILDING_BURNING_RUIN && !b->ruinHasPlague && b->distanceFromEntry) {
			int dist = calc_maximum_distance(x, y, b->x, b->y);
			if (b->figureId4) {
				if (dist < minOccupiedDist) {
					minOccupiedDist = dist;
					minOccupiedBuildingId = buildingId;
				}
			} else if (dist < *distance) {
				*distance = dist;
				minFreeBuildingId = buildingId;
			}
		}
	}
	if (!minFreeBuildingId && minOccupiedDist <= 2) {
		minFreeBuildingId = minOccupiedBuildingId;
		*distance = 2;
	}
	return minFreeBuildingId;
}

static void generateRioter(int buildingId)
{
	int xRoad, yRoad;
	struct Data_Building *b = &Data_Buildings[buildingId];
	if (!Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 4, &xRoad, &yRoad)) {
		return;
	}
	Data_CityInfo.numCriminalsThisMonth++;
	int peopleInMob;
	if (Data_CityInfo.population <= 150) {
		peopleInMob = 1;
	} else if (Data_CityInfo.population <= 300) {
		peopleInMob = 2;
	} else if (Data_CityInfo.population <= 800) {
		peopleInMob = 3;
	} else if (Data_CityInfo.population <= 1200) {
		peopleInMob = 4;
	} else if (Data_CityInfo.population <= 2000) {
		peopleInMob = 5;
	} else {
		peopleInMob = 6;
	}
	int targetX, targetY;
	int targetBuildingId = Formation_Rioter_getTargetBuilding(&targetX, &targetY);
	for (int i = 0; i < peopleInMob; i++) {
		figure *f = figure_create(FIGURE_RIOTER, xRoad, yRoad, DIR_4_BOTTOM);
		f->actionState = FigureActionState_120_RioterCreated;
		f->roamLength = 0;
		f->waitTicks = 10 + 4 * i;
		if (targetBuildingId) {
			f->destinationX = targetX;
			f->destinationY = targetY;
			f->destinationBuildingId = targetBuildingId;
		} else {
			f->state = FigureState_Dead;
		}
	}
	Building_collapseOnFire(buildingId, 0);
	Data_CityInfo.ratingPeaceNumRiotersThisYear++;
	Data_CityInfo.riotCause = Data_CityInfo.populationEmigrationCause;
	CityInfo_Population_changeHappiness(20);
    tutorial_on_crime();
    game.messages.apply_sound_interval(MESSAGE_CAT_RIOT);
  game.messages.post_with_popup_delay(MESSAGE_CAT_RIOT, MESSAGE_RIOT, b->type, map_grid_offset(xRoad, yRoad));
}

static void generateMugger(int buildingId)
{
	Data_CityInfo.numCriminalsThisMonth++;
	struct Data_Building *b = &Data_Buildings[buildingId];
	if (b->houseCriminalActive < 2) {
		b->houseCriminalActive = 2;
		int xRoad, yRoad;
		if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
			figure *f = figure_create(FIGURE_CRIMINAL, xRoad, yRoad, DIR_4_BOTTOM);
			f->waitTicks = 10 + (b->houseGenerationDelay & 0xf);
			Data_CityInfo.ratingPeaceNumCriminalsThisYear++;
			if (Data_CityInfo.financeTaxesThisYear > 20) {
				int moneyStolen = Data_CityInfo.financeTaxesThisYear / 4;
				if (moneyStolen > 400) {
					moneyStolen = 400 - random_byte() / 2;
				}
				game.messages.post(1, MESSAGE_THEFT, moneyStolen, f->gridOffset);
				Data_CityInfo.financeStolenThisYear += moneyStolen;
				Data_CityInfo.treasury -= moneyStolen;
				Data_CityInfo.financeSundriesThisYear += moneyStolen;
			}
		}
	}
}

static void generateProtester(int buildingId)
{
	Data_CityInfo.numProtestersThisMonth++;
	struct Data_Building *b = &Data_Buildings[buildingId];
	if (b->houseCriminalActive < 1) {
		b->houseCriminalActive = 1;
		int xRoad, yRoad;
		if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
			figure *f = figure_create(FIGURE_PROTESTER, xRoad, yRoad, DIR_4_BOTTOM);
			f->waitTicks = 10 + (b->houseGenerationDelay & 0xf);
			Data_CityInfo.ratingPeaceNumCriminalsThisYear++;
		}
	}
}

void Security_Tick_generateCriminal()
{
	int minBuildingId = 0;
	int minHappiness = 50;
	for (int i = 1; i <= Data_Buildings_Extra.highestBuildingIdInUse; i++) {
		struct Data_Building *b = &Data_Buildings[i];
		if (BuildingIsInUse(i) && b->houseSize) {
			if (b->sentiment.houseHappiness >= 50) {
				b->houseCriminalActive = 0;
			} else if (b->sentiment.houseHappiness < minHappiness) {
				minHappiness = b->sentiment.houseHappiness;
				minBuildingId = i;
			}
		}
	}
	if (minBuildingId) {
		if (scenario_is_tutorial_1() || scenario_is_tutorial_2()) {
			return;
		}
		if (Data_CityInfo.citySentiment < 30) {
			if (random_byte() >= Data_CityInfo.citySentiment + 50) {
				if (minHappiness <= 10) {
					generateRioter(minBuildingId);
				} else if (minHappiness < 30) {
					generateMugger(minBuildingId);
				} else if (minHappiness < 50) {
					generateProtester(minBuildingId);
				}
			}
		} else if (Data_CityInfo.citySentiment < 60) {
			if (random_byte() >= Data_CityInfo.citySentiment + 40) {
				if (minHappiness < 30) {
					generateMugger(minBuildingId);
				} else if (minHappiness < 50) {
					generateProtester(minBuildingId);
				}
			}
		} else {
			if (random_byte() >= Data_CityInfo.citySentiment + 20) {
				if (minHappiness < 50) {
					generateProtester(minBuildingId);
				}
			}
		}
	}
}

static void collapseBuilding(int buildingId, struct Data_Building *b)
{
	game.messages.apply_sound_interval(MESSAGE_CAT_COLLAPSE);
	if (!tutorial_handle_collapse()) {
		game.messages.post_with_popup_delay(MESSAGE_CAT_COLLAPSE, MESSAGE_COLLAPSED_BUILDING, b->type, b->gridOffset);
	}
	
	Data_State.undoAvailable = 0;
	b->state = BuildingState_Rubble;
	TerrainGraphics_setBuildingAreaRubble(buildingId, b->x, b->y, b->size);
	Figure_createDustCloud(b->x, b->y, b->size);
	Building_collapseLinked(buildingId, 0);
}

static void fireBuilding(int buildingId, struct Data_Building *b)
{
	game.messages.apply_sound_interval(MESSAGE_CAT_FIRE);
	if (!tutorial_handle_fire()) {
		game.messages.post_with_popup_delay(MESSAGE_CAT_FIRE, MESSAGE_FIRE, b->type, b->gridOffset);
	}
	
	Building_collapseOnFire(buildingId, 0);
	Building_collapseLinked(buildingId, 1);
	sound_effect_play(SOUND_EFFECT_EXPLOSION);
}

void Security_Tick_checkFireCollapse()
{
	Data_CityInfo.numProtestersThisMonth = 0;
	Data_CityInfo.numCriminalsThisMonth = 0;

	scenario_climate climate = scenario_property_climate();
	int recalculateTerrain = 0;
	int randomGlobal = random_byte() & 7;
	for (int i = 1; i <= Data_Buildings_Extra.highestBuildingIdInUse; i++) {
		struct Data_Building *b = &Data_Buildings[i];
		if (!BuildingIsInUse(i) || b->fireProof) {
			continue;
		}
		if (b->type == BUILDING_HIPPODROME && b->prevPartBuildingId) {
			continue;
		}
		int randomBuilding = (i + map_random_get(b->gridOffset)) & 7;
		// damage
		b->damageRisk += (randomBuilding == randomGlobal) ? 3 : 1;
		if (tutorial_extra_damage_risk()) {
			b->damageRisk += 5;
		}
		if (b->houseSize && b->subtype.houseLevel <= HOUSE_LARGE_TENT) {
			b->damageRisk = 0;
		}
		if (b->damageRisk > 200) {
			collapseBuilding(i, b);
			recalculateTerrain = 1;
			continue;
		}
		// fire
		if (randomBuilding == randomGlobal) {
			if (!b->houseSize) {
				b->fireRisk += 5;
			} else if (b->housePopulation <= 0) {
				b->fireRisk = 0;
			} else if (b->subtype.houseLevel <= HOUSE_LARGE_SHACK) {
				b->fireRisk += 10;
			} else if (b->subtype.houseLevel <= HOUSE_GRAND_INSULA) {
				b->fireRisk += 5;
			} else {
				b->fireRisk += 2;
			}
			if (tutorial_extra_fire_risk()) {
				b->fireRisk += 5;
			}
			if (climate == CLIMATE_NORTHERN) {
				b->fireRisk = 0;
			} else if (climate == CLIMATE_DESERT) {
				b->fireRisk += 3;
			}
		}
		if (b->fireRisk > 100) {
			fireBuilding(i, b);
			recalculateTerrain = 1;
		}
	}
	
	if (recalculateTerrain) {
		map_routing_update_land();
	}
}

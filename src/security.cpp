#include "security.h"

#include "building.h"
#include "core/calc.h"
#include "cityinfo.h"
#include "figure.h"
#include "figureaction.h"
#include "formation.h"
#include "playermessage.h"
#include "routing.h"
#include "sidebarmenu.h"
#include "sound.h"
#include "terrain.h"
#include "terraingraphics.h"
#include "tutorial.h"

#include "data/building.hpp"
#include "data/cityinfo.hpp"
#include "data/constants.hpp"
#include "data/grid.hpp"
#include "data/message.hpp"
#include "data/scenario.hpp"
#include "data/settings.hpp"
#include "data/state.hpp"
#include "data/tutorial.hpp"
#include "data/figure.hpp"

#include "core/random.h"
#include "core/time.h"
#include "figure/type.h"

#define EACH_BURNING_RUIN Data_BuildingList.burning.index = 0; Data_BuildingList.burning.index < Data_BuildingList.burning.size; Data_BuildingList.burning.index++

static int burningRuinSpreadDirection;

void Security_Tick_updateFireSpreadDirection()
{
	burningRuinSpreadDirection = random_byte() & 7;
}

void Security_Tick_updateBurningRuins()
{
	int recalculateTerrain = 0;
	Data_BuildingList.burning.index = 0;
	Data_BuildingList.burning.size = 0;
	Data_BuildingList.burning.totalBurning = 0;
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
		Data_BuildingList.burning.totalBurning++;
		Data_BuildingList.burning.items[Data_BuildingList.burning.size++] = i;
		if (Data_BuildingList.burning.size >= 500) {
			Data_BuildingList.burning.size = 499;
		}
		if (Data_Scenario.climate == Climate_Desert) {
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
		int nextBuildingId = Data_Grid_buildingIds[gridOffset + Constant_DirectionGridOffsets[burningRuinSpreadDirection]];
		if (nextBuildingId && !Data_Buildings[nextBuildingId].fireProof) {
			Building_collapseOnFire(nextBuildingId, 0);
			Building_collapseLinked(nextBuildingId, 1);
			Sound_Effects_playChannel(SoundChannel_Explosion);
			recalculateTerrain = 1;
		} else {
			nextBuildingId = Data_Grid_buildingIds[gridOffset + Constant_DirectionGridOffsets[dir1]];
			if (nextBuildingId && !Data_Buildings[nextBuildingId].fireProof) {
				Building_collapseOnFire(nextBuildingId, 0);
				Building_collapseLinked(nextBuildingId, 1);
				Sound_Effects_playChannel(SoundChannel_Explosion);
				recalculateTerrain = 1;
			} else {
				nextBuildingId = Data_Grid_buildingIds[gridOffset + Constant_DirectionGridOffsets[dir2]];
				if (nextBuildingId && !Data_Buildings[nextBuildingId].fireProof) {
					Building_collapseOnFire(nextBuildingId, 0);
					Building_collapseLinked(nextBuildingId, 1);
					Sound_Effects_playChannel(SoundChannel_Explosion);
					recalculateTerrain = 1;
				}
			}
		}
	}
	if (recalculateTerrain) {
		Routing_determineLandCitizen();
		Routing_determineLandNonCitizen();
	}
}

int Security_Fire_getClosestBurningRuin(int x, int y, int *distance)
{
	int minFreeBuildingId = 0;
	int minOccupiedBuildingId = 0;
	int minOccupiedDist = *distance = 10000;
	for (EACH_BURNING_RUIN) {
		int buildingId = Data_BuildingList.burning.items[Data_BuildingList.burning.index];
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
		int figureId = Figure_create(FIGURE_RIOTER, xRoad, yRoad, 4);
		struct Data_Figure *f = &Data_Figures[figureId];
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
	if (!Data_Tutorial.tutorial1.crime) {
		Data_Tutorial.tutorial1.crime = 1;
		SidebarMenu_enableBuildingMenuItemsAndButtons();
	}
	if (time_get_millis() <= 15000 + Data_Message.lastSoundTime.rioterGenerated) {
		PlayerMessage_disableSoundForNextMessage();
	} else {
		Data_Message.lastSoundTime.rioterGenerated = time_get_millis();
	}
	PlayerMessage_postWithPopupDelay(MessageDelay_Riot, Message_11_RiotInTheCity, b->type, GridOffset(xRoad, yRoad));
}

static void generateMugger(int buildingId)
{
	Data_CityInfo.numCriminalsThisMonth++;
	struct Data_Building *b = &Data_Buildings[buildingId];
	if (b->houseCriminalActive < 2) {
		b->houseCriminalActive = 2;
		int xRoad, yRoad;
		if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
			int figureId = Figure_create(FIGURE_CRIMINAL, xRoad, yRoad, 4);
			Data_Figures[figureId].waitTicks = 10 + (b->houseGenerationDelay & 0xf);
			Data_CityInfo.ratingPeaceNumCriminalsThisYear++;
			if (Data_CityInfo.financeTaxesThisYear > 20) {
				int moneyStolen = Data_CityInfo.financeTaxesThisYear / 4;
				if (moneyStolen > 400) {
					moneyStolen = 400 - random_byte() / 2;
				}
				PlayerMessage_post(1, Message_52_Theft, moneyStolen, Data_Figures[figureId].gridOffset);
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
			int figureId = Figure_create(FIGURE_PROTESTER, xRoad, yRoad, 4);
			Data_Figures[figureId].waitTicks = 10 + (b->houseGenerationDelay & 0xf);
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
		if (IsTutorial1() || IsTutorial2()) {
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
	if (time_get_millis() - Data_Message.lastSoundTime.collapse <= 15000) {
		PlayerMessage_disableSoundForNextMessage();
	} else {
		Data_Message.lastSoundTime.collapse = time_get_millis();
	}
	if (Data_Tutorial.tutorial1.collapse) {
		PlayerMessage_postWithPopupDelay(MessageDelay_Collapse, Message_13_CollapsedBuilding, b->type, b->gridOffset);
	} else {
		Tutorial_onCollapse();
	}
	
	Data_State.undoAvailable = 0;
	b->state = BuildingState_Rubble;
	TerrainGraphics_setBuildingAreaRubble(buildingId, b->x, b->y, b->size);
	Figure_createDustCloud(b->x, b->y, b->size);
	Building_collapseLinked(buildingId, 0);
}

static void fireBuilding(int buildingId, struct Data_Building *b)
{
	if (time_get_millis() - Data_Message.lastSoundTime.fire <= 15000) {
		PlayerMessage_disableSoundForNextMessage();
	} else {
		Data_Message.lastSoundTime.fire = time_get_millis();
	}
	if (Data_Tutorial.tutorial1.fire) {
		PlayerMessage_postWithPopupDelay(MessageDelay_Fire, Message_12_FireInTheCity, b->type, b->gridOffset);
	} else {
		Tutorial_onFire();
	}
	
	Building_collapseOnFire(buildingId, 0);
	Building_collapseLinked(buildingId, 1);
	Sound_Effects_playChannel(SoundChannel_Explosion);
}

void Security_Tick_checkFireCollapse()
{
	Data_CityInfo.numProtestersThisMonth = 0;
	Data_CityInfo.numCriminalsThisMonth = 0;
	
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
		int randomBuilding = (i + Data_Grid_random[b->gridOffset]) & 7;
		// damage
		b->damageRisk += (randomBuilding == randomGlobal) ? 3 : 1;
		if (Data_Tutorial.tutorial1.fire == 1 && !Data_Tutorial.tutorial1.collapse) {
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
			if (!Data_Tutorial.tutorial1.fire) {
				b->fireRisk += 5;
			}
			if (Data_Scenario.climate == Climate_Northern) {
				b->fireRisk = 0;
			}
			if (Data_Scenario.climate == Climate_Desert) {
				b->fireRisk += 3;
			}
		}
		if (b->fireRisk > 100) {
			fireBuilding(i, b);
			recalculateTerrain = 1;
		}
	}
	
	if (recalculateTerrain) {
		Routing_determineLandCitizen();
		Routing_determineLandNonCitizen();
	}
}

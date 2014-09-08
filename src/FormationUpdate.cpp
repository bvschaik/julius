#include "Formation.h"

#include "Calc.h"
#include "Grid.h"
#include "PlayerMessage.h"
#include "Sound.h"
#include "TerrainGraphics.h"
#include "Walker.h"
#include "WalkerAction.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Formation.h"
#include "Data/Grid.h"
#include "Data/Random.h"
#include "Data/Settings.h"
#include "Data/Walker.h"

static void changeMoraleOfAllLegions(int amount)
{
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		if (Data_Formations[i].inUse == 1 && !Data_Formations[i].isHerd) {
			if (Data_Formations[i].isLegion) {
				Formation_changeMorale(i, amount);
			}
		}
	}
}

static void changeMoraleOfAllEnemies(int amount)
{
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		if (Data_Formations[i].inUse == 1 && !Data_Formations[i].isHerd) {
			if (!Data_Formations[i].isLegion) {
				Formation_changeMorale(i, amount);
			}
		}
	}
}

static void tickDecreaseLegionDamage()
{
	for (int i = 1; i < MAX_WALKERS; i++) {
		struct Data_Walker *w = &Data_Walkers[i];
		if (w->state == WalkerState_Alive && WalkerIsLegion(w->type)) {
			if (w->actionState == WalkerActionState_80_SoldierAtRest) {
				if (w->damage) {
					w->damage--;
				}
			}
		}
	}
}

static void tickUpdateMorale()
{
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		struct Data_Formation *f = &Data_Formations[i];
		if (f->inUse != 1 || f->isHerd) {
			continue;
		}
		if (f->isLegion) {
			if (!f->isAtFort && !f->inDistantBattle) {
				if (f->morale <= 20 && !f->monthsLowMorale && !f->monthsVeryLowMorale) {
					changeMoraleOfAllLegions(-10);
					changeMoraleOfAllEnemies(10);
				}
				if (f->morale <= 10) {
					f->monthsVeryLowMorale++;
				} else if (f->morale <= 20) {
					f->monthsLowMorale++;
				}
			}
		} else { // enemy
			if (f->morale <= 20 && !f->monthsLowMorale && !f->monthsVeryLowMorale) {
				changeMoraleOfAllLegions(10);
				changeMoraleOfAllEnemies(-10);
			}
			if (f->morale <= 10) {
				f->monthsVeryLowMorale++;
			} else if (f->morale <= 20) {
				f->monthsLowMorale++;
			}
		}
	}
}

static void tickUpdateDirection()
{
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		struct Data_Formation *f = &Data_Formations[i];
		if (f->inUse != 1 || f->isHerd) {
			continue;
		}
		if (f->__unknown66) {
			f->__unknown66--;
		} else if (f->missileFired) {
			f->direction = Data_Walkers[f->walkerIds[0]].direction;
		} else if (f->layout == FormationLayout_DoubleLine1 || f->layout == FormationLayout_SingleLine1) {
			if (f->yHome < f->prevYHome) {
				f->direction = 0;
			} else if (f->yHome > f->prevYHome) {
				f->direction = 4;
			}
		} else if (f->layout == FormationLayout_DoubleLine2 || f->layout == FormationLayout_SingleLine2) {
			if (f->xHome < f->prevXHome) {
				f->direction = 6;
			} else if (f->xHome > f->prevXHome) {
				f->direction = 2;
			}
		} else if (f->layout == FormationLayout_Tortoise || f->layout == FormationLayout_Column) {
			int dx = (f->xHome < f->prevXHome) ? (f->prevXHome - f->xHome) : (f->xHome - f->prevXHome);
			int dy = (f->yHome < f->prevYHome) ? (f->prevYHome - f->yHome) : (f->yHome - f->prevYHome);
			if (dx > dy) {
				if (f->xHome < f->prevXHome) {
					f->direction = 6;
				} else if (f->xHome > f->prevXHome) {
					f->direction = 2;
				}
			} else {
				if (f->yHome < f->prevYHome) {
					f->direction = 0;
				} else if (f->yHome > f->prevYHome) {
					f->direction = 4;
				}
			}
		}
		f->prevXHome = f->xHome;
		f->prevYHome = f->yHome;
	}
}

static void tickUpdateLegions()
{
	for (int i = 1; i <= 6; i++) {
		struct Data_Formation *f = &Data_Formations[i];
		if (f->inUse != 1 || !f->isLegion) {
			continue;
		}
		if (f->cursedByMars) {
			f->cursedByMars--;
		}
		if (f->missileFired) {
			f->missileFired--;
		}
		if (f->missileAttackTimeout) {
			f->missileAttackTimeout--;
		}
		if (f->recentFight) {
			f->recentFight--;
		}
		if (Data_CityInfo.numEnemiesInCity <= 0) {
			f->recentFight = 0;
			f->missileAttackTimeout = 0;
			f->missileFired = 0;
		}
		for (int n = 0; n < 16; n++) {
			if (Data_Walkers[f->walkerIds[n]].actionState == WalkerActionState_150_Attack) {
				f->recentFight = 6;
			}
		}
		if (f->monthsLowMorale || f->monthsVeryLowMorale) {
			// flee back to fort
			for (int n = 0; n < 16; n++) {
				struct Data_Walker *w = &Data_Walkers[f->walkerIds[n]];
				if (w->actionState != WalkerActionState_150_Attack &&
					w->actionState != WalkerActionState_149_Corpse &&
					w->actionState != WalkerActionState_148_Fleeing) {
					w->actionState = WalkerActionState_148_Fleeing;
					WalkerRoute_remove(f->walkerIds[n]);
				}
			}
		} else if (f->layout == FormationLayout_MopUp) {
			if (Data_Formation_Extra.numEnemyFormations +
				Data_CityInfo.numRiotersInCity +
				Data_CityInfo.numAttackingNativesInCity > 0) {
				for (int n = 0; n < 16; n++) {
					struct Data_Walker *w = &Data_Walkers[f->walkerIds[n]];
					if (w->actionState != WalkerActionState_150_Attack &&
						w->actionState != WalkerActionState_149_Corpse) {
						w->actionState = WalkerActionState_86_SoldierMoppingUp;
					}
				}
			} else {
				f->layout = f->layoutBeforeMopUp;
			}
		}
	}
}

static void addRomanSoldierConcentration(int x, int y, int radius, int amount)
{
	int xMin = x - radius;
	int yMin = y - radius;
	int xMax = x + radius;
	int yMax = y + radius;
	if (xMin < 0) xMin = 0;
	if (yMin < 0) yMin = 0;
	if (xMax >= Data_Settings_Map.width) xMax = Data_Settings_Map.width - 1;
	if (yMax >= Data_Settings_Map.height) yMax = Data_Settings_Map.height - 1;

	for (int yy = yMin; yy <= yMax; yy++) {
		for (int xx = xMin; xx <= xMax; xx++) {
			int gridOffset = GridOffset(xx, yy);
			Data_Grid_romanSoldierConcentration[gridOffset] += amount;
			if (Data_Grid_walkerIds[gridOffset] > 0) {
				int type = Data_Walkers[Data_Grid_walkerIds[gridOffset]].type;
				if (WalkerIsLegion(type)) {
					Data_Grid_romanSoldierConcentration[gridOffset] += 2;
				}
			}
		}
	}
}

static void calculateRomanSoldierConcentration()
{
	Grid_clearUByteGrid(Data_Grid_romanSoldierConcentration);
	for (int i = 1; i <= 6; i++) {
		struct Data_Formation *f = &Data_Formations[i];
		if (f->inUse != 1 || !f->isLegion) {
			continue;
		}
		if (f->numWalkers > 0) {
			addRomanSoldierConcentration(f->xHome, f->yHome, 7, 1);
		}
		if (f->numWalkers > 3) {
			addRomanSoldierConcentration(f->xHome, f->yHome, 6, 1);
		}
		if (f->numWalkers > 6) {
			addRomanSoldierConcentration(f->xHome, f->yHome, 5, 1);
		}
		if (f->numWalkers > 9) {
			addRomanSoldierConcentration(f->xHome, f->yHome, 4, 1);
		}
		if (f->numWalkers > 12) {
			addRomanSoldierConcentration(f->xHome, f->yHome, 3, 1);
		}
		if (f->numWalkers > 15) {
			addRomanSoldierConcentration(f->xHome, f->yHome, 2, 1);
		}
	}
}

static void setNativeTargetBuilding(int formationId)
{
	int minBuildingId = 0;
	int minDistance = 10000;
	for (int i = 0; i < MAX_BUILDINGS; i++) {
		if (Data_Buildings[i].inUse != 1) {
			continue;
		}
		switch (Data_Buildings[i].type) {
			case Building_MissionPost:
			case Building_NativeHut:
			case Building_NativeCrops:
			case Building_NativeMeeting:
			case Building_Warehouse:
			case Building_FortGround__:
				break;
			default:
				int distance = Calc_distanceMaximum(
					Data_CityInfo.nativeMainMeetingCenterX,
					Data_CityInfo.nativeMainMeetingCenterY,
					Data_Buildings[i].x, Data_Buildings[i].y);
				if (distance < minDistance) {
					minBuildingId = i;
					minDistance = distance;
				}
		}
	}
	if (minBuildingId > 0) {
		Data_Formations[formationId].destinationX = Data_Buildings[minBuildingId].x;
		Data_Formations[formationId].destinationY = Data_Buildings[minBuildingId].y;
		Data_Formations[formationId].destinationBuildingId = minBuildingId;
	}
}
static void marsKillEnemies()
{
	if (Data_CityInfo.godBlessingMarsEnemiesToKill <= 0) {
		return;
	}
	int toKill = Data_CityInfo.godBlessingMarsEnemiesToKill;
	int gridOffset = 0;
	for (int i = 1; i < MAX_WALKERS && toKill > 0; i++) {
		struct Data_Walker *w = &Data_Walkers[i];
		if (w->state != WalkerState_Alive) {
			continue;
		}
		if (WalkerIsEnemy(w->type) && w->type != Walker_Enemy54_Gladiator) {
			w->actionState = WalkerActionState_149_Corpse;
			toKill--;
			if (!gridOffset) {
				gridOffset = w->gridOffset;
			}
		}
	}
	Data_CityInfo.godBlessingMarsEnemiesToKill = 0;
	PlayerMessage_post(1, 105, 0, gridOffset);
}

static void setFormationWalkersToActionState151(int formationId)
{
	struct Data_Formation *f = &Data_Formations[formationId];
	for (int i = 0; i < 16; i++) {
		if (f->walkerIds[i] > 0) {
			struct Data_Walker *w = &Data_Walkers[f->walkerIds[i]];
			if (w->actionState != WalkerActionState_149_Corpse &&
				w->actionState != WalkerActionState_150_Attack) {
				w->actionState = WalkerActionState_151_EnemyInitial;
				w->waitTicks = 0;
			}
		}
	}
}

static int moveHerdEnemyFormationTo(int formationId, int *x, int *y)
{
	// TODO
	return 0;
}

static void tickUpdateEnemies()
{
	if (Data_Formation_Extra.numEnemyFormations <= 0) {
		// TODO
		for (int i = 0; i < 25; i++) {
			// somevar = 0;
		}
		setNativeTargetBuilding(0);
		return;
	}
	Data_Formation_Extra.daysSinceRomanSoldierConcentration++;
	if (Data_Formation_Extra.daysSinceRomanSoldierConcentration > 4) {
		Data_Formation_Extra.daysSinceRomanSoldierConcentration = 0;
		calculateRomanSoldierConcentration();
	}
	// TODO
	setNativeTargetBuilding(0);
}

static int getHerdRoamingDestination(int formationId, int allowNegativeDesirability, int x, int y, int distance, int direction, int *xTile, int *yTile)
{
	int targetDirection = (formationId + Data_Random.random1_15bit) & 6;
	if (direction) {
		targetDirection = direction;
		allowNegativeDesirability = 1;
	}
	for (int i = 0; i < 4; i++) {
		int xTarget, yTarget;
		switch (targetDirection) {
			case 0: xTarget = x; yTarget = y - distance; break;
			case 1: xTarget = x + distance; yTarget = y - distance; break;
			case 2: xTarget = x + distance; yTarget = y; break;
			case 3: xTarget = x + distance; yTarget = y + distance; break;
			case 4: xTarget = x; yTarget = y + distance; break;
			case 5: xTarget = x - distance; yTarget = y + distance; break;
			case 6: xTarget = x - distance; yTarget = y; break;
			case 7: xTarget = x - distance; yTarget = y - distance; break;
		}
		if (xTarget <= 0) {
			xTarget = 1;
		} else if (yTarget <= 0) {
			yTarget = 1;
		} else if (xTarget >= Data_Settings_Map.width - 1) {
			xTarget = Data_Settings_Map.width - 2;
		} else if (yTarget >= Data_Settings_Map.height - 1) {
			yTarget = Data_Settings_Map.height - 2;
		}
		if (TerrainGraphics_getFreeTileForHerd(xTarget, yTarget, allowNegativeDesirability, xTile, yTile)) {
			return 1;
		}
		targetDirection += 2;
		if (targetDirection > 6) {
			targetDirection = 0;
		}
	}
	return 0;
}

static void moveAnimals(struct Data_Formation *f, int attackingAnimals)
{
	for (int i = 0; i < 16; i++) {
		if (!f->walkerIds[i]) continue;
		int walkerId = f->walkerIds[i];
		struct Data_Walker *w = &Data_Walkers[walkerId];
		if (w->actionState == WalkerActionState_149_Corpse ||
			w->actionState == WalkerActionState_150_Attack) {
			continue;
		}
		w->waitTicks = 401;
		if (attackingAnimals) {
			int targetId = WalkerAction_CombatWolf_getTarget(w->x, w->y, 6);
			if (targetId) {
				w->actionState = WalkerActionState_199_WolfAttacking;
				w->destinationX = Data_Walkers[targetId].x;
				w->destinationY = Data_Walkers[targetId].y;
				w->targetWalkerId = targetId;
				Data_Walkers[targetId].targetedByWalkerId = walkerId;
				w->targetWalkerCreatedSequence = Data_Walkers[targetId].createdSequence;
				WalkerRoute_remove(walkerId);
			} else {
				w->actionState = WalkerActionState_196_HerdAnimalAtRest;
			}
		} else {
			w->actionState = WalkerActionState_196_HerdAnimalAtRest;
		}
	}
}

static void tickUpdateHerds()
{
	if (Data_CityInfo.numAnimalsInCity <= 0) {
		return;
	}
	for (int i = 1; i < 50; i++) {
		struct Data_Formation *f = &Data_Formations[i];
		if (f->inUse != 1 || f->isLegion || !f->isHerd || f->numWalkers <= 0) {
			continue;
		}
		if (f->numWalkers < f->maxWalkers && f->walkerType == Walker_Wolf) {
			// spawn new wolf
			f->herdWolfSpawnDelay++;
			if (f->herdWolfSpawnDelay > 32) {
				f->herdWolfSpawnDelay = 0;
				if (!(Data_Grid_terrain[GridOffset(f->x, f->y)] & Terrain_d73f)) {
					int wolfId = Walker_create(f->walkerType, f->x, f->y, 0);
					Data_Walkers[wolfId].actionState = WalkerActionState_196_HerdAnimalAtRest;
					Data_Walkers[wolfId].formationId = i;
					Data_Walkers[wolfId].waitTicks = wolfId & 0x1f;
				}
			}
		}
		int attackingAnimals = 0;
		for (int w = 0; w < 16; w++) {
			int walkerId = f->walkerIds[i];
			if (walkerId > 0 && Data_Walkers[walkerId].actionState == WalkerActionState_150_Attack) {
				attackingAnimals++;
			}
		}
		if (f->missileAttackTimeout) {
			attackingAnimals = 1;
		}
		if (f->walkerIds[0] && Data_Walkers[f->walkerIds[0]].state == WalkerState_Alive) {
			f->xHome = Data_Walkers[f->walkerIds[0]].x;
			f->yHome = Data_Walkers[f->walkerIds[0]].y;
		}
		int roamDistance;
		int roamDelay;
		int allowNegativeDesirability;
		switch (f->walkerType) {
			case Walker_Sheep:
				roamDistance = 8;
				roamDelay = 20;
				allowNegativeDesirability = 0;
				attackingAnimals = 0;
				break;
			case Walker_Zebra:
				roamDistance = 20;
				roamDelay = 4;
				allowNegativeDesirability = 0;
				attackingAnimals = 0;
				break;
			case Walker_Wolf:
				roamDistance = 16;
				roamDelay = 6;
				allowNegativeDesirability = 1;
				break;
		}
		if (attackingAnimals) {
			f->waitTicks = roamDelay + 1;
		}
		f->waitTicks++;
		if (f->waitTicks > roamDelay) {
			f->waitTicks = 0;
			if (attackingAnimals) {
				f->destinationX = f->xHome;
				f->destinationY = f->yHome;
				moveAnimals(f, attackingAnimals);
			} else {
				int xTile, yTile;
				if (getHerdRoamingDestination(i, allowNegativeDesirability, f->xHome, f->yHome, roamDistance, f->herdDirection, &xTile, &yTile)) {
					f->herdDirection = 0;
					if (moveHerdEnemyFormationTo(i, &xTile, &yTile)) {
						f->destinationX = xTile;
						f->destinationY = yTile;
						if (f->walkerType == Walker_Wolf) {
							Data_CityInfo.soundMarchWolf--;
							if (Data_CityInfo.soundMarchWolf <= 0) {
								Data_CityInfo.soundMarchWolf = 12;
								Sound_Effects_playChannel(SoundChannel_WolfHowl);
							}
						}
						moveAnimals(f, attackingAnimals);
					}
				}
			}
		}
	}
}

void Formation_Tick_updateAll(int secondTime)
{
	Formation_calculateLegionTotals();
	Formation_calculateWalkers();
	tickUpdateDirection();
	tickDecreaseLegionDamage();
	if (!secondTime) {
		tickUpdateMorale();
	}
	Formation_setMaxSoldierPerLegion();
	tickUpdateLegions();
	tickUpdateEnemies();
	tickUpdateHerds();
}

#include "WalkerAction_private.h"

#include "Formation.h"
#include "Routing.h"
#include "Sound.h"
#include "Walker.h"

#include "Data/CityInfo.h"
#include "Data/Event.h"
#include "Data/Formation.h"
#include "Data/Grid.h"

static void enemyInitial(int walkerId, struct Data_Walker *w, struct Data_Formation *f)
{
	Walker_updatePositionInTileList(walkerId);
	w->graphicOffset = 0;
	WalkerRoute_remove(walkerId);
	w->waitTicks--;
	if (w->waitTicks <= 0) {
		if (w->isGhost && w->indexInFormation == 0) {
			if (f->layout == FormationLayout_Enemy8) {
				Sound_Speech_playFile("wavs/drums.wav");
			} else if (f->layout == FormationLayout_Enemy12) {
				Sound_Speech_playFile("wavs/horn2.wav");
			} else {
				Sound_Speech_playFile("wavs/horn2.wav");
			}
		}
		w->isGhost = 0;
		if (f->recentFight) {
			w->actionState = WalkerActionState_154_EnemyFighting;
		} else {
			w->destinationX = f->destinationX + w->formationPositionX;
			w->destinationY = f->destinationY + w->formationPositionY;
			if (Routing_getGeneralDirection(w->x, w->y, w->destinationX, w->destinationY)) {
				w->actionState = WalkerActionState_153_EnemyMarching;
			}
		}
	}
	if (w->type == Walker_Enemy43_Spear || w->type == Walker_Enemy46_Camel ||
		w->type == Walker_Enemy51_Spear || w->type == Walker_Enemy52_MountedArcher) {
		// missile throwers
		w->waitTicksMissile++;
		int xTile, yTile;
		if (w->waitTicksMissile > Constant_WalkerProperties[w->type].missileFrequency) {
			w->waitTicksMissile = 0;
			if (WalkerAction_CombatEnemy_getMissileTarget(walkerId, 10, Data_CityInfo.numSoldiersInCity < 4, &xTile, &yTile)) {
				w->attackGraphicOffset = 1;
				w->direction = Routing_getDirectionForMissileShooter(w->x, w->y, xTile, yTile);
			} else {
				w->attackGraphicOffset = 0;
			}
		}
		if (w->attackGraphicOffset) {
			int missileType;
			switch (f->enemyType) {
				case EnemyType_4_Goth:
				case EnemyType_5_Pergamum:
				case EnemyType_9_Egyptian:
				case EnemyType_10_Carthaginian:
					missileType = Walker_Arrow;
					break;
				default:
					missileType = Walker_Spear;
					break;
			}
			if (w->attackGraphicOffset == 1) {
				Walker_createMissile(walkerId, w->x, w->y, xTile, yTile, missileType);
				f->missileFired = 6;
			}
			if (missileType == Walker_Arrow) {
				Data_CityInfo.soundShootArrow--;
				if (Data_CityInfo.soundShootArrow <= 0) {
					Data_CityInfo.soundShootArrow = 10;
					Sound_Effects_playChannel(SoundChannel_Arrow);
				}
			}
			w->attackGraphicOffset++;
			if (w->attackGraphicOffset > 100) {
				w->attackGraphicOffset = 0;
			}
		}
	}
}

static void enemyMarching(int walkerId, struct Data_Walker *w, struct Data_Formation *f)
{
	w->waitTicks--;
	if (w->waitTicks <= 0) {
		w->waitTicks = 50;
		w->destinationX = f->destinationX + w->formationPositionX;
		w->destinationY = f->destinationY + w->formationPositionY;
		if (Routing_getGeneralDirection(w->x, w->y, w->destinationX, w->destinationY) == 8) {
			w->actionState = WalkerActionState_151_EnemyInitial;
			return;
		}
		w->destinationBuildingId = f->destinationBuildingId;
		WalkerRoute_remove(walkerId);
	}
	WalkerMovement_walkTicks(walkerId, w->speedMultiplier);
	if (w->direction == 8 || w->direction == 9 || w->direction == 10) {
		w->actionState = WalkerActionState_151_EnemyInitial;
	}
}

static void enemyFighting(int walkerId, struct Data_Walker *w, struct Data_Formation *f)
{
	if (!f->recentFight) {
		w->actionState = WalkerActionState_151_EnemyInitial;
	}
	if (w->type != Walker_Enemy46_Camel && w->type != Walker_Enemy47_Elephant) {
		if (w->type == Walker_Enemy48_Chariot || w->type == Walker_Enemy52_MountedArcher) {
			Data_CityInfo.soundMarchHorse--;
			if (Data_CityInfo.soundMarchHorse <= 0) {
				Data_CityInfo.soundMarchHorse = 200;
				Sound_Effects_playChannel(SoundChannel_HorseMoving);
			}
		} else {
			Data_CityInfo.soundMarchEnemy--;
			if (Data_CityInfo.soundMarchEnemy <= 0) {
				Data_CityInfo.soundMarchEnemy = 200;
				Sound_Effects_playChannel(SoundChannel_Marching);
			}
		}
	}
	int targetId = w->targetWalkerId;
	if (Data_Walkers[targetId].state != WalkerState_Alive) {
		w->targetWalkerId = 0;
		targetId = 0;
	}
	if (Data_Walkers[targetId].actionState == WalkerActionState_149_Corpse) {
		w->targetWalkerId = 0;
		targetId = 0;
	}
	if (targetId <= 0) {
		targetId = WalkerAction_CombatEnemy_getTarget(w->x, w->y);
		if (targetId) {
			w->destinationX = Data_Walkers[targetId].x;
			w->destinationY = Data_Walkers[targetId].y;
			w->targetWalkerId = targetId;
			w->targetWalkerCreatedSequence = Data_Walkers[targetId].createdSequence;
			Data_Walkers[targetId].targetedByWalkerId = walkerId;
			WalkerRoute_remove(walkerId);
		}
	}
	if (targetId > 0) {
		WalkerMovement_walkTicks(walkerId, w->speedMultiplier);
		if (w->direction == 8) {
			w->destinationX = Data_Walkers[w->targetWalkerId].x;
			w->destinationY = Data_Walkers[w->targetWalkerId].y;
			WalkerRoute_remove(walkerId);
		} else if (w->direction == 9 || w->direction == 10) {
			w->actionState = WalkerActionState_151_EnemyInitial;
			w->targetWalkerId = 0;
		}
	} else {
		w->actionState = WalkerActionState_151_EnemyInitial;
		w->waitTicks = 50;
	}
}

static void WalkerAction_enemyCommon(int walkerId, struct Data_Walker *w)
{
	struct Data_Formation *f = &Data_Formations[w->formationId];
	Data_CityInfo.numEnemiesInCity++;
	w->terrainUsage = WalkerTerrainUsage_Enemy;
	w->formationPositionX = f->x + WalkerActionFormationLayoutPositionX(f->layout, w->indexInFormation);
	w->formationPositionY = f->y + WalkerActionFormationLayoutPositionY(f->layout, w->indexInFormation);

	switch (w->actionState) {
		case WalkerActionState_150_Attack:
			WalkerAction_Common_handleAttack(walkerId);
			break;
		case WalkerActionState_149_Corpse:
			WalkerAction_Common_handleCorpse(walkerId);
			break;
		case WalkerActionState_148_Fleeing:
			w->destinationX = w->sourceX;
			w->destinationY = w->sourceY;
			WalkerMovement_walkTicks(walkerId, w->speedMultiplier);
			if (w->direction == 8 || w->direction == 9 || w->direction == 10) {
				w->state = WalkerState_Dead;
			}
			break;
		case WalkerActionState_151_EnemyInitial:
			enemyInitial(walkerId, w, f);
			break;
		case WalkerActionState_152_EnemyWaiting:
			Walker_updatePositionInTileList(walkerId);
			break;
		case WalkerActionState_153_EnemyMarching:
			enemyMarching(walkerId, w, f);
			break;
		case WalkerActionState_154_EnemyFighting:
			enemyFighting(walkerId, w, f);
			break;
	}
}

void WalkerAction_enemy43_Spear(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Formation *f = &Data_Formations[w->formationId];
	WalkerActionIncreaseGraphicOffset(w, 12);
	w->cartGraphicId = 0;
	w->speedMultiplier = 1;
	WalkerAction_enemyCommon(walkerId, w);
	
	int dir;
	if (w->actionState == WalkerActionState_150_Attack) {
		dir = w->attackDirection;
	} else if (f->missileFired || w->direction < 8) {
		dir = w->direction;
	} else {
		dir = w->previousTileDirection;
	}
	WalkerActionNormalizeDirection(dir);
	
	w->isEnemyGraphic = 1;
	
	switch (f->enemyType) {
		case EnemyType_5_Pergamum:
		case EnemyType_6_Seleucid:
		case EnemyType_7_Etruscan:
		case EnemyType_8_Greek:
			break;
		default:
			return;
	}
	if (w->actionState == WalkerActionState_150_Attack) {
		if (w->attackGraphicOffset >= 12) {
			w->graphicId = 745 + dir + 8 * ((w->attackGraphicOffset - 12) / 2);
		} else {
			w->graphicId = 745 + dir;
		}
	} else if (w->actionState == WalkerActionState_151_EnemyInitial) {
		w->graphicId = 697 + dir + 8 * WalkerActionMissileLauncherGraphicOffset(w);
	} else if (w->actionState == WalkerActionState_149_Corpse) {
		w->graphicId = 793 + WalkerActionCorpseGraphicOffset(w);
	} else if (w->direction == 11) {
		w->graphicId = 745 + dir + 8 * (w->graphicOffset / 2);
	} else {
		w->graphicId = 601 + dir + 8 * w->graphicOffset;
	}
}

void WalkerAction_enemy44_Sword(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Formation *f = &Data_Formations[w->formationId];
	WalkerActionIncreaseGraphicOffset(w, 12);
	w->cartGraphicId = 0;
	w->speedMultiplier = 1;
	WalkerAction_enemyCommon(walkerId, w);
	
	int dir;
	if (w->actionState == WalkerActionState_150_Attack) {
		dir = w->attackDirection;
	} else if (w->direction < 8) {
		dir = w->direction;
	} else {
		dir = w->previousTileDirection;
	}
	WalkerActionNormalizeDirection(dir);
	
	w->isEnemyGraphic = 1;
	
	switch (f->enemyType) {
		case EnemyType_5_Pergamum:
		case EnemyType_6_Seleucid:
		case EnemyType_9_Egyptian:
			break;
		default:
			return;
	}
	if (w->actionState == WalkerActionState_150_Attack) {
		if (w->attackGraphicOffset >= 12) {
			w->graphicId = 545 + dir + 8 * ((w->attackGraphicOffset - 12) / 2);
		} else {
			w->graphicId = 545 + dir;
		}
	} else if (w->actionState == WalkerActionState_149_Corpse) {
		w->graphicId = 593 + WalkerActionCorpseGraphicOffset(w);
	} else if (w->direction == 11) {
		w->graphicId = 545 + dir + 8 * (w->graphicOffset / 2);
	} else {
		w->graphicId = 449 + dir + 8 * w->graphicOffset;
	}
}

void WalkerAction_enemy45_Sword(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Formation *f = &Data_Formations[w->formationId];
	WalkerActionIncreaseGraphicOffset(w, 12);
	w->cartGraphicId = 0;
	w->speedMultiplier = 1;
	WalkerAction_enemyCommon(walkerId, w);
	
	int dir;
	if (w->actionState == WalkerActionState_150_Attack) {
		dir = w->attackDirection;
	} else if (w->direction < 8) {
		dir = w->direction;
	} else {
		dir = w->previousTileDirection;
	}
	WalkerActionNormalizeDirection(dir);
	
	w->isEnemyGraphic = 1;
	
	switch (f->enemyType) {
		case EnemyType_7_Etruscan:
		case EnemyType_8_Greek:
		case EnemyType_10_Carthaginian:
			break;
		default:
			return;
	}
	if (w->actionState == WalkerActionState_150_Attack) {
		if (w->attackGraphicOffset >= 12) {
			w->graphicId = 545 + dir + 8 * ((w->attackGraphicOffset - 12) / 2);
		} else {
			w->graphicId = 545 + dir;
		}
	} else if (w->actionState == WalkerActionState_149_Corpse) {
		w->graphicId = 593 + WalkerActionCorpseGraphicOffset(w);
	} else if (w->direction == 11) {
		w->graphicId = 545 + dir + 8 * (w->graphicOffset / 2);
	} else {
		w->graphicId = 449 + dir + 8 * w->graphicOffset;
	}
}

void WalkerAction_enemy46_Camel(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Formation *f = &Data_Formations[w->formationId];
	WalkerActionIncreaseGraphicOffset(w, 12);
	w->cartGraphicId = 0;
	w->speedMultiplier = 1;
	WalkerAction_enemyCommon(walkerId, w);
	
	int dir;
	if (w->actionState == WalkerActionState_150_Attack) {
		dir = w->attackDirection;
	} else if (f->missileFired || w->direction < 8) {
		dir = w->direction;
	} else {
		dir = w->previousTileDirection;
	}
	WalkerActionNormalizeDirection(dir);
	
	w->isEnemyGraphic = 1;
	
	if (w->direction == 11) {
		w->graphicId = 601 + dir + 8 * w->graphicOffset;
	} else if (w->actionState == WalkerActionState_150_Attack) {
		w->graphicId = 601 + dir;
	} else if (w->actionState == WalkerActionState_151_EnemyInitial) {
		w->graphicId = 697 + dir + 8 * WalkerActionMissileLauncherGraphicOffset(w);
	} else if (w->actionState == WalkerActionState_149_Corpse) {
		w->graphicId = 745 + WalkerActionCorpseGraphicOffset(w);
	} else {
		w->graphicId = 601 + dir + 8 * w->graphicOffset;
	}
}

void WalkerAction_enemy47_Elephant(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	WalkerActionIncreaseGraphicOffset(w, 12);
	w->cartGraphicId = 0;
	w->speedMultiplier = 1;
	WalkerAction_enemyCommon(walkerId, w);
	
	int dir;
	if (w->actionState == WalkerActionState_150_Attack) {
		dir = w->attackDirection;
	} else if (w->direction < 8) {
		dir = w->direction;
	} else {
		dir = w->previousTileDirection;
	}
	WalkerActionNormalizeDirection(dir);
	
	w->isEnemyGraphic = 1;
	
	if (w->direction == 11) {
		w->graphicId = 601 + dir + 8 * w->graphicOffset;
	} else if (w->actionState == WalkerActionState_150_Attack) {
		w->graphicId = 601 + dir + 8 * w->graphicOffset;
	} else if (w->actionState == WalkerActionState_149_Corpse) {
		w->graphicId = 705 + WalkerActionCorpseGraphicOffset(w);
	} else {
		w->graphicId = 601 + dir + 8 * w->graphicOffset;
	}
}

void WalkerAction_enemy48_Chariot(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	WalkerActionIncreaseGraphicOffset(w, 12);
	w->cartGraphicId = 0;
	w->speedMultiplier = 3;
	WalkerAction_enemyCommon(walkerId, w);
	
	int dir;
	if (w->actionState == WalkerActionState_150_Attack) {
		dir = w->attackDirection;
	} else if (w->direction < 8) {
		dir = w->direction;
	} else {
		dir = w->previousTileDirection;
	}
	WalkerActionNormalizeDirection(dir);
	
	w->isEnemyGraphic = 1;
	
	if (w->direction == 11) {
		w->graphicId = 697 + dir + 8 * (w->graphicOffset / 2);
	} else if (w->actionState == WalkerActionState_150_Attack) {
		w->graphicId = 697 + dir + 8 * (w->graphicOffset / 2);
	} else if (w->actionState == WalkerActionState_149_Corpse) {
		w->graphicId = 745 + WalkerActionCorpseGraphicOffset(w);
	} else {
		w->graphicId = 601 + dir + 8 * w->graphicOffset;
	}
}

void WalkerAction_enemy49_FastSword(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Formation *f = &Data_Formations[w->formationId];
	WalkerActionIncreaseGraphicOffset(w, 12);
	w->cartGraphicId = 0;
	w->speedMultiplier = 2;
	WalkerAction_enemyCommon(walkerId, w);
	
	int dir;
	if (w->actionState == WalkerActionState_150_Attack) {
		dir = w->attackDirection;
	} else if (w->direction < 8) {
		dir = w->direction;
	} else {
		dir = w->previousTileDirection;
	}
	WalkerActionNormalizeDirection(dir);
	
	w->isEnemyGraphic = 1;
	
	int attackId, corpseId, normalId;
	if (f->enemyType == EnemyType_0_Barbarian) {
		attackId = 393;
		corpseId = 441;
		normalId = 297;
	} else if (f->enemyType == EnemyType_1_Numidian) {
		attackId = 593;
		corpseId = 641;
		normalId = 449;
	} else if (f->enemyType == EnemyType_4_Goth) {
		attackId = 545;
		corpseId = 593;
		normalId = 449;
	} else {
		return;
	}
	if (w->actionState == WalkerActionState_150_Attack) {
		if (w->attackGraphicOffset >= 12) {
			w->graphicId = attackId + dir + 8 * ((w->attackGraphicOffset - 12) / 2);
		} else {
			w->graphicId = attackId + dir;
		}
	} else if (w->actionState == WalkerActionState_149_Corpse) {
		w->graphicId = corpseId + WalkerActionCorpseGraphicOffset(w);
	} else if (w->direction == 11) {
		w->graphicId = attackId + dir + 8 * (w->graphicOffset / 2);
	} else {
		w->graphicId = normalId + dir + 8 * w->graphicOffset;
	}
}

void WalkerAction_enemy50_Sword(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Formation *f = &Data_Formations[w->formationId];
	WalkerActionIncreaseGraphicOffset(w, 12);
	w->cartGraphicId = 0;
	w->speedMultiplier = 1;
	WalkerAction_enemyCommon(walkerId, w);
	
	int dir;
	if (w->actionState == WalkerActionState_150_Attack) {
		dir = w->attackDirection;
	} else if (w->direction < 8) {
		dir = w->direction;
	} else {
		dir = w->previousTileDirection;
	}
	WalkerActionNormalizeDirection(dir);
	
	w->isEnemyGraphic = 1;
	
	if (f->enemyType != EnemyType_2_Gaul && f->enemyType != EnemyType_3_Celt) {
		return;
	}
	if (w->actionState == WalkerActionState_150_Attack) {
		if (w->attackGraphicOffset >= 12) {
			w->graphicId = 545 + dir + 8 * ((w->attackGraphicOffset - 12) / 2);
		} else {
			w->graphicId = 545 + dir;
		}
	} else if (w->actionState == WalkerActionState_149_Corpse) {
		w->graphicId = 593 + WalkerActionCorpseGraphicOffset(w);
	} else if (w->direction == 11) {
		w->graphicId = 545 + dir + 8 * (w->graphicOffset / 2);
	} else {
		w->graphicId = 449 + dir + 8 * w->graphicOffset;
	}
}

void WalkerAction_enemy51_Spear(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Formation *f = &Data_Formations[w->formationId];
	WalkerActionIncreaseGraphicOffset(w, 12);
	w->cartGraphicId = 0;
	w->speedMultiplier = 2;
	WalkerAction_enemyCommon(walkerId, w);
	
	int dir;
	if (w->actionState == WalkerActionState_150_Attack) {
		dir = w->attackDirection;
	} else if (f->missileFired || w->direction < 8) {
		dir = w->direction;
	} else {
		dir = w->previousTileDirection;
	}
	WalkerActionNormalizeDirection(dir);
	
	w->isEnemyGraphic = 1;
	
	if (f->enemyType != EnemyType_1_Numidian) {
		return;
	}
	if (w->actionState == WalkerActionState_150_Attack) {
		if (w->attackGraphicOffset >= 12) {
			w->graphicId = 593 + dir + 8 * ((w->attackGraphicOffset - 12) / 2);
		} else {
			w->graphicId = 593 + dir;
		}
	} else if (w->actionState == WalkerActionState_151_EnemyInitial) {
		w->graphicId = 545 + dir + 8 * WalkerActionMissileLauncherGraphicOffset(w);
	} else if (w->actionState == WalkerActionState_149_Corpse) {
		w->graphicId = 641 + WalkerActionCorpseGraphicOffset(w);
	} else if (w->direction == 11) {
		w->graphicId = 593 + dir + 8 * (w->graphicOffset / 2);
	} else {
		w->graphicId = 449 + dir + 8 * w->graphicOffset;
	}
}

void WalkerAction_enemy52_MountedArcher(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Formation *f = &Data_Formations[w->formationId];
	WalkerActionIncreaseGraphicOffset(w, 12);
	w->cartGraphicId = 0;
	w->speedMultiplier = 3;
	WalkerAction_enemyCommon(walkerId, w);
	
	int dir;
	if (w->actionState == WalkerActionState_150_Attack) {
		dir = w->attackDirection;
	} else if (f->missileFired || w->direction < 8) {
		dir = w->direction;
	} else {
		dir = w->previousTileDirection;
	}
	WalkerActionNormalizeDirection(dir);
	
	w->isEnemyGraphic = 1;
	
	if (w->direction == 11) {
		w->graphicId = 601 + dir + 8 * w->graphicOffset;
	} else if (w->actionState == WalkerActionState_150_Attack) {
		w->graphicId = 601 + dir;
	} else if (w->actionState == WalkerActionState_151_EnemyInitial) {
		w->graphicId = 697 + dir + 8 * WalkerActionMissileLauncherGraphicOffset(w);
	} else if (w->actionState == WalkerActionState_149_Corpse) {
		w->graphicId = 745 + WalkerActionCorpseGraphicOffset(w);
	} else {
		w->graphicId = 601 + dir + 8 * w->graphicOffset;
	}
}

void WalkerAction_enemy53_Axe(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Formation *f = &Data_Formations[w->formationId];
	WalkerActionIncreaseGraphicOffset(w, 12);
	w->cartGraphicId = 0;
	w->speedMultiplier = 1;
	WalkerAction_enemyCommon(walkerId, w);
	
	int dir;
	if (w->actionState == WalkerActionState_150_Attack) {
		dir = w->attackDirection;
	} else if (w->direction < 8) {
		dir = w->direction;
	} else {
		dir = w->previousTileDirection;
	}
	WalkerActionNormalizeDirection(dir);
	
	w->isEnemyGraphic = 1;
	
	if (f->enemyType != EnemyType_2_Gaul) {
		return;
	}
	if (w->actionState == WalkerActionState_150_Attack) {
		if (w->attackGraphicOffset >= 12) {
			w->graphicId = 697 + dir + 8 * ((w->attackGraphicOffset - 12) / 2);
		} else {
			w->graphicId = 697 + dir;
		}
	} else if (w->actionState == WalkerActionState_149_Corpse) {
		w->graphicId = 745 + WalkerActionCorpseGraphicOffset(w);
	} else if (w->direction == 11) {
		w->graphicId = 697 + dir + 8 * (w->graphicOffset / 2);
	} else {
		w->graphicId = 601 + dir + 8 * w->graphicOffset;
	}
}

void WalkerAction_enemy54_Gladiator(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->terrainUsage = 0;
	w->useCrossCountry = 0;
	WalkerActionIncreaseGraphicOffset(w, 12);
	if (Data_Event.gladiatorRevolt.state == 2) {
		// end of gladiator revolt: kill gladiators
		if (w->actionState != WalkerActionState_149_Corpse) {
			w->actionState = WalkerActionState_149_Corpse;
			w->waitTicks = 0;
			w->direction = 0;
		}
	}
	switch (w->actionState) {
		case WalkerActionState_150_Attack:
			WalkerAction_Common_handleAttack(walkerId);
			WalkerActionIncreaseGraphicOffset(w, 16);
			break;
		case WalkerActionState_149_Corpse:
			WalkerAction_Common_handleCorpse(walkerId);
			break;
		case WalkerActionState_158_NativeCreated:
			w->graphicOffset = 0;
			w->waitTicks++;
			if (w->waitTicks > 10 + (walkerId & 3)) {
				w->waitTicks = 0;
				w->actionState = WalkerActionState_159_NativeAttacking;
				int xTile, yTile;
				int buildingId = Formation_Rioter_getTargetBuilding(&xTile, &yTile);
				if (buildingId) {
					w->destinationX = xTile;
					w->destinationY = yTile;
					w->destinationBuildingId = buildingId;
					WalkerRoute_remove(walkerId);
				} else {
					w->state = WalkerState_Dead;
				}
			}
			break;
		case WalkerActionState_159_NativeAttacking:
			Data_CityInfo.numAttackingNativesInCity = 10;
			w->terrainUsage = 2;
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == 8 || w->direction == 9 || w->direction == 10) {
				w->actionState = WalkerActionState_158_NativeCreated;
			}
			break;
	}
	int dir;
	if (w->actionState == WalkerActionState_150_Attack || w->direction == 11) {
		dir = w->attackDirection;
	} else if (w->direction < 8) {
		dir = w->direction;
	} else {
		dir = w->previousTileDirection;
	}
	WalkerActionNormalizeDirection(dir);

	if (w->actionState == WalkerActionState_150_Attack || w->direction == 11) {
		w->graphicId = GraphicId(ID_Graphic_Walker_Gladiator) + dir + 104 + 8 * (w->graphicOffset / 2);
	} else if (w->actionState == WalkerActionState_149_Corpse) {
		w->graphicId = GraphicId(ID_Graphic_Walker_Gladiator) + 96 + WalkerActionCorpseGraphicOffset(w);
	} else {
		w->graphicId = GraphicId(ID_Graphic_Walker_Gladiator) + dir + 8 * w->graphicOffset;
	}
}

void WalkerAction_enemyCaesarLegionary(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Formation *f = &Data_Formations[w->formationId];
	Data_CityInfo.numImperialSoldiersInCity++;
	WalkerActionIncreaseGraphicOffset(w, 12);
	w->cartGraphicId = 0;
	w->speedMultiplier = 1;
	WalkerAction_enemyCommon(walkerId, w);
	
	int dir;
	if (w->actionState == WalkerActionState_150_Attack) {
		dir = w->attackDirection;
	} else if (w->direction < 8) {
		dir = w->direction;
	} else {
		dir = w->previousTileDirection;
	}
	WalkerActionNormalizeDirection(dir);
	
	if (w->direction == 11) {
		w->graphicId = GraphicId(ID_Graphic_Walker_CaesarLegionary) + dir + 8 * ((w->attackGraphicOffset - 12) / 2);
	}
	switch (w->actionState) {
		case WalkerActionState_150_Attack:
			if (w->attackGraphicOffset >= 12) {
				w->graphicId = GraphicId(ID_Graphic_Walker_CaesarLegionary) + dir + 8 * ((w->attackGraphicOffset - 12) / 2);
			} else {
				w->graphicId = GraphicId(ID_Graphic_Walker_CaesarLegionary) + dir;
			}
			break;
		case WalkerActionState_149_Corpse:
			w->graphicId = GraphicId(ID_Graphic_Walker_CaesarLegionary) + WalkerActionCorpseGraphicOffset(w) + 152;
			break;
		case WalkerActionState_84_SoldierAtStandard:
			if (f->isHalted && f->layout == FormationLayout_Tortoise && f->missileAttackTimeout) {
				w->graphicId = GraphicId(ID_Graphic_Walker_FortLegionary) + dir + 144;
			} else {
				w->graphicId = GraphicId(ID_Graphic_Walker_FortLegionary) + dir;
			}
			break;
		default:
			w->graphicId = GraphicId(ID_Graphic_Walker_CaesarLegionary) + 48 + dir + 8 * w->graphicOffset;
			break;
	}
}

int WalkerAction_HerdEnemy_moveFormationTo(int formationId, int x, int y, int *xTile, int *yTile)
{
	struct Data_Formation *f = &Data_Formations[formationId];
	int baseOffset = GridOffset(
		WalkerActionFormationLayoutPositionX(f->layout, 0),
		WalkerActionFormationLayoutPositionY(f->layout, 0));
	int walkerOffsets[50];
	walkerOffsets[0] = 0;
	for (int i = 1; i < f->numWalkers; i++) {
		walkerOffsets[i] = GridOffset(
			WalkerActionFormationLayoutPositionX(f->layout, 0),
			WalkerActionFormationLayoutPositionY(f->layout, 0)) - baseOffset;
	}
	Routing_canTravelOverLandNonCitizen(x, y, -1, -1, 0, 600);
	for (int r = 0; r <= 10; r++) {
		int xMin = x - r;
		int yMin = y - r;
		int xMax = x + r;
		int yMax = y + r;
		Bound2ToMap(xMin, yMin, xMax, yMax);
		for (int yy = yMin; yy <= yMax; yy++) {
			for (int xx = xMin; xx <= xMax; xx++) {
				int canMove = 1;
				for (int w = 0; w < f->numWalkers; w++) {
					int gridOffset = GridOffset(x, y) + walkerOffsets[w];
					if (Data_Grid_terrain[gridOffset] & Terrain_1237) {
						canMove = 0;
						break;
					}
					if (Data_Grid_routingDistance[gridOffset] <= 0) {
						canMove = 0;
						break;
					}
					if (Data_Grid_walkerIds[gridOffset] &&
						Data_Walkers[Data_Grid_walkerIds[gridOffset]].formationId != formationId) {
						canMove = 0;
						break;
					}
				}
				if (canMove) {
					*xTile = xx;
					*yTile = yy;
					return 1;
				}
			}
		}
	}
	return 0;
}

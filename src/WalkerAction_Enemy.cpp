#include "WalkerAction_private.h"

#include "Walker.h"

#include "Data/CityInfo.h"
#include "Data/Event.h"
#include "Data/Formation.h"

static void WalkerAction_enemyCommon(int walkerId, struct Data_Walker *w)
{
	// TODO
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
				int buildingId = WalkerAction_Rioter_getTargetBuilding(&xTile, &yTile);
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

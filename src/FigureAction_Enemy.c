#include "FigureAction_private.h"

#include "Figure.h"
#include "Formation.h"

#include "Data/CityInfo.h"
#include "Data/Grid.h"

#include "core/calc.h"
#include "figure/formation.h"
#include "figure/properties.h"
#include "figure/route.h"
#include "map/figure.h"
#include "map/grid.h"
#include "map/routing.h"
#include "scenario/gladiator_revolt.h"
#include "sound/effect.h"
#include "sound/speech.h"

static void enemyInitial(figure *f, const formation *m)
{
	map_figure_update(f);
	f->graphicOffset = 0;
	figure_route_remove(f);
	f->waitTicks--;
	if (f->waitTicks <= 0) {
		if (f->isGhost && f->indexInFormation == 0) {
			if (m->layout == FORMATION_ENEMY8) {
				sound_speech_play_file("wavs/drums.wav");
			} else if (m->layout == FORMATION_ENEMY12) {
				sound_speech_play_file("wavs/horn2.wav");
			} else {
				sound_speech_play_file("wavs/horn1.wav");
			}
		}
		f->isGhost = 0;
		if (m->recent_fight) {
			f->actionState = FigureActionState_154_EnemyFighting;
		} else {
			f->destinationX = m->destination_x + f->formationPositionX;
			f->destinationY = m->destination_y + f->formationPositionY;
			if (calc_general_direction(f->x, f->y, f->destinationX, f->destinationY) < 8) {
				f->actionState = FigureActionState_153_EnemyMarching;
			}
		}
	}
	if (f->type == FIGURE_ENEMY43_SPEAR || f->type == FIGURE_ENEMY46_CAMEL ||
		f->type == FIGURE_ENEMY51_SPEAR || f->type == FIGURE_ENEMY52_MOUNTED_ARCHER) {
		// missile throwers
		f->waitTicksMissile++;
		int xTile, yTile;
		if (f->waitTicksMissile > figure_properties_for_type(f->type)->missile_delay) {
			f->waitTicksMissile = 0;
			if (FigureAction_CombatEnemy_getMissileTarget(f, 10, Data_CityInfo.numSoldiersInCity < 4, &xTile, &yTile)) {
				f->attackGraphicOffset = 1;
				f->direction = calc_missile_shooter_direction(f->x, f->y, xTile, yTile);
			} else {
				f->attackGraphicOffset = 0;
			}
		}
		if (f->attackGraphicOffset) {
			int missileType;
			switch (m->enemy_type) {
				case ENEMY_4_GOTH:
				case ENEMY_5_PERGAMUM:
				case ENEMY_9_EGYPTIAN:
				case ENEMY_10_CARTHAGINIAN:
					missileType = FIGURE_ARROW;
					break;
				default:
					missileType = FIGURE_SPEAR;
					break;
			}
			if (f->attackGraphicOffset == 1) {
				Figure_createMissile(f->id, f->x, f->y, xTile, yTile, missileType);
				formation_record_missile_fired(m->id);
			}
			if (missileType == FIGURE_ARROW) {
				Data_CityInfo.soundShootArrow--;
				if (Data_CityInfo.soundShootArrow <= 0) {
					Data_CityInfo.soundShootArrow = 10;
					sound_effect_play(SOUND_EFFECT_ARROW);
				}
			}
			f->attackGraphicOffset++;
			if (f->attackGraphicOffset > 100) {
				f->attackGraphicOffset = 0;
			}
		}
	}
}

static void enemyMarching(figure *f, const formation *m)
{
	f->waitTicks--;
	if (f->waitTicks <= 0) {
		f->waitTicks = 50;
		f->destinationX = m->destination_x + f->formationPositionX;
		f->destinationY = m->destination_y + f->formationPositionY;
		if (calc_general_direction(f->x, f->y, f->destinationX, f->destinationY) == DIR_FIGURE_AT_DESTINATION) {
			f->actionState = FigureActionState_151_EnemyInitial;
			return;
		}
		f->destinationBuildingId = m->destination_building_id;
		figure_route_remove(f);
	}
	FigureMovement_walkTicks(f, f->speedMultiplier);
	if (f->direction == DIR_FIGURE_AT_DESTINATION ||
		f->direction == DIR_FIGURE_REROUTE ||
		f->direction == DIR_FIGURE_LOST) {
		f->actionState = FigureActionState_151_EnemyInitial;
	}
}

static void enemyFighting(figure *f, const formation *m)
{
	if (!m->recent_fight) {
		f->actionState = FigureActionState_151_EnemyInitial;
	}
	if (f->type != FIGURE_ENEMY46_CAMEL && f->type != FIGURE_ENEMY47_ELEPHANT) {
		if (f->type == FIGURE_ENEMY48_CHARIOT || f->type == FIGURE_ENEMY52_MOUNTED_ARCHER) {
			Data_CityInfo.soundMarchHorse--;
			if (Data_CityInfo.soundMarchHorse <= 0) {
				Data_CityInfo.soundMarchHorse = 200;
				sound_effect_play(SOUND_EFFECT_HORSE_MOVING);
			}
		} else {
			Data_CityInfo.soundMarchEnemy--;
			if (Data_CityInfo.soundMarchEnemy <= 0) {
				Data_CityInfo.soundMarchEnemy = 200;
				sound_effect_play(SOUND_EFFECT_MARCHING);
			}
		}
	}
	int targetId = f->targetFigureId;
	if (figure_is_dead(figure_get(targetId))) {
		f->targetFigureId = 0;
		targetId = 0;
	}
	if (targetId <= 0) {
		targetId = FigureAction_CombatEnemy_getTarget(f->x, f->y);
		if (targetId) {
            figure *target = figure_get(targetId);
			f->destinationX = target->x;
			f->destinationY = target->y;
			f->targetFigureId = targetId;
			f->targetFigureCreatedSequence = target->createdSequence;
			target->targetedByFigureId = f->id;
			figure_route_remove(f);
		}
	}
	if (targetId > 0) {
		FigureMovement_walkTicks(f, f->speedMultiplier);
		if (f->direction == DIR_FIGURE_AT_DESTINATION) {
            figure *target = figure_get(f->targetFigureId);
			f->destinationX = target->x;
			f->destinationY = target->y;
			figure_route_remove(f);
		} else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
			f->actionState = FigureActionState_151_EnemyInitial;
			f->targetFigureId = 0;
		}
	} else {
		f->actionState = FigureActionState_151_EnemyInitial;
		f->waitTicks = 50;
	}
}

static void FigureAction_enemyCommon(figure *f)
{
	const formation *m = formation_get(f->formationId);
	Data_CityInfo.numEnemiesInCity++;
	f->terrainUsage = FigureTerrainUsage_Enemy;
	f->formationPositionX = FigureActionFormationLayoutPositionX(m->layout, f->indexInFormation);
	f->formationPositionY = FigureActionFormationLayoutPositionY(m->layout, f->indexInFormation);

	switch (f->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(f);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(f);
			break;
		case FigureActionState_148_Fleeing:
			f->destinationX = f->sourceX;
			f->destinationY = f->sourceY;
			FigureMovement_walkTicks(f, f->speedMultiplier);
			if (f->direction == DIR_FIGURE_AT_DESTINATION ||
				f->direction == DIR_FIGURE_REROUTE ||
				f->direction == DIR_FIGURE_LOST) {
				f->state = FigureState_Dead;
			}
			break;
		case FigureActionState_151_EnemyInitial:
			enemyInitial(f, m);
			break;
		case FigureActionState_152_EnemyWaiting:
			map_figure_update(f);
			break;
		case FigureActionState_153_EnemyMarching:
			enemyMarching(f, m);
			break;
		case FigureActionState_154_EnemyFighting:
			enemyFighting(f, m);
			break;
	}
}

static int getDirection(figure *f)
{
	int dir;
	if (f->actionState == FigureActionState_150_Attack) {
		dir = f->attackDirection;
	} else if (f->direction < 8) {
		dir = f->direction;
	} else {
		dir = f->previousTileDirection;
	}
	FigureActionNormalizeDirection(dir);
	return dir;
}

static int getDirectionMissile(figure *f, const formation *m)
{
	int dir;
	if (f->actionState == FigureActionState_150_Attack) {
		dir = f->attackDirection;
	} else if (m->missile_fired || f->direction < 8) {
		dir = f->direction;
	} else {
		dir = f->previousTileDirection;
	}
	FigureActionNormalizeDirection(dir);
	return dir;
}

void FigureAction_enemy43_Spear(figure *f)
{
	const formation *m = formation_get(f->formationId);
	FigureActionIncreaseGraphicOffset(f, 12);
	f->cartGraphicId = 0;
	f->speedMultiplier = 1;
	FigureAction_enemyCommon(f);
	
	int dir = getDirectionMissile(f, m);
	
	f->isEnemyGraphic = 1;
	
	switch (m->enemy_type) {
		case ENEMY_5_PERGAMUM:
		case ENEMY_6_SELEUCID:
		case ENEMY_7_ETRUSCAN:
		case ENEMY_8_GREEK:
			break;
		default:
			return;
	}
	if (f->actionState == FigureActionState_150_Attack) {
		if (f->attackGraphicOffset >= 12) {
			f->graphicId = 745 + dir + 8 * ((f->attackGraphicOffset - 12) / 2);
		} else {
			f->graphicId = 745 + dir;
		}
	} else if (f->actionState == FigureActionState_151_EnemyInitial) {
		f->graphicId = 697 + dir + 8 * FigureActionMissileLauncherGraphicOffset(f);
	} else if (f->actionState == FigureActionState_149_Corpse) {
		f->graphicId = 793 + FigureActionCorpseGraphicOffset(f);
	} else if (f->direction == DIR_FIGURE_ATTACK) {
		f->graphicId = 745 + dir + 8 * (f->graphicOffset / 2);
	} else {
		f->graphicId = 601 + dir + 8 * f->graphicOffset;
	}
}

void FigureAction_enemy44_Sword(figure *f)
{
	const formation *m = formation_get(f->formationId);
	FigureActionIncreaseGraphicOffset(f, 12);
	f->cartGraphicId = 0;
	f->speedMultiplier = 1;
	FigureAction_enemyCommon(f);
	
	int dir = getDirection(f);
	
	f->isEnemyGraphic = 1;
	
	switch (m->enemy_type) {
		case ENEMY_5_PERGAMUM:
		case ENEMY_6_SELEUCID:
		case ENEMY_9_EGYPTIAN:
			break;
		default:
			return;
	}
	if (f->actionState == FigureActionState_150_Attack) {
		if (f->attackGraphicOffset >= 12) {
			f->graphicId = 545 + dir + 8 * ((f->attackGraphicOffset - 12) / 2);
		} else {
			f->graphicId = 545 + dir;
		}
	} else if (f->actionState == FigureActionState_149_Corpse) {
		f->graphicId = 593 + FigureActionCorpseGraphicOffset(f);
	} else if (f->direction == DIR_FIGURE_ATTACK) {
		f->graphicId = 545 + dir + 8 * (f->graphicOffset / 2);
	} else {
		f->graphicId = 449 + dir + 8 * f->graphicOffset;
	}
}

void FigureAction_enemy45_Sword(figure *f)
{
	const formation *m = formation_get(f->formationId);
	FigureActionIncreaseGraphicOffset(f, 12);
	f->cartGraphicId = 0;
	f->speedMultiplier = 1;
	FigureAction_enemyCommon(f);
	
	int dir = getDirection(f);
	
	f->isEnemyGraphic = 1;
	
	switch (m->enemy_type) {
		case ENEMY_7_ETRUSCAN:
		case ENEMY_8_GREEK:
		case ENEMY_10_CARTHAGINIAN:
			break;
		default:
			return;
	}
	if (f->actionState == FigureActionState_150_Attack) {
		if (f->attackGraphicOffset >= 12) {
			f->graphicId = 545 + dir + 8 * ((f->attackGraphicOffset - 12) / 2);
		} else {
			f->graphicId = 545 + dir;
		}
	} else if (f->actionState == FigureActionState_149_Corpse) {
		f->graphicId = 593 + FigureActionCorpseGraphicOffset(f);
	} else if (f->direction == DIR_FIGURE_ATTACK) {
		f->graphicId = 545 + dir + 8 * (f->graphicOffset / 2);
	} else {
		f->graphicId = 449 + dir + 8 * f->graphicOffset;
	}
}

void FigureAction_enemy46_Camel(figure *f)
{
	const formation *m = formation_get(f->formationId);
	FigureActionIncreaseGraphicOffset(f, 12);
	f->cartGraphicId = 0;
	f->speedMultiplier = 1;
	FigureAction_enemyCommon(f);
	
	int dir = getDirectionMissile(f, m);
	
	f->isEnemyGraphic = 1;
	
	if (f->direction == DIR_FIGURE_ATTACK) {
		f->graphicId = 601 + dir + 8 * f->graphicOffset;
	} else if (f->actionState == FigureActionState_150_Attack) {
		f->graphicId = 601 + dir;
	} else if (f->actionState == FigureActionState_151_EnemyInitial) {
		f->graphicId = 697 + dir + 8 * FigureActionMissileLauncherGraphicOffset(f);
	} else if (f->actionState == FigureActionState_149_Corpse) {
		f->graphicId = 745 + FigureActionCorpseGraphicOffset(f);
	} else {
		f->graphicId = 601 + dir + 8 * f->graphicOffset;
	}
}

void FigureAction_enemy47_Elephant(figure *f)
{
	FigureActionIncreaseGraphicOffset(f, 12);
	f->cartGraphicId = 0;
	f->speedMultiplier = 1;
	FigureAction_enemyCommon(f);
	
	int dir = getDirection(f);
	
	f->isEnemyGraphic = 1;
	
	if (f->direction == DIR_FIGURE_ATTACK || f->actionState == FigureActionState_150_Attack) {
		f->graphicId = 601 + dir + 8 * f->graphicOffset;
	} else if (f->actionState == FigureActionState_149_Corpse) {
		f->graphicId = 705 + FigureActionCorpseGraphicOffset(f);
	} else {
		f->graphicId = 601 + dir + 8 * f->graphicOffset;
	}
}

void FigureAction_enemy48_Chariot(figure *f)
{
	FigureActionIncreaseGraphicOffset(f, 12);
	f->cartGraphicId = 0;
	f->speedMultiplier = 3;
	FigureAction_enemyCommon(f);
	
	int dir = getDirection(f);
	
	f->isEnemyGraphic = 1;
	
	if (f->direction == DIR_FIGURE_ATTACK || f->actionState == FigureActionState_150_Attack) {
		f->graphicId = 697 + dir + 8 * (f->graphicOffset / 2);
	} else if (f->actionState == FigureActionState_149_Corpse) {
		f->graphicId = 745 + FigureActionCorpseGraphicOffset(f);
	} else {
		f->graphicId = 601 + dir + 8 * f->graphicOffset;
	}
}

void FigureAction_enemy49_FastSword(figure *f)
{
	const formation *m = formation_get(f->formationId);
	FigureActionIncreaseGraphicOffset(f, 12);
	f->cartGraphicId = 0;
	f->speedMultiplier = 2;
	FigureAction_enemyCommon(f);
	
	int dir = getDirection(f);
	
	f->isEnemyGraphic = 1;
	
	int attackId, corpseId, normalId;
	if (m->enemy_type == ENEMY_0_BARBARIAN) {
		attackId = 393;
		corpseId = 441;
		normalId = 297;
	} else if (m->enemy_type == ENEMY_1_NUMIDIAN) {
		attackId = 593;
		corpseId = 641;
		normalId = 449;
	} else if (m->enemy_type == ENEMY_4_GOTH) {
		attackId = 545;
		corpseId = 593;
		normalId = 449;
	} else {
		return;
	}
	if (f->actionState == FigureActionState_150_Attack) {
		if (f->attackGraphicOffset >= 12) {
			f->graphicId = attackId + dir + 8 * ((f->attackGraphicOffset - 12) / 2);
		} else {
			f->graphicId = attackId + dir;
		}
	} else if (f->actionState == FigureActionState_149_Corpse) {
		f->graphicId = corpseId + FigureActionCorpseGraphicOffset(f);
	} else if (f->direction == DIR_FIGURE_ATTACK) {
		f->graphicId = attackId + dir + 8 * (f->graphicOffset / 2);
	} else {
		f->graphicId = normalId + dir + 8 * f->graphicOffset;
	}
}

void FigureAction_enemy50_Sword(figure *f)
{
	const formation *m = formation_get(f->formationId);
	FigureActionIncreaseGraphicOffset(f, 12);
	f->cartGraphicId = 0;
	f->speedMultiplier = 1;
	FigureAction_enemyCommon(f);
	
	int dir = getDirection(f);
	
	f->isEnemyGraphic = 1;
	
	if (m->enemy_type != ENEMY_2_GAUL && m->enemy_type != ENEMY_3_CELT) {
		return;
	}
	if (f->actionState == FigureActionState_150_Attack) {
		if (f->attackGraphicOffset >= 12) {
			f->graphicId = 545 + dir + 8 * ((f->attackGraphicOffset - 12) / 2);
		} else {
			f->graphicId = 545 + dir;
		}
	} else if (f->actionState == FigureActionState_149_Corpse) {
		f->graphicId = 593 + FigureActionCorpseGraphicOffset(f);
	} else if (f->direction == DIR_FIGURE_ATTACK) {
		f->graphicId = 545 + dir + 8 * (f->graphicOffset / 2);
	} else {
		f->graphicId = 449 + dir + 8 * f->graphicOffset;
	}
}

void FigureAction_enemy51_Spear(figure *f)
{
	const formation *m = formation_get(f->formationId);
	FigureActionIncreaseGraphicOffset(f, 12);
	f->cartGraphicId = 0;
	f->speedMultiplier = 2;
	FigureAction_enemyCommon(f);
	
	int dir = getDirectionMissile(f, m);
	
	f->isEnemyGraphic = 1;
	
	if (m->enemy_type != ENEMY_1_NUMIDIAN) {
		return;
	}
	if (f->actionState == FigureActionState_150_Attack) {
		if (f->attackGraphicOffset >= 12) {
			f->graphicId = 593 + dir + 8 * ((f->attackGraphicOffset - 12) / 2);
		} else {
			f->graphicId = 593 + dir;
		}
	} else if (f->actionState == FigureActionState_151_EnemyInitial) {
		f->graphicId = 545 + dir + 8 * FigureActionMissileLauncherGraphicOffset(f);
	} else if (f->actionState == FigureActionState_149_Corpse) {
		f->graphicId = 641 + FigureActionCorpseGraphicOffset(f);
	} else if (f->direction == DIR_FIGURE_ATTACK) {
		f->graphicId = 593 + dir + 8 * (f->graphicOffset / 2);
	} else {
		f->graphicId = 449 + dir + 8 * f->graphicOffset;
	}
}

void FigureAction_enemy52_MountedArcher(figure *f)
{
	const formation *m = formation_get(f->formationId);
	FigureActionIncreaseGraphicOffset(f, 12);
	f->cartGraphicId = 0;
	f->speedMultiplier = 3;
	FigureAction_enemyCommon(f);
	
	int dir = getDirectionMissile(f, m);
	
	f->isEnemyGraphic = 1;
	
	if (f->direction == DIR_FIGURE_ATTACK) {
		f->graphicId = 601 + dir + 8 * f->graphicOffset;
	} else if (f->actionState == FigureActionState_150_Attack) {
		f->graphicId = 601 + dir;
	} else if (f->actionState == FigureActionState_151_EnemyInitial) {
		f->graphicId = 697 + dir + 8 * FigureActionMissileLauncherGraphicOffset(f);
	} else if (f->actionState == FigureActionState_149_Corpse) {
		f->graphicId = 745 + FigureActionCorpseGraphicOffset(f);
	} else {
		f->graphicId = 601 + dir + 8 * f->graphicOffset;
	}
}

void FigureAction_enemy53_Axe(figure *f)
{
	const formation *m = formation_get(f->formationId);
	FigureActionIncreaseGraphicOffset(f, 12);
	f->cartGraphicId = 0;
	f->speedMultiplier = 1;
	FigureAction_enemyCommon(f);
	
	int dir = getDirection(f);
	
	f->isEnemyGraphic = 1;
	
	if (m->enemy_type != ENEMY_2_GAUL) {
		return;
	}
	if (f->actionState == FigureActionState_150_Attack) {
		if (f->attackGraphicOffset >= 12) {
			f->graphicId = 697 + dir + 8 * ((f->attackGraphicOffset - 12) / 2);
		} else {
			f->graphicId = 697 + dir;
		}
	} else if (f->actionState == FigureActionState_149_Corpse) {
		f->graphicId = 745 + FigureActionCorpseGraphicOffset(f);
	} else if (f->direction == DIR_FIGURE_ATTACK) {
		f->graphicId = 697 + dir + 8 * (f->graphicOffset / 2);
	} else {
		f->graphicId = 601 + dir + 8 * f->graphicOffset;
	}
}

void FigureAction_enemy54_Gladiator(figure *f)
{
	f->terrainUsage = FigureTerrainUsage_Any;
	f->useCrossCountry = 0;
	FigureActionIncreaseGraphicOffset(f, 12);
	if (scenario_gladiator_revolt_is_finished()) {
		// end of gladiator revolt: kill gladiators
		if (f->actionState != FigureActionState_149_Corpse) {
			f->actionState = FigureActionState_149_Corpse;
			f->waitTicks = 0;
			f->direction = 0;
		}
	}
	switch (f->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(f);
			FigureActionIncreaseGraphicOffset(f, 16);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(f);
			break;
		case FigureActionState_158_NativeCreated:
			f->graphicOffset = 0;
			f->waitTicks++;
			if (f->waitTicks > 10 + (f->id & 3)) {
				f->waitTicks = 0;
				f->actionState = FigureActionState_159_NativeAttacking;
				int xTile, yTile;
				int buildingId = Formation_Rioter_getTargetBuilding(&xTile, &yTile);
				if (buildingId) {
					f->destinationX = xTile;
					f->destinationY = yTile;
					f->destinationBuildingId = buildingId;
					figure_route_remove(f);
				} else {
					f->state = FigureState_Dead;
				}
			}
			break;
		case FigureActionState_159_NativeAttacking:
			Data_CityInfo.numAttackingNativesInCity = 10;
			f->terrainUsage = FigureTerrainUsage_Enemy;
			FigureMovement_walkTicks(f, 1);
			if (f->direction == DIR_FIGURE_AT_DESTINATION ||
				f->direction == DIR_FIGURE_REROUTE ||
				f->direction == DIR_FIGURE_LOST) {
				f->actionState = FigureActionState_158_NativeCreated;
			}
			break;
	}
	int dir;
	if (f->actionState == FigureActionState_150_Attack || f->direction == DIR_FIGURE_ATTACK) {
		dir = f->attackDirection;
	} else if (f->direction < 8) {
		dir = f->direction;
	} else {
		dir = f->previousTileDirection;
	}
	FigureActionNormalizeDirection(dir);

	if (f->actionState == FigureActionState_150_Attack || f->direction == DIR_FIGURE_ATTACK) {
		f->graphicId = image_group(GROUP_FIGURE_GLADIATOR) + dir + 104 + 8 * (f->graphicOffset / 2);
	} else if (f->actionState == FigureActionState_149_Corpse) {
		f->graphicId = image_group(GROUP_FIGURE_GLADIATOR) + 96 + FigureActionCorpseGraphicOffset(f);
	} else {
		f->graphicId = image_group(GROUP_FIGURE_GLADIATOR) + dir + 8 * f->graphicOffset;
	}
}

void FigureAction_enemyCaesarLegionary(figure *f)
{
	const formation *m = formation_get(f->formationId);
	Data_CityInfo.numImperialSoldiersInCity++;
	FigureActionIncreaseGraphicOffset(f, 12);
	f->cartGraphicId = 0;
	f->speedMultiplier = 1;
	FigureAction_enemyCommon(f);
	
	int dir = getDirection(f);
	
	if (f->direction == DIR_FIGURE_ATTACK) {
		f->graphicId = image_group(GROUP_FIGURE_CAESAR_LEGIONARY) + dir +
			8 * ((f->attackGraphicOffset - 12) / 2);
	}
	switch (f->actionState) {
		case FigureActionState_150_Attack:
			if (f->attackGraphicOffset >= 12) {
				f->graphicId = image_group(GROUP_FIGURE_CAESAR_LEGIONARY) + dir +
					8 * ((f->attackGraphicOffset - 12) / 2);
			} else {
				f->graphicId = image_group(GROUP_FIGURE_CAESAR_LEGIONARY) + dir;
			}
			break;
		case FigureActionState_149_Corpse:
			f->graphicId = image_group(GROUP_FIGURE_CAESAR_LEGIONARY) +
				FigureActionCorpseGraphicOffset(f) + 152;
			break;
		case FigureActionState_84_SoldierAtStandard:
			if (m->is_halted && m->layout == FORMATION_COLUMN && m->missile_attack_timeout) {
				f->graphicId = image_group(GROUP_BUILDING_FORT_LEGIONARY) + dir + 144;
			} else {
				f->graphicId = image_group(GROUP_BUILDING_FORT_LEGIONARY) + dir;
			}
			break;
		default:
			f->graphicId = image_group(GROUP_FIGURE_CAESAR_LEGIONARY) + 48 + dir + 8 * f->graphicOffset;
			break;
	}
}

int FigureAction_HerdEnemy_moveFormationTo(int formationId, int x, int y, int *xTile, int *yTile)
{
	const formation *m = formation_get(formationId);
	int baseOffset = map_grid_offset(
		FigureActionFormationLayoutPositionX(m->layout, 0),
		FigureActionFormationLayoutPositionY(m->layout, 0));
	int figureOffsets[50];
	figureOffsets[0] = 0;
	for (int i = 1; i < m->num_figures; i++) {
		figureOffsets[i] = map_grid_offset(
			FigureActionFormationLayoutPositionX(m->layout, i),
			FigureActionFormationLayoutPositionY(m->layout, i)) - baseOffset;
	}
	map_routing_noncitizen_can_travel_over_land(x, y, -1, -1, 0, 600);
	for (int r = 0; r <= 10; r++) {
		int xMin = x - r;
		int yMin = y - r;
		int xMax = x + r;
		int yMax = y + r;
		map_grid_bound_area(&xMin, &yMin, &xMax, &yMax);
		for (int yy = yMin; yy <= yMax; yy++) {
			for (int xx = xMin; xx <= xMax; xx++) {
				int canMove = 1;
				for (int fig = 0; fig < m->num_figures; fig++) {
					int gridOffset = map_grid_offset(xx, yy) + figureOffsets[fig];
                    if (gridOffset < 0 || gridOffset >= GRID_SIZE * GRID_SIZE) {
                        canMove = 0;
                        break;
                    }
					if (Data_Grid_terrain[gridOffset] & Terrain_1237) {
						canMove = 0;
						break;
					}
					if (map_routing_distance(gridOffset) <= 0) {
						canMove = 0;
						break;
					}
					if (map_has_figure_at(gridOffset) &&
						figure_get(map_figure_at(gridOffset))->formationId != formationId) {
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

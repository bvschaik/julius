#include "FigureAction_private.h"

#include "Figure.h"
#include "Formation.h"
#include "Sound.h"
#include "Data/Formation.h"

static const int cloudGraphicOffsets[] = {
	0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2,
	2, 2, 2, 2, 3, 3, 3, 4, 4, 5, 6, 7
};

void FigureAction_explosionCloud(int walkerId)
{
	struct Data_Walker *f = &Data_Walkers[walkerId];
	f->useCrossCountry = 1;
	f->progressOnTile++;
	if (f->progressOnTile > 44) {
		f->state = FigureState_Dead;
	}
	FigureMovement_crossCountryWalkTicks(walkerId, f->speedMultiplier);
	if (f->progressOnTile < 48) {
		f->graphicId = GraphicId(ID_Graphic_Figure_Explosion) +
			cloudGraphicOffsets[f->progressOnTile / 2];
	} else {
		f->graphicId = GraphicId(ID_Graphic_Figure_Explosion) + 7;
	}
}

void FigureAction_arrow(int walkerId)
{
	struct Data_Walker *f = &Data_Walkers[walkerId];
	f->useCrossCountry = 1;
	f->progressOnTile++;
	if (f->progressOnTile > 120) {
		f->state = FigureState_Dead;
	}
	int shouldDie = FigureMovement_crossCountryWalkTicks(walkerId, 4);
	int targetId = Figure_getCitizenOnSameTile(walkerId);
	if (targetId) {
		int targetType = Data_Walkers[targetId].type;
		int formationId = Data_Walkers[targetId].formationId;
		int maxDamage = Constant_FigureProperties[targetType].maxDamage;
		int damageInflicted =
			Constant_FigureProperties[f->type].missileAttackValue -
			Constant_FigureProperties[targetType].missileDefenseValue;
		if (damageInflicted < 0) {
			damageInflicted = 0;
		}
		if (targetType == Figure_FortLegionary &&
			Data_Formations[formationId].isHalted &&
			Data_Formations[formationId].layout == FormationLayout_Tortoise) {
			damageInflicted = 1;
		}
		int targetDamage = damageInflicted + Data_Walkers[targetId].damage;
		if (targetDamage <= maxDamage) {
			Data_Walkers[targetId].damage = targetDamage;
		} else { // kill target
			Data_Walkers[targetId].damage = maxDamage + 1;
			Data_Walkers[targetId].actionState = FigureActionState_149_Corpse;
			Data_Walkers[targetId].waitTicks = 0;
			Figure_playDieSound(targetType);
			Formation_updateAfterDeath(formationId);
		}
		f->state = FigureState_Dead;
		int arrowFormation = Data_Walkers[f->buildingId].formationId;
		Data_Formations[formationId].missileAttackTimeout = 6;
		Data_Formations[formationId].missileAttackFormationId = arrowFormation;
		Sound_Effects_playChannel(SoundChannel_ArrowHit);
	} else if (shouldDie) {
		f->state = FigureState_Dead;
	}
	int dir = (16 + f->direction - 2 * Data_Settings_Map.orientation) % 16;
	f->graphicId = GraphicId(ID_Graphic_Figure_Missile) + 16 + dir;
}

void FigureAction_spear(int walkerId)
{
	struct Data_Walker *f = &Data_Walkers[walkerId];
	f->useCrossCountry = 1;
	f->progressOnTile++;
	if (f->progressOnTile > 120) {
		f->state = FigureState_Dead;
	}
	int shouldDie = FigureMovement_crossCountryWalkTicks(walkerId, 4);
	int targetId = Figure_getCitizenOnSameTile(walkerId);
	if (targetId) {
		int targetType = Data_Walkers[targetId].type;
		int formationId = Data_Walkers[targetId].formationId;
		int maxDamage = Constant_FigureProperties[targetType].maxDamage;
		int damageInflicted =
			Constant_FigureProperties[f->type].missileAttackValue -
			Constant_FigureProperties[targetType].missileDefenseValue;
		if (damageInflicted < 0) {
			damageInflicted = 0;
		}
		if (targetType == Figure_FortLegionary &&
			Data_Formations[formationId].isHalted &&
			Data_Formations[formationId].layout == FormationLayout_Tortoise) {
			damageInflicted = 1;
		}
		int targetDamage = damageInflicted + Data_Walkers[targetId].damage;
		if (targetDamage <= maxDamage) {
			Data_Walkers[targetId].damage = targetDamage;
		} else { // kill target
			Data_Walkers[targetId].damage = maxDamage + 1;
			Data_Walkers[targetId].actionState = FigureActionState_149_Corpse;
			Data_Walkers[targetId].waitTicks = 0;
			Figure_playDieSound(targetType);
			Formation_updateAfterDeath(formationId);
		}
		int arrowFormation = Data_Walkers[f->buildingId].formationId;
		Data_Formations[formationId].missileAttackTimeout = 6;
		Data_Formations[formationId].missileAttackFormationId = arrowFormation;
		Sound_Effects_playChannel(SoundChannel_Javelin);
		f->state = FigureState_Dead;
	} else if (shouldDie) {
		f->state = FigureState_Dead;
	}
	int dir = (16 + f->direction - 2 * Data_Settings_Map.orientation) % 16;
	f->graphicId = GraphicId(ID_Graphic_Figure_Missile) + dir;
}

void FigureAction_javelin(int walkerId)
{
	struct Data_Walker *f = &Data_Walkers[walkerId];
	f->useCrossCountry = 1;
	f->progressOnTile++;
	if (f->progressOnTile > 120) {
		f->state = FigureState_Dead;
	}
	int shouldDie = FigureMovement_crossCountryWalkTicks(walkerId, 4);
	int targetId = Figure_getNonCitizenOnSameTile(walkerId);
	if (targetId) {
		int targetType = Data_Walkers[targetId].type;
		int formationId = Data_Walkers[targetId].formationId;
		int maxDamage = Constant_FigureProperties[targetType].maxDamage;
		int damageInflicted =
			Constant_FigureProperties[f->type].missileAttackValue -
			Constant_FigureProperties[targetType].missileDefenseValue;
		if (damageInflicted < 0) {
			damageInflicted = 0;
		}
		if (targetType == Figure_EnemyCaesarLegionary &&
			Data_Formations[formationId].isHalted &&
			Data_Formations[formationId].layout == FormationLayout_Tortoise) {
			damageInflicted = 1;
		}
		int targetDamage = damageInflicted + Data_Walkers[targetId].damage;
		if (targetDamage <= maxDamage) {
			Data_Walkers[targetId].damage = targetDamage;
		} else { // kill target
			Data_Walkers[targetId].damage = maxDamage + 1;
			Data_Walkers[targetId].actionState = FigureActionState_149_Corpse;
			Data_Walkers[targetId].waitTicks = 0;
			Figure_playDieSound(targetType);
			Formation_updateAfterDeath(formationId);
		}
		int javelinFormation = Data_Walkers[f->buildingId].formationId;
		Data_Formations[formationId].missileAttackTimeout = 6;
		Data_Formations[formationId].missileAttackFormationId = javelinFormation;
		Sound_Effects_playChannel(SoundChannel_Javelin);
		f->state = FigureState_Dead;
	} else if (shouldDie) {
		f->state = FigureState_Dead;
	}
	int dir = (16 + f->direction - 2 * Data_Settings_Map.orientation) % 16;
	f->graphicId = GraphicId(ID_Graphic_Figure_Missile) + dir;
}

void FigureAction_bolt(int walkerId)
{
	struct Data_Walker *f = &Data_Walkers[walkerId];
	f->useCrossCountry = 1;
	f->progressOnTile++;
	if (f->progressOnTile > 120) {
		f->state = FigureState_Dead;
	}
	int shouldDie = FigureMovement_crossCountryWalkTicks(walkerId, 4);
	int targetId = Figure_getNonCitizenOnSameTile(walkerId);
	if (targetId) {
		int targetType = Data_Walkers[targetId].type;
		int formationId = Data_Walkers[targetId].formationId;
		int maxDamage = Constant_FigureProperties[targetType].maxDamage;
		int damageInflicted =
			Constant_FigureProperties[f->type].missileAttackValue -
			Constant_FigureProperties[targetType].missileDefenseValue;
		if (damageInflicted < 0) {
			damageInflicted = 0;
		}
		int targetDamage = damageInflicted + Data_Walkers[targetId].damage;
		if (targetDamage <= maxDamage) {
			Data_Walkers[targetId].damage = targetDamage;
		} else { // kill target
			Data_Walkers[targetId].damage = maxDamage + 1;
			Data_Walkers[targetId].actionState = FigureActionState_149_Corpse;
			Data_Walkers[targetId].waitTicks = 0;
			Figure_playDieSound(targetType);
			Formation_updateAfterDeath(formationId);
		}
		Sound_Effects_playChannel(SoundChannel_BallistaHitPerson);
		f->state = FigureState_Dead;
	} else if (shouldDie) {
		f->state = FigureState_Dead;
		Sound_Effects_playChannel(SoundChannel_BallistaHitGround);
	}
	int dir = (16 + f->direction - 2 * Data_Settings_Map.orientation) % 16;
	f->graphicId = GraphicId(ID_Graphic_Figure_Missile) + 32 + dir;
}

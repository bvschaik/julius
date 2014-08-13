#include "WalkerAction_private.h"

#include "Formation.h"
#include "Sound.h"
#include "Walker.h"
#include "Data/Formation.h"

static const int cloudGraphicOffsets[] = {
	0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2,
	2, 2, 2, 2, 3, 3, 3, 4, 4, 5, 6, 7
};

void WalkerAction_explosionCloud(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->useCrossCountry = 1;
	w->progressOnTile++;
	if (w->progressOnTile > 44) {
		w->state = WalkerState_Dead;
	}
	WalkerMovement_crossCountryWalkTicks(walkerId, w->speedMultiplier);
	if (w->progressOnTile < 48) {
		w->graphicId = GraphicId(ID_Graphic_Walker_Explosion) +
			cloudGraphicOffsets[w->progressOnTile / 2]; // dword_5FA2A8
	} else {
		w->graphicId = GraphicId(ID_Graphic_Walker_Explosion) + 7;
	}
}

void WalkerAction_arrow(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->useCrossCountry = 1;
	w->progressOnTile++;
	if (w->progressOnTile > 120) {
		w->state = WalkerState_Dead;
	}
	int shouldDie = WalkerMovement_crossCountryWalkTicks(walkerId, 4);
	int targetId = Walker_getCitizenOnSameTile(walkerId);
	if (targetId) {
		int targetType = Data_Walkers[targetId].type;
		int formationId = Data_Walkers[targetId].formationId;
		int maxDamage = Constant_WalkerProperties[targetType].maxDamage;
		int damageInflicted =
			Constant_WalkerProperties[w->type].missileAttackValue -
			Constant_WalkerProperties[targetType].missileDefenseValue;
		if (damageInflicted < 0) {
			damageInflicted = 0;
		}
		if (targetType == Walker_FortLegionary &&
			Data_Formations[formationId].isHalted &&
			Data_Formations[formationId].layout == FormationLayout_Tortoise) {
			damageInflicted = 1;
		}
		int targetDamage = damageInflicted + Data_Walkers[targetId].damage;
		if (targetDamage <= maxDamage) {
			Data_Walkers[targetId].damage = targetDamage;
		} else { // kill target
			Data_Walkers[targetId].damage = maxDamage + 1;
			Data_Walkers[targetId].actionState = WalkerActionState_149_Corpse;
			Data_Walkers[targetId].waitTicks = 0;
			Walker_playDieSound(targetType);
			Formation_updateAfterDeath(formationId);
		}
		w->state = WalkerState_Dead;
		int arrowFormation = Data_Buildings[w->buildingId].formationId;
		Data_Formations[formationId].missileAttackTimeout = 6;
		Data_Formations[formationId].missileAttackFormationId = arrowFormation;
		Sound_Effects_playChannel(SoundChannel_ArrowHit);
	} else if (shouldDie) {
		w->state = WalkerState_Dead;
	}
	int dir = (16 + w->direction - 2 * Data_Settings_Map.orientation) % 16;
	w->graphicId = GraphicId(ID_Graphic_Walker_Missile) + 16 + dir;
}

void WalkerAction_spear(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->useCrossCountry = 1;
	w->progressOnTile++;
	if (w->progressOnTile > 120) {
		w->state = WalkerState_Dead;
	}
	int shouldDie = WalkerMovement_crossCountryWalkTicks(walkerId, 4);
	int targetId = Walker_getCitizenOnSameTile(walkerId);
	if (targetId) {
		int targetType = Data_Walkers[targetId].type;
		int formationId = Data_Walkers[targetId].formationId;
		int maxDamage = Constant_WalkerProperties[targetType].maxDamage;
		int damageInflicted =
			Constant_WalkerProperties[w->type].missileAttackValue -
			Constant_WalkerProperties[targetType].missileDefenseValue;
		if (damageInflicted < 0) {
			damageInflicted = 0;
		}
		if (targetType == Walker_FortLegionary &&
			Data_Formations[formationId].isHalted &&
			Data_Formations[formationId].layout == FormationLayout_Tortoise) {
			damageInflicted = 1;
		}
		int targetDamage = damageInflicted + Data_Walkers[targetId].damage;
		if (targetDamage <= maxDamage) {
			Data_Walkers[targetId].damage = targetDamage;
		} else { // kill target
			Data_Walkers[targetId].damage = maxDamage + 1;
			Data_Walkers[targetId].actionState = WalkerActionState_149_Corpse;
			Data_Walkers[targetId].waitTicks = 0;
			Walker_playDieSound(targetType);
			Formation_updateAfterDeath(formationId);
		}
		int arrowFormation = Data_Buildings[w->buildingId].formationId;
		Data_Formations[formationId].missileAttackTimeout = 6;
		Data_Formations[formationId].missileAttackFormationId = arrowFormation;
		Sound_Effects_playChannel(SoundChannel_Javelin);
		w->state = WalkerState_Dead;
	} else if (shouldDie) {
		w->state = WalkerState_Dead;
	}
	int dir = (16 + w->direction - 2 * Data_Settings_Map.orientation) % 16;
	w->graphicId = GraphicId(ID_Graphic_Walker_Missile) + dir;
}

void WalkerAction_javelin(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->useCrossCountry = 1;
	w->progressOnTile++;
	if (w->progressOnTile > 120) {
		w->state = WalkerState_Dead;
	}
	int shouldDie = WalkerMovement_crossCountryWalkTicks(walkerId, 4);
	int targetId = Walker_getNonCitizenOnSameTile(walkerId);
	if (targetId) {
		int targetType = Data_Walkers[targetId].type;
		int formationId = Data_Walkers[targetId].formationId;
		int maxDamage = Constant_WalkerProperties[targetType].maxDamage;
		int damageInflicted =
			Constant_WalkerProperties[w->type].missileAttackValue -
			Constant_WalkerProperties[targetType].missileDefenseValue;
		if (damageInflicted < 0) {
			damageInflicted = 0;
		}
		if (targetType == Walker_EnemyCaesarLegionary &&
			Data_Formations[formationId].isHalted &&
			Data_Formations[formationId].layout == FormationLayout_Tortoise) {
			damageInflicted = 1;
		}
		int targetDamage = damageInflicted + Data_Walkers[targetId].damage;
		if (targetDamage <= maxDamage) {
			Data_Walkers[targetId].damage = targetDamage;
		} else { // kill target
			Data_Walkers[targetId].damage = maxDamage + 1;
			Data_Walkers[targetId].actionState = WalkerActionState_149_Corpse;
			Data_Walkers[targetId].waitTicks = 0;
			Walker_playDieSound(targetType);
			Formation_updateAfterDeath(formationId);
		}
		int javelinFormation = Data_Buildings[w->buildingId].formationId;
		Data_Formations[formationId].missileAttackTimeout = 6;
		Data_Formations[formationId].missileAttackFormationId = javelinFormation;
		Sound_Effects_playChannel(SoundChannel_Javelin);
		w->state = WalkerState_Dead;
	} else if (shouldDie) {
		w->state = WalkerState_Dead;
	}
	int dir = (16 + w->direction - 2 * Data_Settings_Map.orientation) % 16;
	w->graphicId = GraphicId(ID_Graphic_Walker_Missile) + dir;
}

void WalkerAction_bolt(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->useCrossCountry = 1;
	w->progressOnTile++;
	if (w->progressOnTile > 120) {
		w->state = WalkerState_Dead;
	}
	int shouldDie = WalkerMovement_crossCountryWalkTicks(walkerId, 4);
	int targetId = Walker_getNonCitizenOnSameTile(walkerId);
	if (targetId) {
		int targetType = Data_Walkers[targetId].type;
		int formationId = Data_Walkers[targetId].formationId;
		int maxDamage = Constant_WalkerProperties[targetType].maxDamage;
		int damageInflicted =
			Constant_WalkerProperties[w->type].missileAttackValue -
			Constant_WalkerProperties[targetType].missileDefenseValue;
		if (damageInflicted < 0) {
			damageInflicted = 0;
		}
		int targetDamage = damageInflicted + Data_Walkers[targetId].damage;
		if (targetDamage <= maxDamage) {
			Data_Walkers[targetId].damage = targetDamage;
		} else { // kill target
			Data_Walkers[targetId].damage = maxDamage + 1;
			Data_Walkers[targetId].actionState = WalkerActionState_149_Corpse;
			Data_Walkers[targetId].waitTicks = 0;
			Walker_playDieSound(targetType);
			Formation_updateAfterDeath(formationId);
		}
		Sound_Effects_playChannel(SoundChannel_BallistaHitPerson);
		w->state = WalkerState_Dead;
	} else if (shouldDie) {
		w->state = WalkerState_Dead;
		Sound_Effects_playChannel(SoundChannel_BallistaHitGround);
	}
	int dir = (16 + w->direction - 2 * Data_Settings_Map.orientation) % 16;
	w->graphicId = GraphicId(ID_Graphic_Walker_Missile) + 32 + dir;
}

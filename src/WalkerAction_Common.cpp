#include "WalkerAction_private.h"

#include "Formation.h"
#include "Sound.h"
#include "Walker.h"

#include "Data/Constants.h"
#include "Data/Formation.h"

const int walkerActionCorpseGraphicOffsets[128] = {
	0, 1, 2, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
};

const int walkerActionMissileLauncherGraphicOffsets[128] = {
	0, 1, 2, 3, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const int walkerActionFormationLayoutPositionX[13][16] = {
	{0, 1, 0, 1, -1, -1, 0, 1, -1, 2, 2, 2, 0, 1, -1, 2},
	{0, 0, -1, 1, -1, 1, -2, -2, 2, 2, -3, -3, 3, 3, -4, -4},
	{0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1},
	{0, 2, -2, 1, -1, 3, -3, 4, -4, 5, 6, -5, -6, 7, 8, -7},
	{0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1},
	{0, 0, 1, 0, -1, 1, 1, -1, -1, 1, 1, -1, -1, 1, -1, 0},
	{0, 1, 0, 1, 2, 2, 1, 0, 2, 3, 3, 3, 1, 2, 0, 3},
	{0, 1, 0, 1, 2, 2, 1, 0, 2, 3, 3, 3, 1, 2, 0, 3},
	{0, 1, 0, 0, 1, -1, 2, -1, 1, 0, 1, 0, 1, -1, 1, -1},
	{0, 2, -1, 1, 1, -1, 3, -2, 0, -4, -1, 0, 1, 4, 2, -5}, // herd
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 2, 0, 2, -2, -2, 0, 2, -2, 4, 4, 4, 0, 2, -2, 4},
	{0, 1, 0, 1, 2, 2, 1, 0, 2, 3, 3, 3, 1, 2, 0, 3}
};
const int walkerActionFormationLayoutPositionY[13][16] = {
	{0, 0, 1, 1, 0, 1, -1, -1, -1, -1, 0, 1, 2, 2, 2, 2},
	{0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1},
	{0, -1, 1, 0, -1, 1, -2, -2, 2, 2, -3, -3, 3, 3, -4, -4},
	{0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1},
	{0, -2, 2, -1, 1, -3, 3, -4, 4, -5, -6, 5, 6, -7, -8, 7},
	{0, -1, 0, 1, 0, -1, 1, 1, -1, -1, 1, 1, -1, 0, 0, 0},
	{0, 0, 1, 1, 0, 1, 2, 2, 2, 0, 1, 2, 3, 3, 3, 3},
	{0, 0, 1, 1, 0, 1, 2, 2, 2, 0, 1, 2, 3, 3, 3, 3},
	{0, -1, 1, 0, 0, 1, 1, -1, -1, 1, 0, 2, 1, 1, -2, 1},
	{0, 1, -1, 1, 0, 1, 1, -1, 2, 0, 3, 5, 4, 0, 3, 2}, // herd
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 2, 2, 0, 2, -2, -2, -2, -2, 0, 2, 4, 4, 4, 4},
	{0, 0, 1, 1, 0, 1, 2, 2, 2, 0, 1, 2, 3, 3, 3, 3}
};

static const int cartOffsetsX[] = {13, 18, 12, 0, -13, -18, -13, 0};
static const int cartOffsetsY[] = {-7, -1, 7, 11, 6, -1, -7, -12};

void WalkerAction_Common_handleCorpse(int walkerId)
{
	if (Data_Walkers[walkerId].waitTicks < 0) {
		Data_Walkers[walkerId].waitTicks = 0;
	}
	Data_Walkers[walkerId].waitTicks++;
	if (Data_Walkers[walkerId].waitTicks >= 128) {
		Data_Walkers[walkerId].waitTicks = 127;
		Data_Walkers[walkerId].state = WalkerState_Dead;
	}
}

static int attackIsSameDirection(int dir1, int dir2)
{
	if (dir1 == dir2) {
		return 1;
	}
	int dir2Off = dir2 <= 0 ? 7 : dir2 - 1;
	if (dir1 == dir2Off) {
		return 1;
	}
	dir2Off = dir2 >= 7 ? 0 : dir2 + 1;
	if (dir1 == dir2Off) {
		return 1;
	}
	return 0;
}

static void resumeActivityAfterAttack(int walkerId, struct Data_Walker *w)
{
	w->numAttackers = 0;
	w->actionState = w->actionStateBeforeAttack;
	w->opponentId = 0;
	w->attackerId1 = 0;
	w->attackerId2 = 0;
	WalkerRoute_remove(walkerId);
}

static void hitOpponent(int walkerId, struct Data_Walker *w)
{
	struct Data_Formation *f = &Data_Formations[w->formationId];
	struct Data_Walker *opponent = &Data_Walkers[w->opponentId];
	struct Data_Formation *opponentFormation = &Data_Formations[opponent->formationId];
	
	int cat = Constant_WalkerProperties[opponent->type].category;
	if (cat == WalkerCategory_Citizen || cat == WalkerCategory_Criminal) {
		w->attackGraphicOffset = 12;
	} else {
		w->attackGraphicOffset = 0;
	}
	int walkerAttack = Constant_WalkerProperties[w->type].attackValue;
	int opponentDefense = Constant_WalkerProperties[opponent->type].defenseValue;
	
	// attack modifiers
	if (w->type == Walker_Wolf) {
		switch (Data_Settings.difficulty) {
			case Difficulty_VeryEasy: walkerAttack = 2; break;
			case Difficulty_Easy: walkerAttack = 4; break;
			case Difficulty_Normal: walkerAttack = 6; break;
		}
	}
	if (opponent->opponentId != walkerId && f->walkerType != Walker_FortLegionary &&
			attackIsSameDirection(w->attackDirection, opponent->attackDirection)) {
		walkerAttack += 4; // attack opponent on the (exposed) back
		Sound_Effects_playChannel(SoundChannel_SwordSwing);
	}
	if (f->isHalted && f->walkerType == Walker_FortLegionary &&
			attackIsSameDirection(w->attackDirection, f->direction)) {
		walkerAttack += 4; // coordinated formation attack bonus
	}
	// defense modifiers
	if (opponentFormation->isHalted &&
			(opponentFormation->walkerType == Walker_FortLegionary ||
			opponentFormation->walkerType == Walker_EnemyCaesarLegionary)) {
		if (!attackIsSameDirection(opponent->attackDirection, opponentFormation->direction)) {
			opponentDefense -= 4; // opponent not attacking in coordinated formation
		} else if (opponentFormation->layout == FormationLayout_Tortoise) {
			opponentDefense += 7;
		} else if (opponentFormation->layout == FormationLayout_DoubleLine1 ||
				opponentFormation->layout == FormationLayout_DoubleLine2) {
			opponentDefense += 4;
		}
	}
	
	int maxDamage = Constant_WalkerProperties[opponent->type].maxDamage;
	int netAttack = walkerAttack - opponentDefense;
	if (netAttack < 0) {
		netAttack = 0;
	}
	opponent->damage += netAttack;
	if (opponent->damage <= maxDamage) {
		Walker_playHitSound(w->type);
	} else {
		opponent->actionState = WalkerActionState_149_Corpse;
		opponent->waitTicks = 0;
		Walker_playDieSound(opponent->type);
		Formation_updateAfterDeath(opponent->formationId);
	}
}

void WalkerAction_Common_handleAttack(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	
	if (w->progressOnTile <= 5) {
		w->progressOnTile++;
		WalkerMovement_advanceTick(w);
	}
	if (w->numAttackers == 0) {
		resumeActivityAfterAttack(walkerId, w);
		return;
	}
	if (w->numAttackers == 1) {
		int targetId = w->opponentId;
		if (WalkerIsDead(targetId)) {
			resumeActivityAfterAttack(walkerId, w);
			return;
		}
	} else if (w->numAttackers == 2) {
		int targetId = w->opponentId;
		if (WalkerIsDead(targetId)) {
			if (targetId == w->attackerId1) {
				w->opponentId = w->attackerId2;
			} else if (targetId == w->attackerId2) {
				w->opponentId = w->attackerId1;
			}
			targetId = w->opponentId;
			if (WalkerIsDead(targetId)) {
				resumeActivityAfterAttack(walkerId, w);
				return;
			}
			w->numAttackers = 1;
			w->attackerId1 = targetId;
			w->attackerId2 = 0;
		}
	}
	w->attackGraphicOffset++;
	if (w->attackGraphicOffset >= 24) {
		hitOpponent(walkerId, w);
	}
}

void WalkerAction_Common_setCartOffset(int walkerId, int direction)
{
	Data_Walkers[walkerId].xOffsetCart = cartOffsetsX[direction];
	Data_Walkers[walkerId].yOffsetCart = cartOffsetsY[direction];
}

void WalkerAction_Common_setCrossCountryDestination(int walkerId, struct Data_Walker *w, int xDst, int yDst)
{
	w->destinationX = xDst;
	w->destinationY = yDst;
	WalkerMovement_crossCountrySetDirection(
		walkerId, w->crossCountryX, w->crossCountryY,
		15 * xDst, 15 * yDst, 0);
}

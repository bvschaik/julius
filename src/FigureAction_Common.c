#include "FigureAction_private.h"

#include "Figure.h"
#include "Formation.h"

#include "figure/formation.h"
#include "figure/properties.h"
#include "figure/route.h"
#include "figure/sound.h"
#include "game/difficulty.h"
#include "sound/effect.h"

const int figureActionMissileLauncherGraphicOffsets[128] = {
	0, 1, 2, 3, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void FigureAction_Common_handleCorpse(figure *f)
{
	if (f->waitTicks < 0) {
		f->waitTicks = 0;
	}
	f->waitTicks++;
	if (f->waitTicks >= 128) {
		f->waitTicks = 127;
		f->state = FigureState_Dead;
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

static void resumeActivityAfterAttack(figure *f)
{
	f->numAttackers = 0;
	f->actionState = f->actionStateBeforeAttack;
	f->opponentId = 0;
	f->attackerId1 = 0;
	f->attackerId2 = 0;
	figure_route_remove(f);
}

static void hitOpponent(figure *f)
{
	const formation *m = formation_get(f->formationId);
	figure *opponent = figure_get(f->opponentId);
	const formation *opponentFormation = formation_get(opponent->formationId);
	
	const figure_properties *props = figure_properties_for_type(f->type);
	const figure_properties *opponent_props = figure_properties_for_type(opponent->type);
	int cat = opponent_props->category;
	if (cat == FIGURE_CATEGORY_CITIZEN || cat == FIGURE_CATEGORY_CRIMINAL) {
		f->attackGraphicOffset = 12;
	} else {
		f->attackGraphicOffset = 0;
	}
	int figureAttack = props->attack_value;
	int opponentDefense = opponent_props->defense_value;
	
	// attack modifiers
	if (f->type == FIGURE_WOLF) {
	    figureAttack = difficulty_adjust_wolf_attack(figureAttack);
	}
	if (opponent->opponentId != f->id && m->figure_type != FIGURE_FORT_LEGIONARY &&
			attackIsSameDirection(f->attackDirection, opponent->attackDirection)) {
		figureAttack += 4; // attack opponent on the (exposed) back
		sound_effect_play(SOUND_EFFECT_SWORD_SWING);
	}
	if (m->is_halted && m->figure_type == FIGURE_FORT_LEGIONARY &&
			attackIsSameDirection(f->attackDirection, m->direction)) {
		figureAttack += 4; // coordinated formation attack bonus
	}
	// defense modifiers
	if (opponentFormation->is_halted &&
			(opponentFormation->figure_type == FIGURE_FORT_LEGIONARY ||
			opponentFormation->figure_type == FIGURE_ENEMY_CAESAR_LEGIONARY)) {
		if (!attackIsSameDirection(opponent->attackDirection, opponentFormation->direction)) {
			opponentDefense -= 4; // opponent not attacking in coordinated formation
		} else if (opponentFormation->layout == FORMATION_COLUMN) {
			opponentDefense += 7;
		} else if (opponentFormation->layout == FORMATION_DOUBLE_LINE_1 ||
				opponentFormation->layout == FORMATION_DOUBLE_LINE_2) {
			opponentDefense += 4;
		}
	}
	
	int maxDamage = opponent_props->max_damage;
	int netAttack = figureAttack - opponentDefense;
	if (netAttack < 0) {
		netAttack = 0;
	}
	opponent->damage += netAttack;
	if (opponent->damage <= maxDamage) {
		figure_play_hit_sound(f->type);
	} else {
		opponent->actionState = FigureActionState_149_Corpse;
		opponent->waitTicks = 0;
		figure_play_die_sound(opponent->type);
		Formation_updateAfterDeath(opponent->formationId);
	}
}

void FigureAction_Common_handleAttack(figure *f)
{
	if (f->progressOnTile <= 5) {
		f->progressOnTile++;
		FigureMovement_advanceTick(f);
	}
	if (f->numAttackers == 0) {
		resumeActivityAfterAttack(f);
		return;
	}
	if (f->numAttackers == 1) {
		figure *target = figure_get(f->opponentId);
		if (figure_is_dead(target)) {
			resumeActivityAfterAttack(f);
			return;
		}
	} else if (f->numAttackers == 2) {
		if (figure_is_dead(figure_get(f->opponentId))) {
			if (f->opponentId == f->attackerId1) {
				f->opponentId = f->attackerId2;
			} else if (f->opponentId == f->attackerId2) {
				f->opponentId = f->attackerId1;
			}
			if (figure_is_dead(figure_get(f->opponentId))) {
				resumeActivityAfterAttack(f);
				return;
			}
			f->numAttackers = 1;
			f->attackerId1 = f->opponentId;
			f->attackerId2 = 0;
		}
	}
	f->attackGraphicOffset++;
	if (f->attackGraphicOffset >= 24) {
		hitOpponent(f);
	}
}

void FigureAction_Common_setCrossCountryDestination(figure *f, int xDst, int yDst)
{
	f->destinationX = xDst;
	f->destinationY = yDst;
	FigureMovement_crossCountrySetDirection(
		f, f->crossCountryX, f->crossCountryY,
		15 * xDst, 15 * yDst, 0);
}

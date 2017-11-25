#include "figureaction_private.h"

#include "figure.h"
#include "formation.h"

#include <sound>
#include <data>

#include "figure/formation.h"
#include "figure/properties.h"
#include "game/difficulty.h"

const int figureActionCorpseGraphicOffsets[128] =
{
    0, 1, 2, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
};

const int figureActionMissileLauncherGraphicOffsets[128] =
{
    0, 1, 2, 3, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const int figureActionFormationLayoutPositionX[13][16] =
{
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
const int figureActionFormationLayoutPositionY[13][16] =
{
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

void FigureAction_Common_handleCorpse(int figureId)
{
    if (Data_Figures[figureId].waitTicks < 0)
    {
        Data_Figures[figureId].waitTicks = 0;
    }
    Data_Figures[figureId].waitTicks++;
    if (Data_Figures[figureId].waitTicks >= 128)
    {
        Data_Figures[figureId].waitTicks = 127;
        Data_Figures[figureId].state = FigureState_Dead;
    }
}

static int attackIsSameDirection(int dir1, int dir2)
{
    if (dir1 == dir2)
    {
        return 1;
    }
    int dir2Off = dir2 <= 0 ? 7 : dir2 - 1;
    if (dir1 == dir2Off)
    {
        return 1;
    }
    dir2Off = dir2 >= 7 ? 0 : dir2 + 1;
    if (dir1 == dir2Off)
    {
        return 1;
    }
    return 0;
}

static void resumeActivityAfterAttack(int figureId, struct Data_Figure *f)
{
    f->numAttackers = 0;
    f->actionState = f->actionStateBeforeAttack;
    f->opponentId = 0;
    f->attackerId1 = 0;
    f->attackerId2 = 0;
    FigureRoute_remove(figureId);
}

static void hitOpponent(int figureId, struct Data_Figure *f)
{
    const formation *m = formation_get(f->formationId);
    struct Data_Figure *opponent = &Data_Figures[f->opponentId];
    const formation *opponentFormation = formation_get(opponent->formationId);

    const figure_properties *props = figure_properties_for_type(f->type);
    const figure_properties *opponent_props = figure_properties_for_type(opponent->type);
    int cat = opponent_props->category;
    if (cat == FIGURE_CATEGORY_CITIZEN || cat == FIGURE_CATEGORY_CRIMINAL)
    {
        f->attackGraphicOffset = 12;
    }
    else
    {
        f->attackGraphicOffset = 0;
    }
    int figureAttack = props->attack_value;
    int opponentDefense = opponent_props->defense_value;

    // attack modifiers
    if (f->type == FIGURE_WOLF)
    {
        figureAttack = difficulty_adjust_wolf_attack(figureAttack);
    }
    if (opponent->opponentId != figureId && m->figure_type != FIGURE_FORT_LEGIONARY &&
            attackIsSameDirection(f->attackDirection, opponent->attackDirection))
    {
        figureAttack += 4; // attack opponent on the (exposed) back
        sound_effect_play(SOUND_EFFECT_SWORD_SWING);
    }
    if (m->is_halted && m->figure_type == FIGURE_FORT_LEGIONARY &&
            attackIsSameDirection(f->attackDirection, m->direction))
    {
        figureAttack += 4; // coordinated formation attack bonus
    }
    // defense modifiers
    if (opponentFormation->is_halted &&
            (opponentFormation->figure_type == FIGURE_FORT_LEGIONARY ||
             opponentFormation->figure_type == FIGURE_ENEMY_CAESAR_LEGIONARY))
    {
        if (!attackIsSameDirection(opponent->attackDirection, opponentFormation->direction))
        {
            opponentDefense -= 4; // opponent not attacking in coordinated formation
        }
        else if (opponentFormation->layout == FORMATION_COLUMN)
        {
            opponentDefense += 7;
        }
        else if (opponentFormation->layout == FORMATION_DOUBLE_LINE_1 ||
                 opponentFormation->layout == FORMATION_DOUBLE_LINE_2)
        {
            opponentDefense += 4;
        }
    }

    int maxDamage = opponent_props->max_damage;
    int netAttack = figureAttack - opponentDefense;
    if (netAttack < 0)
    {
        netAttack = 0;
    }
    opponent->damage += netAttack;
    if (opponent->damage <= maxDamage)
    {
        Figure_playHitSound(f->type);
    }
    else
    {
        opponent->actionState = FigureActionState_149_Corpse;
        opponent->waitTicks = 0;
        Figure_playDieSound(opponent->type);
        Formation_updateAfterDeath(opponent->formationId);
    }
}

void FigureAction_Common_handleAttack(int figureId)
{
    struct Data_Figure *f = &Data_Figures[figureId];

    if (f->progressOnTile <= 5)
    {
        f->progressOnTile++;
        FigureMovement_advanceTick(f);
    }
    if (f->numAttackers == 0)
    {
        resumeActivityAfterAttack(figureId, f);
        return;
    }
    if (f->numAttackers == 1)
    {
        int targetId = f->opponentId;
        if (FigureIsDead(targetId))
        {
            resumeActivityAfterAttack(figureId, f);
            return;
        }
    }
    else if (f->numAttackers == 2)
    {
        int targetId = f->opponentId;
        if (FigureIsDead(targetId))
        {
            if (targetId == f->attackerId1)
            {
                f->opponentId = f->attackerId2;
            }
            else if (targetId == f->attackerId2)
            {
                f->opponentId = f->attackerId1;
            }
            targetId = f->opponentId;
            if (FigureIsDead(targetId))
            {
                resumeActivityAfterAttack(figureId, f);
                return;
            }
            f->numAttackers = 1;
            f->attackerId1 = targetId;
            f->attackerId2 = 0;
        }
    }
    f->attackGraphicOffset++;
    if (f->attackGraphicOffset >= 24)
    {
        hitOpponent(figureId, f);
    }
}

void FigureAction_Common_setCartOffset(int figureId, int direction)
{
    Data_Figures[figureId].xOffsetCart = cartOffsetsX[direction];
    Data_Figures[figureId].yOffsetCart = cartOffsetsY[direction];
}

void FigureAction_Common_setCrossCountryDestination(int figureId, struct Data_Figure *f, int xDst, int yDst)
{
    f->destinationX = xDst;
    f->destinationY = yDst;
    FigureMovement_crossCountrySetDirection(
        figureId, f->crossCountryX, f->crossCountryY,
        15 * xDst, 15 * yDst, 0);
}

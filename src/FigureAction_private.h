#ifndef FIGUREACTION_PRIVATE_H
#define FIGUREACTION_PRIVATE_H

#define FigureActionIncreaseGraphicOffset(f, max) (f)->graphicOffset++; if ((f)->graphicOffset >= (max)) (f)->graphicOffset = 0;
#define WalkerActionDirection(f) ((8 + f->direction - Data_Settings_Map.orientation) % 8)
#define WalkerActionNormalizeDirection(d) ((d) = (8 + (d) - Data_Settings_Map.orientation) % 8)
#define WalkerActionCorpseGraphicOffset(f) walkerActionCorpseGraphicOffsets[f->waitTicks / 2]
#define WalkerActionMissileLauncherGraphicOffset(f) walkerActionMissileLauncherGraphicOffsets[f->attackGraphicOffset / 2]
#define WalkerActionFormationLayoutPositionX(layout, index) walkerActionFormationLayoutPositionX[layout][index]
#define WalkerActionFormationLayoutPositionY(layout, index) walkerActionFormationLayoutPositionY[layout][index]

#define WalkerActionUpdateGraphic(f,g) \
	if ((f)->actionState == FigureActionState_149_Corpse) {\
		(f)->graphicId = (g) + walkerActionCorpseGraphicOffsets[f->waitTicks / 2] + 96;\
	} else {\
		(f)->graphicId = (g) + WalkerActionDirection(f) + 8 * (f)->graphicOffset;\
	}

extern const int walkerActionCorpseGraphicOffsets[128];
extern const int walkerActionMissileLauncherGraphicOffsets[128];
extern const int walkerActionFormationLayoutPositionX[13][16];
extern const int walkerActionFormationLayoutPositionY[13][16];

#include "FigureMovement.h"
#include "WalkerAction.h"

#include "Data/Building.h"
#include "Data/Constants.h"
#include "Data/Graphics.h"
#include "Data/Settings.h"
#include "Data/Figure.h"

#endif

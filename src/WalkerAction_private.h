#ifndef WALKERACTION_PRIVATE_H
#define WALKERACTION_PRIVATE_H

#define WalkerActionIncreaseGraphicOffset(w, max) (w)->graphicOffset++; if ((w)->graphicOffset >= (max)) (w)->graphicOffset = 0;
#define WalkerActionDirection(w) ((8 + w->direction - Data_Settings_Map.orientation) % 8)
#define WalkerActionNormalizeDirection(d) ((d) = (8 + (d) - Data_Settings_Map.orientation) % 8)
#define WalkerActionCorpseGraphicOffset(w) walkerActionCorpseGraphicOffsets[w->waitTicks / 2]
#define WalkerActionMissileLauncherGraphicOffset(w) walkerActionMissileLauncherGraphicOffsets[w->attackGraphicOffset / 2]
#define WalkerActionFormationLayoutPositionX(layout, index) walkerActionFormationLayoutPositionX[layout][index]
#define WalkerActionFormationLayoutPositionY(layout, index) walkerActionFormationLayoutPositionY[layout][index]

#define WalkerActionUpdateGraphic(w,g) \
	if ((w)->actionState == FigureActionState_149_Corpse) {\
		(w)->graphicId = (g) + walkerActionCorpseGraphicOffsets[w->waitTicks / 2] + 96;\
	} else {\
		(w)->graphicId = (g) + WalkerActionDirection(w) + 8 * (w)->graphicOffset;\
	}

extern const int walkerActionCorpseGraphicOffsets[128];
extern const int walkerActionMissileLauncherGraphicOffsets[128];
extern const int walkerActionFormationLayoutPositionX[13][16];
extern const int walkerActionFormationLayoutPositionY[13][16];

#include "WalkerAction.h"
#include "WalkerMovement.h"

#include "Data/Building.h"
#include "Data/Constants.h"
#include "Data/Graphics.h"
#include "Data/Settings.h"
#include "Data/Figure.h"

#endif

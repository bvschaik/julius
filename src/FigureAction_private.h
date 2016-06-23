#ifndef FIGUREACTION_PRIVATE_H
#define FIGUREACTION_PRIVATE_H

#define FigureActionIncreaseGraphicOffset(f, max) (f)->graphicOffset++; if ((f)->graphicOffset >= (max)) (f)->graphicOffset = 0;
#define FigureActionDirection(f) ((8 + f->direction - Data_Settings_Map.orientation) % 8)
#define FigureActionNormalizeDirection(d) ((d) = (8 + (d) - Data_Settings_Map.orientation) % 8)
#define FigureActionCorpseGraphicOffset(f) figureActionCorpseGraphicOffsets[f->waitTicks / 2]
#define FigureActionMissileLauncherGraphicOffset(f) figureActionMissileLauncherGraphicOffsets[f->attackGraphicOffset / 2]
#define FigureActionFormationLayoutPositionX(layout, index) figureActionFormationLayoutPositionX[layout][index]
#define FigureActionFormationLayoutPositionY(layout, index) figureActionFormationLayoutPositionY[layout][index]

#define FigureActionUpdateGraphic(f,g) \
	if ((f)->actionState == FigureActionState_149_Corpse) {\
		(f)->graphicId = (g) + figureActionCorpseGraphicOffsets[f->waitTicks / 2] + 96;\
	} else {\
		(f)->graphicId = (g) + FigureActionDirection(f) + 8 * (f)->graphicOffset;\
	}

extern const int figureActionCorpseGraphicOffsets[128];
extern const int figureActionMissileLauncherGraphicOffsets[128];
extern const int figureActionFormationLayoutPositionX[13][16];
extern const int figureActionFormationLayoutPositionY[13][16];

#include "FigureAction.h"
#include "FigureMovement.h"

#include "Data/Building.h"
#include "Data/Constants.h"
#include "Data/Graphics.h"
#include "Data/Settings.h"
#include "Data/Figure.h"

#endif

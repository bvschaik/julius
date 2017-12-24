#ifndef FIGUREACTION_PRIVATE_H
#define FIGUREACTION_PRIVATE_H

#include "figure/image.h"

#define FigureActionIncreaseGraphicOffset(f, max) (f)->graphicOffset++; if ((f)->graphicOffset >= (max)) (f)->graphicOffset = 0;
#define FigureActionDirection(f) ((f->direction < Data_State.map.orientation ? 8 : 0) + f->direction - Data_State.map.orientation)
#define FigureActionNormalizeDirection(d) ((d) = (8 + (d) - Data_State.map.orientation) % 8)
#define FigureActionCorpseGraphicOffset(f) figure_image_corpse_offset(f)
#define FigureActionMissileLauncherGraphicOffset(f) figureActionMissileLauncherGraphicOffsets[f->attackGraphicOffset / 2]
#define FigureActionFormationLayoutPositionX(layout, index) figureActionFormationLayoutPositionX[layout][index]
#define FigureActionFormationLayoutPositionY(layout, index) figureActionFormationLayoutPositionY[layout][index]

#define FigureActionUpdateGraphic(f,g) figure_image_update(f, g)

extern const int figureActionMissileLauncherGraphicOffsets[128];
extern const int figureActionFormationLayoutPositionX[13][16];
extern const int figureActionFormationLayoutPositionY[13][16];

#include "FigureAction.h"
#include "FigureMovement.h"

#include "Data/State.h"

#include "graphics/image.h"

#endif

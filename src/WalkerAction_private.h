#ifndef WALKERACTION_PRIVATE_H
#define WALKERACTION_PRIVATE_H

#define WalkerActionIncreaseGraphicOffset(w, max) (w)->graphicOffset++; if ((w)->graphicOffset >= (max)) (w)->graphicOffset = 0;
#define WalkerActionDirection(w) ((8 + w->direction - Data_Settings_Map.orientation) % 8)
#define WalkerActionCorpseGraphicOffset(w) walkerActionCorpseGraphicOffsets[w->waitTicks / 2]

#define WalkerActionUpdateGraphic(w,g) \
	if ((w)->actionState == WalkerActionState_149_Corpse) {\
		(w)->graphicId = (g) + walkerActionCorpseGraphicOffsets[w->waitTicks / 2] + 96;\
	} else {\
		(w)->graphicId = (g) + WalkerActionDirection(w) + 8 * (w)->graphicOffset;\
	}

extern const int walkerActionCorpseGraphicOffsets[128];

#include "WalkerAction.h"
#include "WalkerMovement.h"

#include "Data/Building.h"
#include "Data/Constants.h"
#include "Data/Graphics.h"
#include "Data/Settings.h"
#include "Data/Walker.h"

#endif

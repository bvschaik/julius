#include "WalkerAction_private.h"

static const int seagullOffsetsX[] = {0, 0, -2, 1, 2, -3, 4, -1};
static const int seagullOffsetsY[] = {0, -2, 0, 2, 0, 1, -3, 4};

void WalkerAction_seagulls(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->terrainUsage = 0;
	w->isGhost = 0;
	w->useCrossCountry = 1;
	if (!(w->graphicOffset & 3) && WalkerMovement_crossCountryWalkTicks(walkerId, 1)) {
		w->progressOnTile++;
		if (w->progressOnTile > 8) {
			w->progressOnTile = 0;
		}
		WalkerAction_Common_setCrossCountryDestination(walkerId, w,
			seagullOffsetsX[(int)w->progressOnTile], seagullOffsetsY[(int)w->progressOnTile]);
	}
	if (walkerId & 1) {
		WalkerActionIncreaseGraphicOffset(w, 54);
		w->graphicId = GraphicId(ID_Graphic_Walker_Seagulls) + w->graphicOffset / 3;
	} else {
		WalkerActionIncreaseGraphicOffset(w, 72);
		w->graphicId = GraphicId(ID_Graphic_Walker_Seagulls) + 18 + w->graphicOffset / 3;
	}
}

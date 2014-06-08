#include "WalkerAction_private.h"

static const int criminalOffsets[] =
	{0, 0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1};

void WalkerAction_protestor(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->terrainUsage = 1;
	WalkerActionIncreaseGraphicOffset(w, 64);
	w->cartGraphicId = 0;
	if (w->actionState == WalkerActionState_149_Corpse) {
		w->state = WalkerState_Dead;
	}
	w->waitTicks++;
	if (w->waitTicks > 200) {
		w->state = WalkerState_Dead;
		w->graphicOffset = 0;
	}
	if (w->actionState == WalkerActionState_149_Corpse) {
		w->graphicId = GraphicId(ID_Graphic_Walker_Criminal) +
			WalkerActionCorpseGraphicOffset(w) + 96;
	} else {
		w->graphicId = GraphicId(ID_Graphic_Walker_Criminal) +
			criminalOffsets[w->graphicOffset / 4] + 104;
	}
}

void WalkerAction_criminal(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->terrainUsage = 1;
	WalkerActionIncreaseGraphicOffset(w, 32);
	w->cartGraphicId = 0;
	if (w->actionState == WalkerActionState_149_Corpse) {
		w->state = WalkerState_Dead;
	}
	w->waitTicks++;
	if (w->waitTicks > 200) {
		w->state = WalkerState_Dead;
		w->graphicOffset = 0;
	}
	if (w->actionState == WalkerActionState_149_Corpse) {
		w->graphicId = GraphicId(ID_Graphic_Walker_Criminal) +
			WalkerActionCorpseGraphicOffset(w) + 96;
	} else {
		w->graphicId = GraphicId(ID_Graphic_Walker_Criminal) +
			criminalOffsets[w->graphicOffset / 2] + 104;
	}
}

int WalkerAction_rioterCollapseBuilding(int walkerId)
{
	// TODO
	return 0;
}

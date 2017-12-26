#include "Figure.h"

#include "figure/figure.h"

int Figure_hasNearbyEnemy(int xStart, int yStart, int xEnd, int yEnd)
{
	for (int i = 1; i < MAX_FIGURES; i++) {
		figure *f = figure_get(i);
		if (f->state != FigureState_Alive || !FigureIsEnemy(f->type)) {
			continue;
		}
		int dx = (f->x > xStart) ? (f->x - xStart) : (xStart - f->x);
		int dy = (f->y > yStart) ? (f->y - yStart) : (yStart - f->y);
		if (dx <= 12 && dy <= 12) {
			return 1;
		}
		dx = (f->x > xEnd) ? (f->x - xEnd) : (xEnd - f->x);
		dy = (f->y > yEnd) ? (f->y - yEnd) : (yEnd - f->y);
		if (dx <= 12 && dy <= 12) {
			return 1;
		}
	}
	return 0;
}

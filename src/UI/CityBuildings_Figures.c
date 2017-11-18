#include "CityBuildings.h"
#include "CityBuildings_private.h"

#include "../Graphics.h"

#include "../Data/Building.h"
#include "../Data/Figure.h"
#include "../Data/Settings.h"
#include "../Data/State.h"

#include "figure/formation.h"
#include "graphics/image.h"

static int showOnOverlay(struct Data_Figure *f)
{
	switch (Data_State.currentOverlay) {
		case Overlay_Water:
		case Overlay_Desirability:
			return 0;
		case Overlay_Native:
			return f->type == FIGURE_INDIGENOUS_NATIVE || f->type == FIGURE_MISSIONARY;
		case Overlay_Fire:
			return f->type == FIGURE_PREFECT;
		case Overlay_Damage:
			return f->type == FIGURE_ENGINEER;
		case Overlay_TaxIncome:
			return f->type == FIGURE_TAX_COLLECTOR;
		case Overlay_Crime:
			return f->type == FIGURE_PREFECT || f->type == FIGURE_PROTESTER ||
				f->type == FIGURE_CRIMINAL || f->type == FIGURE_RIOTER;
		case Overlay_Entertainment:
			return f->type == FIGURE_ACTOR || f->type == FIGURE_GLADIATOR ||
				f->type == FIGURE_LION_TAMER || f->type == FIGURE_CHARIOTEER;
		case Overlay_Education:
			return f->type == FIGURE_SCHOOL_CHILD || f->type == FIGURE_LIBRARIAN ||
				f->type == FIGURE_TEACHER;
		case Overlay_Theater:
			if (f->type == FIGURE_ACTOR) {
				if (f->actionState == FigureActionState_94_EntertainerRoaming ||
					f->actionState == FigureActionState_95_EntertainerReturning) {
					return Data_Buildings[f->buildingId].type == BUILDING_THEATER;
				} else {
					return Data_Buildings[f->destinationBuildingId].type == BUILDING_THEATER;
				}
			}
			return 0;
		case Overlay_Amphitheater:
			if (f->type == FIGURE_ACTOR || f->type == FIGURE_GLADIATOR) {
				if (f->actionState == FigureActionState_94_EntertainerRoaming ||
					f->actionState == FigureActionState_95_EntertainerReturning) {
					return Data_Buildings[f->buildingId].type == BUILDING_AMPHITHEATER;
				} else {
					return Data_Buildings[f->destinationBuildingId].type == BUILDING_AMPHITHEATER;
				}
			}
			return 0;
		case Overlay_Colosseum:
			if (f->type == FIGURE_GLADIATOR) {
				if (f->actionState == FigureActionState_94_EntertainerRoaming ||
					f->actionState == FigureActionState_95_EntertainerReturning) {
					return Data_Buildings[f->buildingId].type == BUILDING_COLOSSEUM;
				} else {
					return Data_Buildings[f->destinationBuildingId].type == BUILDING_COLOSSEUM;
				}
			} else if (f->type == FIGURE_LION_TAMER) {
				return 1;
			}
			return 0;
		case Overlay_Hippodrome:
			return f->type == FIGURE_CHARIOTEER;
		case Overlay_Religion:
			return f->type == FIGURE_PRIEST;
		case Overlay_School:
			return f->type == FIGURE_SCHOOL_CHILD;
		case Overlay_Library:
			return f->type == FIGURE_LIBRARIAN;
		case Overlay_Academy:
			return f->type == FIGURE_TEACHER;
		case Overlay_Barber:
			return f->type == FIGURE_BARBER;
		case Overlay_Bathhouse:
			return f->type == FIGURE_BATHHOUSE_WORKER;
		case Overlay_Clinic:
			return f->type == FIGURE_DOCTOR;
		case Overlay_Hospital:
			return f->type == FIGURE_SURGEON;
		case Overlay_FoodStocks:
			if (f->type == FIGURE_MARKET_BUYER || f->type == FIGURE_MARKET_TRADER ||
				f->type == FIGURE_DELIVERY_BOY || f->type == FIGURE_FISHING_BOAT) {
				return 1;
			} else if (f->type == FIGURE_CART_PUSHER) {
				return f->resourceId == Resource_Wheat || f->resourceId == Resource_Vegetables ||
					f->resourceId == Resource_Fruit || f->resourceId == Resource_Meat;
			}
			return 0;
		case Overlay_Problems:
			if (f->type == FIGURE_LABOR_SEEKER) {
				return Data_Buildings[f->buildingId].showOnProblemOverlay;
			} else if (f->type == FIGURE_CART_PUSHER) {
				return f->actionState == FigureActionState_20_CartpusherInitial || f->minMaxSeen;
			}
			return 0;
	}
	return 1;
}

static void drawFigureWithCart(struct Data_Figure *f, int xOffset, int yOffset)
{
	if (f->yOffsetCart >= 0) {
		Graphics_drawImage(f->graphicId, xOffset, yOffset);
		Graphics_drawImage(f->cartGraphicId, xOffset + f->xOffsetCart, yOffset + f->yOffsetCart);
	} else {
		Graphics_drawImage(f->cartGraphicId, xOffset + f->xOffsetCart, yOffset + f->yOffsetCart);
		Graphics_drawImage(f->graphicId, xOffset, yOffset);
	}
}

static void drawHippodromeHorses(struct Data_Figure *f, int xOffset, int yOffset)
{
	int val = f->waitTicksMissile;
	switch (Data_Settings_Map.orientation) {
		case Dir_0_Top:
			xOffset += 10;
			if (val <= 10) {
				yOffset -= 2;
			} else if (val <= 11) {
				yOffset -= 10;
			} else if (val <= 12) {
				yOffset -= 18;
			} else if (val <= 13) {
				yOffset -= 16;
			} else if (val <= 20) {
				yOffset -= 14;
			} else if (val <= 21) {
				yOffset -= 10;
			} else {
				yOffset -= 2;
			}
			break;
		case Dir_2_Right:
			xOffset -= 10;
			if (val <= 9) {
				yOffset -= 12;
			} else if (val <= 10) {
				yOffset += 4;
			} else if (val <= 11) {
				xOffset -= 5;
				yOffset += 2;
			} else if (val <= 13) {
				xOffset -= 5;
			} else if (val <= 20) {
				yOffset -= 2;
			} else if (val <= 21) {
				yOffset -= 6;
			} else {
				yOffset -= 12;
			}
		case Dir_4_Bottom:
			xOffset += 20;
			if (val <= 9) {
				yOffset += 4;
			} else if (val <= 10) {
				xOffset += 10;
				yOffset += 4;
			} else if (val <= 11) {
				xOffset += 10;
				yOffset -= 4;
			} else if (val <= 13) {
				yOffset -= 6;
			} else if (val <= 20) {
				yOffset -= 12;
			} else if (val <= 21) {
				yOffset -= 10;
			} else {
				yOffset -= 2;
			}
			break;
		case Dir_6_Left:
			xOffset -= 10;
			if (val <= 9) {
				yOffset -= 12;
			} else if (val <= 10) {
				yOffset += 4;
			} else if (val <= 11) {
				yOffset += 2;
			} else if (val <= 13) {
				// no change
			} else if (val <= 20) {
				yOffset -= 2;
			} else if (val <= 21) {
				yOffset -= 6;
			} else {
				yOffset -= 12;
			}
			break;
	}
	drawFigureWithCart(f, xOffset, yOffset);
}

static int tileOffsetToPixelOffsetX(int x, int y)
{
	int dir = Data_Settings_Map.orientation;
	if (dir == Dir_0_Top || dir == Dir_4_Bottom) {
		int base = 2 * x - 2 * y;
		return dir == Dir_0_Top ? base : -base;
	} else {
		int base = 2 * x + 2 * y;
		return dir == Dir_2_Right ? base : -base;
	}
}

static int tileOffsetToPixelOffsetY(int x, int y)
{
	int dir = Data_Settings_Map.orientation;
	if (dir == Dir_0_Top || dir == Dir_4_Bottom) {
		int base = x + y;
		return dir == Dir_0_Top ? base : -base;
	} else {
		int base = x - y;
		return dir == Dir_6_Left ? base : -base;
	}
}

static int tileProgressToPixelOffsetX(int direction, int progress)
{
	int offset = 0;
	if (direction == 0 || direction == 2) {
		offset = 2 * progress - 28;
	} else if (direction == 1) {
		offset = 4 * progress - 56;
	} else if (direction == 3 || direction == 7) {
		offset = 0;
	} else if (direction == 4 || direction == 6) {
		offset = 28 - 2 * progress;
	} else if (direction == 5) {
		offset = 56 - 4 * progress;
	}
	return offset;
}

static int tileProgressToPixelOffsetY(int direction, int progress)
{
	int offset = 0;
	if (direction == 0 || direction == 6) {
		offset = 14 - progress;
	} else if (direction == 1 || direction == 5) {
		offset = 0;
	} else if (direction == 2 || direction == 4) {
		offset = progress - 14;
	} else if (direction == 3) {
		offset = 2 * progress - 28;
	} else if (direction == 7) {
		offset = 28 - 2 * progress;
	}
	return offset;
}

void UI_CityBuildings_drawFigure(int figureId, int xOffset, int yOffset, int selectedFigureId, struct UI_CityPixelCoordinate *coord)
{
	struct Data_Figure *f = &Data_Figures[figureId];

	// determining x/y offset on tile
	int xTileOffset = 0;
	int yTileOffset = 0;
	if (f->useCrossCountry) {
		xTileOffset = tileOffsetToPixelOffsetX(f->crossCountryX % 15, f->crossCountryY % 15);
		yTileOffset = tileOffsetToPixelOffsetY(f->crossCountryX % 15, f->crossCountryY % 15);
		yTileOffset -= f->missileDamage;
	} else {
		int direction = (8 + f->direction - Data_Settings_Map.orientation) % 8;
		xTileOffset = tileProgressToPixelOffsetX(direction, f->progressOnTile);
		yTileOffset = tileProgressToPixelOffsetY(direction, f->progressOnTile);
		yTileOffset -= f->currentHeight;
		if (f->numPreviousFiguresOnSameTile && f->type != FIGURE_BALLISTA) {
			static const int xOffsets[] = {
				0, 8, 8, -8, -8, 0, 16, 0, -16, 8, -8, 16, -16, 16, -16, 8, -8, 0, 24, 0, -24, 0, 0, 0
			};
			static const int yOffsets[] = {
				0, 0, 8, 8, -8, -16, 0, 16, 0, -16, 16, 8, -8, -8, 8, 16, -16, -24, 0, 24, 0, 0, 0, 0
			};
			xTileOffset += xOffsets[f->numPreviousFiguresOnSameTile];
			yTileOffset += yOffsets[f->numPreviousFiguresOnSameTile];
		}
	}

	xTileOffset += 29;
	yTileOffset += 15;

	const image *img = f->isEnemyGraphic ? image_get_enemy(f->graphicId) : image_get(f->graphicId);
	xOffset += xTileOffset - img->sprite_offset_x;
	yOffset += yTileOffset - img->sprite_offset_y;

	// excluding figures
	if (selectedFigureId == 9999) {
		if (!showOnOverlay(f)) {
			return;
		}
	} else if (selectedFigureId) {
		if (figureId != selectedFigureId) {
			return;
		}
		if (coord) {
			coord->x = xOffset;
			coord->y = yOffset;
		}
	}

	// actual drawing
	if (f->cartGraphicId) {
		switch (f->type) {
			case FIGURE_CART_PUSHER:
			case FIGURE_WAREHOUSEMAN:
			case FIGURE_LION_TAMER:
			case FIGURE_DOCKMAN:
			case FIGURE_NATIVE_TRADER:
			case FIGURE_IMMIGRANT:
			case FIGURE_EMIGRANT:
				drawFigureWithCart(f, xOffset, yOffset);
				break;
			case FIGURE_HIPPODROME_HORSES:
				drawHippodromeHorses(f, xOffset, yOffset);
				break;
			case FIGURE_FORT_STANDARD:
				if (!formation_get(f->formationId)->in_distant_battle) {
					// base
					Graphics_drawImage(f->graphicId, xOffset, yOffset);
					// flag
					Graphics_drawImage(f->cartGraphicId,
						xOffset, yOffset - image_get(f->cartGraphicId)->height);
					// top icon
					int iconGraphicId = image_group(ID_Graphic_FortStandardIcons) + f->formationId - 1;
					Graphics_drawImage(iconGraphicId,
						xOffset, yOffset - image_get(iconGraphicId)->height - image_get(f->cartGraphicId)->height);
				}
				break;
			default:
				Graphics_drawImage(f->graphicId, xOffset, yOffset);
				break;
		}
	} else {
		if (f->isEnemyGraphic) {
			Graphics_drawEnemyImage(f->graphicId, xOffset, yOffset);
		} else {
			Graphics_drawImage(f->graphicId, xOffset, yOffset);
		}
	}
}

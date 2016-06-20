#include "CityBuildings.h"
#include "CityBuildings_private.h"

#include "../Graphics.h"

#include "../Data/Building.h"
#include "../Data/Constants.h"
#include "../Data/Figure.h"
#include "../Data/Formation.h"
#include "../Data/Settings.h"
#include "../Data/State.h"

static int showOnOverlay(struct Data_Walker *f)
{
	switch (Data_State.currentOverlay) {
		case Overlay_Water:
		case Overlay_Desirability:
			return 0;
		case Overlay_Native:
			return f->type == Figure_IndigenousNative || f->type == Figure_Missionary;
		case Overlay_Fire:
			return f->type == Figure_Prefect;
		case Overlay_Damage:
			return f->type == Figure_Engineer;
		case Overlay_TaxIncome:
			return f->type == Figure_TaxCollector;
		case Overlay_Crime:
			return f->type == Figure_Prefect || f->type == Figure_Protester ||
				f->type == Figure_Criminal || f->type == Figure_Rioter;
		case Overlay_Entertainment:
			return f->type == Figure_Actor || f->type == Figure_Gladiator ||
				f->type == Figure_LionTamer || f->type == Figure_Charioteer;
		case Overlay_Education:
			return f->type == Figure_SchoolChild || f->type == Figure_Librarian ||
				f->type == Figure_Teacher;
		case Overlay_Theater:
			if (f->type == Figure_Actor) {
				if (f->actionState == FigureActionState_94_EntertainerRoaming ||
					f->actionState == FigureActionState_95_EntertainerReturning) {
					return Data_Buildings[f->buildingId].type == Building_Theater;
				} else {
					return Data_Buildings[f->destinationBuildingId].type == Building_Theater;
				}
			}
			return 0;
		case Overlay_Amphitheater:
			if (f->type == Figure_Actor || f->type == Figure_Gladiator) {
				if (f->actionState == FigureActionState_94_EntertainerRoaming ||
					f->actionState == FigureActionState_95_EntertainerReturning) {
					return Data_Buildings[f->buildingId].type == Building_Amphitheater;
				} else {
					return Data_Buildings[f->destinationBuildingId].type == Building_Amphitheater;
				}
			}
			return 0;
		case Overlay_Colosseum:
			if (f->type == Figure_Gladiator) {
				if (f->actionState == FigureActionState_94_EntertainerRoaming ||
					f->actionState == FigureActionState_95_EntertainerReturning) {
					return Data_Buildings[f->buildingId].type == Building_Colosseum;
				} else {
					return Data_Buildings[f->destinationBuildingId].type == Building_Colosseum;
				}
			} else if (f->type == Figure_LionTamer) {
				return 1;
			}
			return 0;
		case Overlay_Hippodrome:
			return f->type == Figure_Charioteer;
		case Overlay_Religion:
			return f->type == Figure_Priest;
		case Overlay_School:
			return f->type == Figure_SchoolChild;
		case Overlay_Library:
			return f->type == Figure_Librarian;
		case Overlay_Academy:
			return f->type == Figure_Teacher;
		case Overlay_Barber:
			return f->type == Figure_Barber;
		case Overlay_Bathhouse:
			return f->type == Figure_BathhouseWorker;
		case Overlay_Clinic:
			return f->type == Figure_Doctor;
		case Overlay_Hospital:
			return f->type == Figure_Surgeon;
		case Overlay_FoodStocks:
			if (f->type == Figure_MarketBuyer || f->type == Figure_MarketTrader ||
				f->type == Figure_DeliveryBoy || f->type == Figure_FishingBoat) {
				return 1;
			} else if (f->type == Figure_CartPusher) {
				return f->resourceId == Resource_Wheat || f->resourceId == Resource_Vegetables ||
					f->resourceId == Resource_Fruit || f->resourceId == Resource_Meat;
			}
			return 0;
		case Overlay_Problems:
			if (f->type == Figure_LaborSeeker) {
				return Data_Buildings[f->buildingId].showOnProblemOverlay;
			} else if (f->type == Figure_CartPusher) {
				return f->actionState == FigureActionState_20_CartpusherInitial || f->minMaxSeen;
			}
			return 0;
	}
	return 1;
}

static void drawWalkerWithCart(struct Data_Walker *f, int xOffset, int yOffset)
{
	if (f->yOffsetCart >= 0) {
		Graphics_drawImage(f->graphicId, xOffset, yOffset);
		Graphics_drawImage(f->cartGraphicId, xOffset + f->xOffsetCart, yOffset + f->yOffsetCart);
	} else {
		Graphics_drawImage(f->cartGraphicId, xOffset + f->xOffsetCart, yOffset + f->yOffsetCart);
		Graphics_drawImage(f->graphicId, xOffset, yOffset);
	}
}

static void drawHippodromeHorses(struct Data_Walker *f, int xOffset, int yOffset)
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
	drawWalkerWithCart(f, xOffset, yOffset);
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

void UI_CityBuildings_drawWalker(int walkerId, int xOffset, int yOffset, int selectedWalkerId, struct UI_CityPixelCoordinate *coord)
{
	struct Data_Walker *f = &Data_Walkers[walkerId];

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
		if (f->numPreviousWalkersOnSameTile && f->type != Figure_Ballista) {
			static const int xOffsets[] = {
				0, 8, 8, -8, -8, 0, 16, 0, -16, 8, -8, 16, -16, 16, -16, 8, -8, 0, 24, 0, -24, 0, 0, 0
			};
			static const int yOffsets[] = {
				0, 0, 8, 8, -8, -16, 0, 16, 0, -16, 16, 8, -8, -8, 8, 16, -16, -24, 0, 24, 0, 0, 0, 0
			};
			xTileOffset += xOffsets[f->numPreviousWalkersOnSameTile];
			yTileOffset += yOffsets[f->numPreviousWalkersOnSameTile];
		}
	}

	xTileOffset += 29;
	yTileOffset += 15;
	if (f->isEnemyGraphic) {
		xOffset += xTileOffset - GraphicEnemySpriteOffsetX(f->graphicId);
		yOffset += yTileOffset - GraphicEnemySpriteOffsetY(f->graphicId);
	} else {
		xOffset += xTileOffset - GraphicSpriteOffsetX(f->graphicId);
		yOffset += yTileOffset - GraphicSpriteOffsetY(f->graphicId);
	}

	// excluding walkers
	if (selectedWalkerId == 9999) {
		if (!showOnOverlay(f)) {
			return;
		}
	} else if (selectedWalkerId) {
		if (walkerId != selectedWalkerId) {
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
			case Figure_CartPusher:
			case Figure_Warehouseman:
			case Figure_LionTamer:
			case Figure_Dockman:
			case Figure_NativeTrader:
			case Figure_Immigrant:
			case Figure_Emigrant:
				drawWalkerWithCart(f, xOffset, yOffset);
				break;
			case Figure_HippodromeMiniHorses:
				drawHippodromeHorses(f, xOffset, yOffset);
				break;
			case Figure_FortStandard:
				if (!Data_Formations[f->formationId].inDistantBattle) {
					// base
					Graphics_drawImage(f->graphicId, xOffset, yOffset);
					// flag
					Graphics_drawImage(f->cartGraphicId,
						xOffset, yOffset - GraphicHeight(f->cartGraphicId));
					// top icon
					int iconGraphicId = GraphicId(ID_Graphic_FortStandardIcons) + f->formationId - 1;
					Graphics_drawImage(iconGraphicId,
						xOffset, yOffset - GraphicHeight(iconGraphicId) - GraphicHeight(f->cartGraphicId));
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

#include "../Data/Building.h"
#include "../Data/CityInfo.h"
#include "../Data/CityView.h"
#include "../Data/Constants.h"
#include "../Data/Graphics.h"
#include "../Data/Grid.h"
#include "../Data/Model.h"
#include "../Data/Settings.h"
#include "../Data/State.h"
#include "../Data/Walker.h"
#include "../Animation.h"
#include "../Building.h"
#include "../Calc.h"
#include "../CityView.h"
#include "../Graphics.h"
#include "../Sound.h"
#include "../Time.h"

#include <cstdio>

#define DRAWFOOT_SIZE1(g,x,y) Graphics_drawIsometricFootprint(g, x, y, 0)
#define DRAWFOOT_SIZE2(g,x,y) Graphics_drawIsometricFootprint(g, x + 30, y - 15, 0)
#define DRAWFOOT_SIZE3(g,x,y) Graphics_drawIsometricFootprint(g, x + 60, y - 30, 0)
#define DRAWFOOT_SIZE4(g,x,y) Graphics_drawIsometricFootprint(g, x + 90, y - 45, 0)
#define DRAWFOOT_SIZE5(g,x,y) Graphics_drawIsometricFootprint(g, x + 120, y - 60, 0)

#define DRAWTOP_SIZE1(g,x,y) Graphics_drawIsometricTop(g, x, y, 0)
#define DRAWTOP_SIZE2(g,x,y) Graphics_drawIsometricTop(g, x + 30, y - 15, 0)
#define DRAWTOP_SIZE3(g,x,y) Graphics_drawIsometricTop(g, x + 60, y - 30, 0)
#define DRAWTOP_SIZE4(g,x,y) Graphics_drawIsometricTop(g, x + 90, y - 45, 0)
#define DRAWTOP_SIZE5(g,x,y) Graphics_drawIsometricTop(g, x + 120, y - 60, 0)

#define DRAWTOP_SIZE1_C(g,x,y,c) Graphics_drawIsometricTop(g, x, y, c)
#define DRAWTOP_SIZE2_C(g,x,y,c) Graphics_drawIsometricTop(g, x + 30, y - 15, c)
#define DRAWTOP_SIZE3_C(g,x,y,c) Graphics_drawIsometricTop(g, x + 60, y - 30, c)
#define DRAWTOP_SIZE4_C(g,x,y,c) Graphics_drawIsometricTop(g, x + 90, y - 45, c)
#define DRAWTOP_SIZE5_C(g,x,y,c) Graphics_drawIsometricTop(g, x + 120, y - 60, c)

#define FOREACH_XY_VIEW(block)\
	int odd = 0;\
	int yView = Data_CityView.yInTiles - 8;\
	int yGraphic = Data_CityView.yOffsetInPixels - 9*15;\
	for (int y = 0; y < Data_CityView.heightInTiles + 14; y++) {\
		int xGraphic = -(4*58 + 8);\
		if (odd) {\
			xGraphic += Data_CityView.xOffsetInPixels - 30;\
			odd = 0;\
		} else {\
			xGraphic += Data_CityView.xOffsetInPixels;\
			odd = 1;\
		}\
		int xView = Data_CityView.xInTiles - 4;\
		for (int x = 0; x < Data_CityView.widthInTiles + 7; x++) {\
			if (xView >= 0 && xView < VIEW_X_MAX &&\
				yView >= 0 && yView < VIEW_Y_MAX) {\
				block;\
			}\
			xGraphic += 60;\
			xView++;\
		}\
		yGraphic += 15;\
		yView++;\
	}

#define FOREACH_Y_VIEW(block)\
	int odd = 0;\
	int yView = Data_CityView.yInTiles - 8;\
	int yGraphic = Data_CityView.yOffsetInPixels - 9*15;\
	int xGraphic, xView;\
	for (int y = 0; y < Data_CityView.heightInTiles + 14; y++) {\
		if (yView >= 0 && yView < VIEW_Y_MAX) {\
			block;\
		}\
		yGraphic += 15;\
		yView++;\
	}

#define FOREACH_X_VIEW(block)\
	xGraphic = -(4*58 + 8);\
	if (odd) {\
		xGraphic += Data_CityView.xOffsetInPixels - 30;\
		odd = 0;\
	} else {\
		xGraphic += Data_CityView.xOffsetInPixels;\
		odd = 1;\
	}\
	xView = Data_CityView.xInTiles - 4;\
	for (int x = 0; x < Data_CityView.widthInTiles + 7; x++) {\
		if (xView >= 0 && xView < VIEW_X_MAX) {\
			int gridOffset = ViewToGridOffset(xView, yView);\
			if (gridOffset >= 0) {\
				block;\
			}\
		}\
		xGraphic += 60;\
		xView++;\
	}

static void drawBuildingFootprints();
static void drawBuildingTopsWalkersAnimation(int selectedWalkerId);
static void drawOverlayFootprints();
static void drawFootprintForWaterOverlay(int gridOffset, int xOffset, int yOffset);
static void drawTopForWaterOverlay(int gridOffset, int xOffset, int yOffset);
static void drawFootprintForNativeOverlay(int gridOffset, int xOffset, int yOffset);
static void drawTopForNativeOverlay(int gridOffset, int xOffset, int yOffset);
static void drawBuildingFootprintForOverlay(int buildingId, int gridOffset, int xOffset, int yOffset, int graphicOffset);
static void drawBuildingFootprintForDesirabilityOverlay(int gridOffset, int xOffset, int yOffset);
static void drawBuildingTopForDesirabilityOverlay(int gridOffset, int xOffset, int yOffset);
static void drawBuildingTopForFireOverlay(int gridOffset, int buildingId, int xOffset, int yOffset);
static void drawBuildingTopForDamageOverlay(int gridOffset, int buildingId, int xOffset, int yOffset);
static void drawBuildingTopForCrimeOverlay(int gridOffset, int buildingId, int xOffset, int yOffset);
static void drawBuildingTopForEntertainmentOverlay(int gridOffset, int buildingId, int xOffset, int yOffset);
static void drawBuildingTopForEducationOverlay(int gridOffset, int buildingId, int xOffset, int yOffset);
static void drawBuildingTopForTheaterOverlay(int gridOffset, int buildingId, int xOffset, int yOffset);
static void drawBuildingTopForAmphitheaterOverlay(int gridOffset, int buildingId, int xOffset, int yOffset);
static void drawBuildingTopForColosseumOverlay(int gridOffset, int buildingId, int xOffset, int yOffset);
static void drawBuildingTopForHippodromeOverlay(int gridOffset, int buildingId, int xOffset, int yOffset);
static void drawBuildingTopForFoodStocksOverlay(int gridOffset, int buildingId, int xOffset, int yOffset);
static void drawBuildingTopForBathhouseOverlay(int gridOffset, int buildingId, int xOffset, int yOffset);
static void drawBuildingTopForReligionOverlay(int gridOffset, int buildingId, int xOffset, int yOffset);
static void drawBuildingTopForSchoolOverlay(int gridOffset, int buildingId, int xOffset, int yOffset);
static void drawBuildingTopForLibraryOverlay(int gridOffset, int buildingId, int xOffset, int yOffset);
static void drawBuildingTopForAcademyOverlay(int gridOffset, int buildingId, int xOffset, int yOffset);
static void drawBuildingTopForBarberOverlay(int gridOffset, int buildingId, int xOffset, int yOffset);
static void drawBuildingTopForClinicsOverlay(int gridOffset, int buildingId, int xOffset, int yOffset);
static void drawBuildingTopForHospitalOverlay(int gridOffset, int buildingId, int xOffset, int yOffset);
static void drawBuildingTopForTaxIncomeOverlay(int gridOffset, int buildingId, int xOffset, int yOffset);
static void drawOverlayColumn(int height, int xOffset, int yOffset, int isRed);

static void drawWalker(int walkerId, int xOffset, int yOffset, int selectedWalkerId);
static void drawBridge(int gridOffset, int xOffset, int yOffset);

static int lastAnimationTime = 0;
static int advanceAnimation;

void UI_CityBuildings_drawForeground(int x, int y)
{
	//printf("Drawing city at %d %d\n", x, y);
	Data_CityView.xInTiles = x;
	Data_CityView.yInTiles = y;
	Graphics_setClipRectangle(
		Data_CityView.xOffsetInPixels, Data_CityView.yOffsetInPixels,
		Data_CityView.widthInPixels, Data_CityView.heightInPixels);
	/*printf("Cliprectangle set to %d, %d, %d, %d\n",
		Data_CityView.xOffsetInPixels, Data_CityView.yOffsetInPixels,
		Data_CityView.widthInPixels, Data_CityView.heightInPixels);*/

	advanceAnimation = 0;
	int now = Time_getMillis();
	if (now - lastAnimationTime > 60 || now - lastAnimationTime < 0) {
		lastAnimationTime = now;
		advanceAnimation = 1;
	}

	// TODO overlay and stuff
	//Data_State.currentOverlay = Overlay_Fire;
	if (Data_State.currentOverlay) {
		drawOverlayFootprints();
	} else {
		drawBuildingFootprints();
		drawBuildingTopsWalkersAnimation(0);
	}

	Graphics_resetClipRectangle();
}

static void drawBuildingFootprints()
{
	int graphicIdWaterFirst = GraphicId(ID_Graphic_TerrainWater);
	int graphicIdWaterLast = 5 + graphicIdWaterFirst;

	FOREACH_XY_VIEW({
		//we can draw!
		int gridOffset = ViewToGridOffset(xView, yView);
		// TODO something related to elevation/reservoirSimon?
		if (gridOffset < 0) {
			// Outside map: draw black tile
			Graphics_drawIsometricFootprint(GraphicId(ID_Graphic_TerrainBlack),
				xGraphic, yGraphic, 0);
		} else if (Data_Grid_edge[gridOffset] & Edge_LeftmostTile) {
			// Valid gridOffset and leftmost tile -> draw
			int buildingId = Data_Grid_buildingIds[gridOffset];
			Color colorMask = 0;
			if (buildingId) {
				if (Data_Buildings[buildingId].isDeleted) {
					colorMask = 0xF863;
				}
				if (x < 4) {
					Sound_City_markBuildingView(buildingId, 0);
				} else if (x > Data_CityView.widthInTiles + 2) {
					Sound_City_markBuildingView(buildingId, 4);
				} else {
					Sound_City_markBuildingView(buildingId, 2);
				}
			}
			if (Data_Grid_terrain[gridOffset] & Terrain_Garden) {
				Data_Buildings[0].type = Terrain_Garden;
				Sound_City_markBuildingView(0, 2);
			}
			int graphicId = Data_Grid_graphicIds[gridOffset];
			if (Data_Grid_bitfields[gridOffset] & Bitfield_Overlay) {
				graphicId = GraphicId(ID_Graphic_TerrainOverlay);
			}
			switch (Data_Grid_bitfields[gridOffset] & Bitfield_Sizes) {
				case Bitfield_Size1:
					if (advanceAnimation &&
						graphicId >= graphicIdWaterFirst &&
						graphicId <= graphicIdWaterLast) {
						graphicId++;
						if (graphicId > graphicIdWaterLast) {
							graphicId = graphicIdWaterFirst;
						}
						Data_Grid_graphicIds[gridOffset] = graphicId;
					}
					Graphics_drawIsometricFootprint(graphicId, xGraphic, yGraphic, colorMask);
					break;
				case Bitfield_Size2:
					Graphics_drawIsometricFootprint(graphicId, xGraphic + 30, yGraphic - 15, colorMask);
					break;
				case Bitfield_Size3:
					Graphics_drawIsometricFootprint(graphicId, xGraphic + 60, yGraphic - 30, colorMask);
					break;
				case Bitfield_Size4:
					Graphics_drawIsometricFootprint(graphicId, xGraphic + 90, yGraphic - 45, colorMask);
					break;
				case Bitfield_Size5:
					Graphics_drawIsometricFootprint(graphicId, xGraphic + 120, yGraphic - 60, colorMask);
					break;
			}
		}
	});
}

static void drawBuildingTopsWalkersAnimation(int selectedWalkerId)
{
	FOREACH_Y_VIEW(
		FOREACH_X_VIEW(
			if (Data_Grid_edge[gridOffset] & Edge_LeftmostTile) {
				int buildingId = Data_Grid_buildingIds[gridOffset];
				int graphicId = Data_Grid_graphicIds[gridOffset];
				Color colorMask = 0;
				if (buildingId && Data_Buildings[buildingId].isDeleted) {
					colorMask = Color_MaskRed;
				}
				switch (Data_Grid_bitfields[gridOffset] & Bitfield_Sizes) {
					case 0: DRAWTOP_SIZE1_C(graphicId, xGraphic, yGraphic, colorMask); break;
					case 1: DRAWTOP_SIZE2_C(graphicId, xGraphic, yGraphic, colorMask); break;
					case 2: DRAWTOP_SIZE3_C(graphicId, xGraphic, yGraphic, colorMask); break;
					case 4: DRAWTOP_SIZE4_C(graphicId, xGraphic, yGraphic, colorMask); break;
					case 8: DRAWTOP_SIZE5_C(graphicId, xGraphic, yGraphic, colorMask); break;
				}
				// specific buildings
				struct Data_Building *b = &Data_Buildings[buildingId];
				if (b->type == Building_SenateUpgraded) {
					// rating flags
					graphicId = GraphicId(ID_Graphic_Senate);
					Graphics_drawImageMasked(graphicId + 1, xGraphic + 138,
						yGraphic + 44 - Data_CityInfo.ratingCulture / 2, colorMask);
					Graphics_drawImageMasked(graphicId + 2, xGraphic + 168,
						yGraphic + 36 - Data_CityInfo.ratingProsperity / 2, colorMask);
					Graphics_drawImageMasked(graphicId + 3, xGraphic + 198,
						yGraphic + 27 - Data_CityInfo.ratingPeace / 2, colorMask);
					Graphics_drawImageMasked(graphicId + 4, xGraphic + 228,
						yGraphic + 19 - Data_CityInfo.ratingFavor / 2, colorMask);
					// unemployed
					graphicId = GraphicId(ID_Graphic_Homeless);
					if (Data_CityInfo.unemploymentPercentageForSenate > 0) {
						Graphics_drawImageMasked(graphicId + 108,
							xGraphic + 80, yGraphic, colorMask);
					}
					if (Data_CityInfo.unemploymentPercentageForSenate > 5) {
						Graphics_drawImageMasked(graphicId + 104,
							xGraphic + 230, yGraphic - 30, colorMask);
					}
					if (Data_CityInfo.unemploymentPercentageForSenate > 10) {
						Graphics_drawImageMasked(graphicId + 107,
							xGraphic + 100, yGraphic + 20, colorMask);
					}
					if (Data_CityInfo.unemploymentPercentageForSenate > 15) {
						Graphics_drawImageMasked(graphicId + 106,
							xGraphic + 235, yGraphic - 10, colorMask);
					}
					if (Data_CityInfo.unemploymentPercentageForSenate > 20) {
						Graphics_drawImageMasked(graphicId + 106,
							xGraphic + 66, yGraphic + 20, colorMask);
					}
				}
				if (b->type == Building_Amphitheater && b->numWorkers > 0) {
					Graphics_drawImageMasked(GraphicId(ID_Graphic_AmphitheaterShow),
						xGraphic + 36, yGraphic - 47, colorMask);
				}
				if (b->type == Building_Theater && b->numWorkers > 0) {
					Graphics_drawImageMasked(GraphicId(ID_Graphic_TheaterShow),
						xGraphic + 34, yGraphic - 22, colorMask);
				}
				if (b->type == Building_Hippodrome &&
					Data_Buildings[Building_getMainBuildingId(buildingId)].numWorkers > 0 &&
					Data_CityInfo.entertainmentHippodromeHasShow) {
					int subtype = b->subtype.orientation;
					if ((subtype == 0 || subtype == 3) && Data_CityInfo.population > 2000) {
						switch (Data_Settings_Map.orientation) {
							case Direction_Top:
								Graphics_drawImageMasked(
									GraphicId(ID_Graphic_Hippodrome2) + 6,
									xGraphic + 147, yGraphic - 72, colorMask);
								break;
							case Direction_Right:
								Graphics_drawImageMasked(
									GraphicId(ID_Graphic_Hippodrome1) + 8,
									xGraphic + 58, yGraphic - 79, colorMask);
								break;
							case Direction_Bottom:
								Graphics_drawImageMasked(
									GraphicId(ID_Graphic_Hippodrome2) + 8,
									xGraphic + 119, yGraphic - 80, colorMask);
								break;
							case Direction_Left:
								Graphics_drawImageMasked(
									GraphicId(ID_Graphic_Hippodrome1) + 6,
									xGraphic, yGraphic - 72, colorMask);
						}
					} else if ((subtype == 1 || subtype == 4) && Data_CityInfo.population > 100) {
						switch (Data_Settings_Map.orientation) {
							case Direction_Top:
							case Direction_Bottom:
								Graphics_drawImageMasked(
									GraphicId(ID_Graphic_Hippodrome2) + 7,
									xGraphic + 122, yGraphic - 79, colorMask);
								break;
							case Direction_Right:
							case Direction_Left:
								Graphics_drawImageMasked(
									GraphicId(ID_Graphic_Hippodrome1) + 7,
									xGraphic, yGraphic - 80, colorMask);
						}
					} else if ((subtype == 2 || subtype == 5) && Data_CityInfo.population > 1000) {
						switch (Data_Settings_Map.orientation) {
							case Direction_Top:
								Graphics_drawImageMasked(
									GraphicId(ID_Graphic_Hippodrome2) + 8,
									xGraphic + 119, yGraphic - 80, colorMask);
								break;
							case Direction_Right:
								Graphics_drawImageMasked(
									GraphicId(ID_Graphic_Hippodrome1) + 6,
									xGraphic, yGraphic - 72, colorMask);
								break;
							case Direction_Bottom:
								Graphics_drawImageMasked(
									GraphicId(ID_Graphic_Hippodrome2) + 6,
									xGraphic + 147, yGraphic - 72, colorMask);
								break;
							case Direction_Left:
								Graphics_drawImageMasked(
									GraphicId(ID_Graphic_Hippodrome1) + 8,
									xGraphic + 58, yGraphic - 79, colorMask);
						}
					}
				}
				if (b->type == Building_Colosseum && b->numWorkers > 0) {
					Graphics_drawImageMasked(GraphicId(ID_Graphic_ColosseumShow),
						xGraphic + 70, yGraphic - 90, colorMask);
				}
				// workshops
				if (b->type == Building_WineWorkshop) {
					if (b->rawMaterialsStored >= 2 || b->data.industry.hasFullResource) {
						Graphics_drawImageMasked(GraphicId(ID_Graphic_WorkshopRawMaterial),
							xGraphic + 45, yGraphic + 23, colorMask);
					}
				}
				if (b->type == Building_OilWorkshop) {
					if (b->rawMaterialsStored >= 2 || b->data.industry.hasFullResource) {
						Graphics_drawImageMasked(GraphicId(ID_Graphic_WorkshopRawMaterial) + 1,
							xGraphic + 35, yGraphic + 15, colorMask);
					}
				}
				if (b->type == Building_WeaponsWorkshop) {
					if (b->rawMaterialsStored >= 2 || b->data.industry.hasFullResource) {
						Graphics_drawImageMasked(GraphicId(ID_Graphic_WorkshopRawMaterial) + 3,
							xGraphic + 46, yGraphic + 24, colorMask);
					}
				}
				if (b->type == Building_FurnitureWorkshop) {
					if (b->rawMaterialsStored >= 2 || b->data.industry.hasFullResource) {
						Graphics_drawImageMasked(GraphicId(ID_Graphic_WorkshopRawMaterial) + 2,
							xGraphic + 48, yGraphic + 19, colorMask);
					}
				}
				if (b->type == Building_PotteryWorkshop) {
					if (b->rawMaterialsStored >= 2 || b->data.industry.hasFullResource) {
						Graphics_drawImageMasked(GraphicId(ID_Graphic_WorkshopRawMaterial) + 4,
							xGraphic + 47, yGraphic + 24, colorMask);
					}
				}
			}
		);
		// draw walkers
		FOREACH_X_VIEW(
			int walkerId = Data_Grid_walkerIds[gridOffset];
			while (walkerId) {
				if (!Data_Walkers[walkerId].isGhost) {
					drawWalker(walkerId, xGraphic, yGraphic, selectedWalkerId);
				}
				walkerId = Data_Walkers[walkerId].nextWalkerIdOnSameTile;
			}
		);
		// draw animation
		FOREACH_X_VIEW(
			int graphicId = Data_Grid_graphicIds[gridOffset];
			if (GraphicNumAnimationSprites(graphicId)) {
				if (Data_Grid_edge[gridOffset] & Edge_LeftmostTile) {
					int buildingId = Data_Grid_buildingIds[gridOffset];
					struct Data_Building *b = &Data_Buildings[buildingId];
					int colorMask = 0;
					if (buildingId && b->isDeleted) {
						colorMask = Color_MaskRed;
					}
					if (b->type == Building_Dock) {
						int numDockers = Building_Dock_getNumIdleDockers(buildingId);
						if (numDockers > 0) {
							int graphicIdDock = Data_Grid_graphicIds[b->gridOffset];
							int graphicIdDockers = GraphicId(ID_Graphic_Dockers);
							if (graphicIdDock == GraphicId(ID_Graphic_Dock)) {
								graphicIdDockers += 0;
							} else if (graphicIdDock == GraphicId(ID_Graphic_Dock2)) {
								graphicIdDockers += 3;
							} else if (graphicIdDock == GraphicId(ID_Graphic_Dock3)) {
								graphicIdDockers += 6;
							} else {
								graphicIdDockers += 9;
							}
							if (numDockers == 2) {
								graphicIdDockers += 1;
							} else if (numDockers == 3) {
								graphicIdDockers += 2;
							}
							Graphics_drawImageMasked(graphicIdDockers,
								xGraphic + GraphicSpriteOffsetX(graphicIdDockers),
								yGraphic + GraphicSpriteOffsetX(graphicIdDockers),
								colorMask);
						}
					}
					if (b->type == Building_Warehouse) {
						Graphics_drawImageMasked(GraphicId(ID_Graphic_Warehouse) + 17,
							xGraphic - 4, yGraphic - 42, colorMask);
						if (buildingId == Data_CityInfo.buildingTradeCenterBuildingId) {
							Graphics_drawImageMasked(GraphicId(ID_Graphic_TradeCenterFlag),
								xGraphic + 19, yGraphic - 56, colorMask);
						}
					}
					if (b->type == Building_Granary) {
						Graphics_drawImageMasked(GraphicId(ID_Graphic_Granary) + 1,
							xGraphic + GraphicSpriteOffsetX(graphicId),
							yGraphic + 60 + GraphicSpriteOffsetY(graphicId) - GraphicHeight(graphicId),
							colorMask);
						if (b->data.granary.spaceLeft < 2400) {
							Graphics_drawImageMasked(GraphicId(ID_Graphic_Granary) + 2,
								xGraphic + 33, yGraphic - 60, colorMask);
						}
						if (b->data.granary.spaceLeft < 1800) {
							Graphics_drawImageMasked(GraphicId(ID_Graphic_Granary) + 3,
								xGraphic + 56, yGraphic - 50, colorMask);
						}
						if (b->data.granary.spaceLeft < 1200) {
							Graphics_drawImageMasked(GraphicId(ID_Graphic_Granary) + 4,
								xGraphic + 91, yGraphic - 50, colorMask);
						}
						if (b->data.granary.spaceLeft < 600) {
							Graphics_drawImageMasked(GraphicId(ID_Graphic_Granary) + 5,
								xGraphic + 117, yGraphic - 62, colorMask);
						}
					}
					if (b->type == Building_BurningRuin && b->ruinHasPlague) {
						Graphics_drawImageMasked(GraphicId(ID_Graphic_PlagueSkull),
							xGraphic + 18, yGraphic - 32, colorMask);
					}
					int animationOffset = Animation_getIndexForCityBuilding(graphicId, gridOffset);
					if (b->type != Building_Hippodrome && animationOffset > 0) {
						if (animationOffset > GraphicNumAnimationSprites(graphicId)) {
							animationOffset = GraphicNumAnimationSprites(graphicId);
						}
						if (b->type == Building_Granary) {
							Graphics_drawImageMasked(graphicId + animationOffset + 5,
								xGraphic + 77, yGraphic - 49, colorMask);
						} else {
							int ydiff = 0;
							switch (Data_Grid_bitfields[gridOffset] & Bitfield_Sizes) {
								case 0: ydiff = 30; break;
								case 1: ydiff = 45; break;
								case 2: ydiff = 60; break;
								case 4: ydiff = 75; break;
								case 8: ydiff = 90; break;
							}
							Graphics_drawImageMasked(graphicId + animationOffset,
								xGraphic + GraphicSpriteOffsetX(graphicId),
								yGraphic + ydiff + GraphicSpriteOffsetY(graphicId) - GraphicHeight(graphicId),
								colorMask);
						}
					}
				}
			} else if (Data_Grid_spriteOffsets[gridOffset]) {
				drawBridge(gridOffset, xGraphic, yGraphic);
			} else if (Data_Buildings[Data_Grid_buildingIds[gridOffset]].type == Building_FortGround__) {
				if (Data_Grid_edge[gridOffset] & Edge_LeftmostTile) {
					int buildingId = Data_Grid_buildingIds[gridOffset];
					int offset = 0;
					switch (Data_Buildings[buildingId].subtype.fortWalkerType) {
						case Walker_FortLegionary: offset = 4; break;
						case Walker_FortMounted: offset = 3; break;
						case Walker_FortJavelin: offset = 2; break;
					}
					if (offset) {
						Graphics_drawImage(GraphicId(ID_Graphic_Fort) + offset,
							xGraphic + 81, yGraphic + 5);
					}
				}
			} else if (Data_Buildings[Data_Grid_buildingIds[gridOffset]].type == Building_Gatehouse) {
				int xy = Data_Grid_edge[gridOffset] & Edge_MaskXY;
				if ((Data_Settings_Map.orientation == Direction_Top && xy == 9) ||
					(Data_Settings_Map.orientation == Direction_Right && xy == 8) ||
					(Data_Settings_Map.orientation == Direction_Bottom && xy == 0) ||
					(Data_Settings_Map.orientation == Direction_Left && xy == 1)) {
					int buildingId = Data_Grid_buildingIds[gridOffset];
					int graphicId = GraphicId(ID_Graphic_Gatehouse);
					if (Data_Buildings[buildingId].subtype.orientation == 1) {
						if (Data_Settings_Map.orientation == Direction_Top || Data_Settings_Map.orientation == Direction_Bottom) {
							Graphics_drawImage(graphicId, xGraphic - 22, yGraphic - 80);
						} else {
							Graphics_drawImage(graphicId + 1, xGraphic - 18, yGraphic - 81);
						}
					} else if (Data_Buildings[buildingId].subtype.orientation == 2) {
						if (Data_Settings_Map.orientation == Direction_Top || Data_Settings_Map.orientation == Direction_Bottom) {
							Graphics_drawImage(graphicId + 1, xGraphic - 18, yGraphic - 81);
						} else {
							Graphics_drawImage(graphicId, xGraphic - 22, yGraphic - 80);
						}
					}
				}
			}
		);
	);
}

static void drawOverlayFootprints()
{
	FOREACH_XY_VIEW({
		int gridOffset = ViewToGridOffset(xView, yView);
		// TODO something related to elevation/reservoirSimon?
		if (gridOffset < 0) {
			// Outside map: draw black tile
			DRAWFOOT_SIZE1(GraphicId(ID_Graphic_TerrainBlack),
				xGraphic, yGraphic);
		} else if (Data_State.currentOverlay == Overlay_Desirability) {
			drawBuildingFootprintForDesirabilityOverlay(gridOffset, xGraphic, yGraphic);
		} else if (Data_Grid_edge[gridOffset] & Edge_LeftmostTile) {
			int terrain = Data_Grid_terrain[gridOffset];
			if (Data_State.currentOverlay == Overlay_Water) {
				drawFootprintForWaterOverlay(gridOffset, xGraphic, yGraphic);
			} else if (Data_State.currentOverlay == Overlay_Native) {
				drawFootprintForNativeOverlay(gridOffset, xGraphic, yGraphic);
			} else if (terrain & (Terrain_Aqueduct | Terrain_Wall)) {
				// display grass
				int graphicId = GraphicId(ID_Graphic_TerrainGrass1) + (Data_Grid_random[gridOffset] & 7);
				DRAWFOOT_SIZE1(graphicId, xGraphic, yGraphic);
			} else if ((terrain & Terrain_Road) && !(terrain & Terrain_Building)) {
				int graphicId = Data_Grid_graphicIds[gridOffset];
				switch (Data_Grid_bitfields[gridOffset] & Bitfield_Sizes) {
					case Bitfield_Size1:
						DRAWFOOT_SIZE1(graphicId, xGraphic, yGraphic);
						break;
					case Bitfield_Size2:
						DRAWFOOT_SIZE2(graphicId, xGraphic, yGraphic);
						break;
				}
			} else if (terrain & Terrain_Building) {
				drawBuildingFootprintForOverlay(Data_Grid_buildingIds[gridOffset],
					gridOffset, xGraphic, yGraphic, 0);
			} else {
				int graphicId = Data_Grid_graphicIds[gridOffset];
				switch (Data_Grid_bitfields[gridOffset] & Bitfield_Sizes) {
					case Bitfield_Size1:
						DRAWFOOT_SIZE1(graphicId, xGraphic, yGraphic);
						break;
					case Bitfield_Size2:
						DRAWFOOT_SIZE2(graphicId, xGraphic, yGraphic);
						break;
					case Bitfield_Size3:
						DRAWFOOT_SIZE3(graphicId, xGraphic, yGraphic);
						break;
					case Bitfield_Size4:
						DRAWFOOT_SIZE4(graphicId, xGraphic, yGraphic);
						break;
					case Bitfield_Size5:
						DRAWFOOT_SIZE5(graphicId, xGraphic, yGraphic);
						break;
				}
			}
		}
	});
}

#define WATER_TERRAIN 0x1777

static void drawFootprintForWaterOverlay(int gridOffset, int xOffset, int yOffset)
{
	int terrain = Data_Grid_terrain[gridOffset];
	if (terrain & WATER_TERRAIN) {
		if (terrain & Terrain_Building) {
			drawBuildingFootprintForOverlay(Data_Grid_buildingIds[gridOffset],
				gridOffset, xOffset, yOffset, 0);
		} else {
			int graphicId = Data_Grid_graphicIds[gridOffset];
			switch (Data_Grid_bitfields[gridOffset] & Bitfield_Sizes) {
				case Bitfield_Size1:
					DRAWFOOT_SIZE1(graphicId, xOffset, yOffset);
					break;
				case Bitfield_Size2:
					DRAWFOOT_SIZE2(graphicId, xOffset, yOffset);
					break;
				case Bitfield_Size3:
					DRAWFOOT_SIZE3(graphicId, xOffset, yOffset);
					break;
				case Bitfield_Size4:
					DRAWFOOT_SIZE4(graphicId, xOffset, yOffset);
					break;
				case Bitfield_Size5:
					DRAWFOOT_SIZE5(graphicId, xOffset, yOffset);
					break;
			}
		}
	} else if (terrain & Terrain_Wall) {
		// display grass
		int graphicId = GraphicId(ID_Graphic_TerrainGrass1) + (Data_Grid_random[gridOffset] & 7);
		DRAWFOOT_SIZE1(graphicId, xOffset, yOffset);
	} else if (terrain & Terrain_Building) {
		int buildingId = Data_Grid_buildingIds[gridOffset];
		if (buildingId && Data_Buildings[buildingId].houseHasWellAccess == 1) {
			terrain |= Terrain_FountainRange;
		}
		if (Data_Buildings[buildingId].type == Building_Well || Data_Buildings[buildingId].type == Building_Fountain) {
			DRAWFOOT_SIZE1(Data_Grid_graphicIds[gridOffset], xOffset, yOffset);
		} else if (Data_Buildings[buildingId].type == Building_Reservoir) {
			DRAWFOOT_SIZE3(Data_Grid_graphicIds[gridOffset], xOffset, yOffset);
		} else {
			int graphicOffset;
			switch (terrain & (Terrain_ReservoirRange | Terrain_FountainRange)) {
				case Terrain_ReservoirRange | Terrain_FountainRange:
					graphicOffset = 24;
					break;
				case Terrain_ReservoirRange:
					graphicOffset = 8;
					break;
				case Terrain_FountainRange:
					graphicOffset = 16;
					break;
				default:
					graphicOffset = 0;
					break;
			}
			drawBuildingFootprintForOverlay(buildingId, gridOffset, xOffset, yOffset, graphicOffset);
		}
	} else {
		int graphicId = GraphicId(ID_Graphic_TerrainOverlay);
		switch (terrain & (Terrain_ReservoirRange | Terrain_FountainRange)) {
			case Terrain_ReservoirRange | Terrain_FountainRange:
				graphicId += 27;
				break;
			case Terrain_ReservoirRange:
				graphicId += 11;
				break;
			case Terrain_FountainRange:
				graphicId += 19;
				break;
			default:
				graphicId = Data_Grid_graphicIds[gridOffset];
				break;
		}
		DRAWFOOT_SIZE1(graphicId, xOffset, yOffset);
	}
}

static void drawTopForWaterOverlay(int gridOffset, int xOffset, int yOffset)
{
	int terrain = Data_Grid_terrain[gridOffset];
	if (terrain & WATER_TERRAIN) {
		if (!(terrain & Terrain_Building)) {
			int graphicId = Data_Grid_graphicIds[gridOffset];
			switch (Data_Grid_bitfields[gridOffset] & Bitfield_Sizes) {
				case Bitfield_Size1:
					DRAWTOP_SIZE1(graphicId, xOffset, yOffset);
					break;
				case Bitfield_Size2:
					DRAWTOP_SIZE2(graphicId, xOffset, yOffset);
					break;
				case Bitfield_Size3:
					DRAWTOP_SIZE3(graphicId, xOffset, yOffset);
					break;
				case Bitfield_Size4:
					DRAWTOP_SIZE4(graphicId, xOffset, yOffset);
					break;
				case Bitfield_Size5:
					DRAWTOP_SIZE5(graphicId, xOffset, yOffset);
					break;
			}
		}
	} else if (Data_Grid_buildingIds[gridOffset]) {
		int buildingId = Data_Grid_buildingIds[gridOffset];
		if (Data_Buildings[buildingId].type == Building_Well || Data_Buildings[buildingId].type == Building_Fountain) {
			DRAWTOP_SIZE1(Data_Grid_graphicIds[gridOffset], xOffset, yOffset);
		} else if (Data_Buildings[buildingId].type == Building_Reservoir) {
			DRAWTOP_SIZE3(Data_Grid_graphicIds[gridOffset], xOffset, yOffset);
		}
	}
}

#define NATIVE_NATURAL_TERRAIN 0x1637

static void drawFootprintForNativeOverlay(int gridOffset, int xOffset, int yOffset)
{
	int terrain = Data_Grid_terrain[gridOffset];
	if (terrain & NATIVE_NATURAL_TERRAIN) {
		if (terrain & Terrain_Building) {
			drawBuildingFootprintForOverlay(Data_Grid_buildingIds[gridOffset],
				gridOffset, xOffset, yOffset, 0);
		} else {
			int graphicId = Data_Grid_graphicIds[gridOffset];
			switch (Data_Grid_bitfields[gridOffset] & Bitfield_Sizes) {
				case Bitfield_Size1:
					DRAWFOOT_SIZE1(graphicId, xOffset, yOffset);
					break;
				case Bitfield_Size2:
					DRAWFOOT_SIZE2(graphicId, xOffset, yOffset);
					break;
				case Bitfield_Size3:
					DRAWFOOT_SIZE3(graphicId, xOffset, yOffset);
					break;
				case Bitfield_Size4:
					DRAWFOOT_SIZE4(graphicId, xOffset, yOffset);
					break;
				case Bitfield_Size5:
					DRAWFOOT_SIZE5(graphicId, xOffset, yOffset);
					break;
			}
		}
	} else if (terrain & (Terrain_Wall | Terrain_Aqueduct)) {
		// display grass
		int graphicId = GraphicId(ID_Graphic_TerrainGrass1) + (Data_Grid_random[gridOffset] & 7);
		DRAWFOOT_SIZE1(graphicId, xOffset, yOffset);
	} else if (terrain & Terrain_Building) {
		drawBuildingFootprintForOverlay(Data_Grid_buildingIds[gridOffset],
		gridOffset, xOffset, yOffset, 0);
	} else {
		int graphicId = GraphicId(ID_Graphic_TerrainDesirability);
		if (Data_Grid_edge[gridOffset] & Edge_NativeLand) {
			DRAWTOP_SIZE1(graphicId + 1, xOffset, yOffset);
		} else {
			// kan alleen maar road/meadow/gatehouse zijn
			switch (Data_Grid_bitfields[gridOffset] & Bitfield_Sizes) {
				case Bitfield_Size1:
					DRAWFOOT_SIZE1(graphicId, xOffset, yOffset);
					break;
				case Bitfield_Size2:
					DRAWFOOT_SIZE2(graphicId, xOffset, yOffset);
					break;
			}
		}
	}
}

static void drawTopForNativeOverlay(int gridOffset, int xOffset, int yOffset)
{
	int terrain = Data_Grid_terrain[gridOffset];
	if (terrain & NATIVE_NATURAL_TERRAIN) {
		if (!(terrain & Terrain_Building)) {
			int graphicId = Data_Grid_graphicIds[gridOffset];
			switch (Data_Grid_bitfields[gridOffset] & Bitfield_Sizes) {
				case Bitfield_Size1:
					DRAWTOP_SIZE1(graphicId, xOffset, yOffset);
					break;
				case Bitfield_Size2:
					DRAWTOP_SIZE2(graphicId, xOffset, yOffset);
					break;
				case Bitfield_Size3:
					DRAWTOP_SIZE3(graphicId, xOffset, yOffset);
					break;
				case Bitfield_Size4:
					DRAWTOP_SIZE4(graphicId, xOffset, yOffset);
					break;
				case Bitfield_Size5:
					DRAWTOP_SIZE5(graphicId, xOffset, yOffset);
					break;
			}
		}
	} else if (Data_Grid_buildingIds[gridOffset]) {
		int graphicId = Data_Grid_graphicIds[gridOffset];
		switch (Data_Buildings[Data_Grid_buildingIds[gridOffset]].type) {
			case Building_NativeHut:
				DRAWTOP_SIZE1(graphicId, xOffset, yOffset);
				break;
			case Building_NativeMeeting:
			case Building_MissionPost:
				DRAWTOP_SIZE2(graphicId, xOffset, yOffset);
				break;
		}
	}
}

static void drawBuildingFootprintForOverlay(int buildingId, int gridOffset, int xOffset, int yOffset, int graphicOffset)
{
	if (!buildingId) {
		return;
	}
	
	int graphicId;
	int origGraphicId = Data_Grid_graphicIds[gridOffset];
	Data_Building *b = &Data_Buildings[buildingId];
	if (b->size == 1) {
		graphicId = GraphicId(ID_Graphic_TerrainOverlay);
		if (b->houseSize) {
			graphicId += 4;
		}
		switch (Data_State.currentOverlay) {
			case Overlay_Damage:
				if (b->type == Building_EngineersPost) {
					graphicId = origGraphicId;
				}
				break;
			case Overlay_Barber:
				if (b->type == Building_Barber) {
					graphicId = origGraphicId;
				}
				break;
			case Overlay_Clinic:
				if (b->type == Building_Doctor) {
					graphicId = origGraphicId;
				}
				break;
			case Overlay_Native:
				if (b->type == Building_NativeHut) {
					graphicId = origGraphicId;
					graphicOffset = 0;
				}
				break;
			case Overlay_Problems:
				if (b->showOnProblemOverlay) {
					graphicId = origGraphicId;
				}
				break;
			case Overlay_Fire:
			case Overlay_Crime:
				if (b->type == Building_Prefecture || b->type == Building_BurningRuin) {
					graphicId = origGraphicId;
				}
				break;
		}
		graphicId += graphicOffset;
		DRAWFOOT_SIZE1(graphicId, xOffset, yOffset);
	} else if (b->size == 2) {
		int drawOrig = 0;
		switch (Data_State.currentOverlay) {
			case Overlay_Entertainment:
			case Overlay_Theater:
				if (b->type == Building_Theater) {
					drawOrig = 1;
				}
				break;
			case Overlay_Education:
				if (b->type == Building_School || b->type == Building_Library) {
					drawOrig = 1;
				}
				break;
			case Overlay_School:
				if (b->type == Building_School) {
					drawOrig = 1;
				}
				break;
			case Overlay_Library:
				if (b->type == Building_Library) {
					drawOrig = 1;
				}
				break;
			case Overlay_Bathhouse:
				if (b->type == Building_Bathhouse) {
					drawOrig = 1;
				}
				break;
			case Overlay_Religion:
				if (b->type == Building_Oracle || b->type == Building_SmallTempleCeres ||
					b->type == Building_SmallTempleNeptune || b->type == Building_SmallTempleMercury ||
					b->type == Building_SmallTempleMars || b->type == Building_SmallTempleVenus) {
					drawOrig = 1;
				}
				break;
			case Overlay_FoodStocks:
				if (b->type == Building_Market || b->type == Building_Wharf) {
					drawOrig = 1;
				}
				break;
			case Overlay_TaxIncome:
				if (b->type == Building_Forum) {
					drawOrig = 1;
				}
				break;
			case Overlay_Native:
				if (b->type == Building_NativeMeeting || b->type == Building_MissionPost) {
					drawOrig = 1;
				}
				break;
			case Overlay_Problems:
				if (b->showOnProblemOverlay) {
					drawOrig = 1;
				}
				break;
		}
		if (drawOrig) {
			DRAWFOOT_SIZE2(Data_Grid_graphicIds[gridOffset], xOffset, yOffset);
		} else {
			int graphicBase = GraphicId(ID_Graphic_TerrainOverlay) + graphicOffset;
			if (b->houseSize) {
				graphicBase += 4;
			}
			int xTileOffset[] = {30, 0, 60, 30};
			int yTileOffset[] = {-15, 0, 0, 15};
			for (int i = 0; i < 4; i++) {
				Graphics_drawIsometricFootprint(graphicBase + i,
					xOffset + xTileOffset[i], yOffset + yTileOffset[i], 0);
			}
		}
	} else if (b->size == 3) {
		int drawOrig = 0;
		switch (Data_State.currentOverlay) {
			case Overlay_Entertainment:
				if (b->type == Building_Amphitheater || b->type == Building_GladiatorSchool ||
					b->type == Building_LionHouse || b->type == Building_ActorColony ||
					b->type == Building_ChariotMaker) {
					drawOrig = 1;
				}
				break;
			case Overlay_Theater:
				if (b->type == Building_ActorColony) {
					drawOrig = 1;
				}
				break;
			case Overlay_Amphitheater:
				if (b->type == Building_ActorColony || b->type == Building_GladiatorSchool ||
					b->type == Building_Amphitheater) {
					drawOrig = 1;
				}
				break;
			case Overlay_Colosseum:
				if (b->type == Building_GladiatorSchool || b->type == Building_LionHouse) {
					drawOrig = 1;
				}
				break;
			case Overlay_Hippodrome:
				if (b->type == Building_ChariotMaker) {
					drawOrig = 1;
				}
				break;
			case Overlay_Education:
			case Overlay_Academy:
				if (b->type == Building_Academy) {
					drawOrig = 1;
				}
				break;
			case Overlay_Hospital:
				if (b->type == Building_Hospital) {
					drawOrig = 1;
				}
				break;
			case Overlay_Religion:
				if (b->type == Building_LargeTempleCeres || b->type == Building_LargeTempleNeptune ||
					b->type == Building_LargeTempleMercury || b->type == Building_LargeTempleMars ||
					b->type == Building_LargeTempleVenus) {
					drawOrig = 1;
				}
				break;
			case Overlay_FoodStocks:
				if (b->type == Building_Granary) {
					drawOrig = 1;
				}
				break;
			case Overlay_Problems:
				if (b->showOnProblemOverlay) {
					drawOrig = 1;
				}
				break;
		}
		// farms have apparently multiple tiles with 0x40
		if (drawOrig) {
			if (b->type >= Building_WheatFarm && b->type <= Building_PigFarm) {
				int isField = 0;
				int edge = Data_Grid_edge[gridOffset];
				if ((Data_Settings_Map.orientation == Direction_Top && edge != 0x48) ||
					(Data_Settings_Map.orientation == Direction_Right && edge != 0x40) ||
					(Data_Settings_Map.orientation == Direction_Bottom && edge != 0x41) ||
					(Data_Settings_Map.orientation == Direction_Left && edge != 0x49)) {
					isField = 1;
				}
				if (isField) {
					if (edge & Edge_LeftmostTile) {
						DRAWFOOT_SIZE1(Data_Grid_graphicIds[gridOffset], xOffset, yOffset);
					}
				} else { // farmhouse
					DRAWFOOT_SIZE2(Data_Grid_graphicIds[gridOffset], xOffset, yOffset);
				}
			} else {
				DRAWFOOT_SIZE3(Data_Grid_graphicIds[gridOffset], xOffset, yOffset);
			}
		} else {
			int draw = 1;
			if (b->type >= Building_WheatFarm && b->type <= Building_PigFarm) {
				int edge = Data_Grid_edge[gridOffset];
				if ((Data_Settings_Map.orientation == Direction_Top && edge != 0x50) ||
					(Data_Settings_Map.orientation == Direction_Right && edge != 0x40) ||
					(Data_Settings_Map.orientation == Direction_Bottom && edge != 0x42) ||
					(Data_Settings_Map.orientation == Direction_Left && edge != 0x52)) {
					draw = 0;
				}
			}
			if (draw) {
				int graphicBase = GraphicId(ID_Graphic_TerrainOverlay) + graphicOffset;
				if (b->houseSize) {
					graphicBase += 4;
				}
				int graphicTileOffset[] = {0, 1, 2, 1, 3, 2, 3, 3, 3};
				int xTileOffset[] = {60, 30, 90, 0, 60, 120, 30, 90, 60};
				int yTileOffset[] = {-30, -15, -15, 0, 0, 0, 15, 15, 30};
				for (int i = 0; i < 9; i++) {
					Graphics_drawIsometricFootprint(graphicBase + graphicTileOffset[i],
						xOffset + xTileOffset[i], yOffset + yTileOffset[i], 0);
				}
			}
		}
	} else if (b->size == 4) {
		int graphicBase = GraphicId(ID_Graphic_TerrainOverlay) + graphicOffset;
		if (b->houseSize) {
			graphicBase += 4;
		}
		int graphicTileOffset[] = {0, 1, 2, 1, 3, 2, 1, 3, 3, 2, 3, 3, 3, 3, 3, 3};
		int xTileOffset[] = {
			90,
			60, 120,
			30, 90, 150,
			0, 60, 120, 180,
			30, 90, 150,
			60, 120,
			90
		};
		int yTileOffset[] = {
			-45,
			-30, -30,
			-15, -15, -15,
			0, 0, 0, 0,
			15, 15, 15,
			30, 30,
			45
		};
		for (int i = 0; i < 16; i++) {
			Graphics_drawIsometricFootprint(graphicBase + graphicTileOffset[i],
				xOffset + xTileOffset[i], yOffset + yTileOffset[i], 0);
		}
	} else if (b->size == 5) {
		int drawOrig = 0;
		switch (Data_State.currentOverlay) {
			case Overlay_Entertainment:
				if (b->type == Building_Hippodrome || b->type == Building_Colosseum) {
					drawOrig = 1;
				}
				break;
			case Overlay_Colosseum:
				if (b->type == Building_Colosseum) {
					drawOrig = 1;
				}
				break;
			case Overlay_Hippodrome:
				if (b->type == Building_Hippodrome) {
					drawOrig = 1;
				}
				break;
			case Overlay_TaxIncome:
				if (b->type == Building_SenateUpgraded) {
					drawOrig = 1;
				}
				break;
			case Overlay_Problems:
				if (b->showOnProblemOverlay) {
					drawOrig = 1;
				}
				break;
		}
		if (drawOrig) {
			DRAWFOOT_SIZE5(Data_Grid_graphicIds[gridOffset], xOffset, yOffset);
		} else {
			int graphicBase = GraphicId(ID_Graphic_TerrainOverlay) + graphicOffset;
			if (b->houseSize) {
				graphicBase += 4;
			}
			int graphicTileOffset[] = {0, 1, 2, 1, 3, 2, 1, 3, 3, 2,
				1, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
			int xTileOffset[] = {
				120,
				90, 150,
				60, 120, 180,
				30, 90, 150, 210,
				0, 60, 120, 180, 240,
				30, 90, 150, 210,
				60, 120, 180,
				90, 150,
				120
			};
			int yTileOffset[] = {
				-60,
				-45, -45,
				-30, -30, -30,
				-15, -15, -15, -15,
				0, 0, 0, 0, 0,
				15, 15, 15, 15,
				30, 30, 30,
				45, 45,
				60
			};
			for (int i = 0; i < 25; i++) {
				Graphics_drawIsometricFootprint(graphicBase + graphicTileOffset[i],
					xOffset + xTileOffset[i], yOffset + yTileOffset[i], 0);
			}
		}
	}
}


static void drawBuildingFootprintForDesirabilityOverlay(int gridOffset, int xOffset, int yOffset)
{
	int terrain = Data_Grid_terrain[gridOffset];
	if ((terrain & Terrain_NaturalElements) && !(terrain & Terrain_Building)) {
		// display normal tile
		if (Data_Grid_edge[gridOffset] & Edge_LeftmostTile) {
			int graphicId = Data_Grid_graphicIds[gridOffset];
			switch (Data_Grid_bitfields[gridOffset] & Bitfield_Sizes) {
				case Bitfield_Size1:
					DRAWFOOT_SIZE1(graphicId, xOffset, yOffset);
					break;
				case Bitfield_Size2:
					DRAWFOOT_SIZE2(graphicId, xOffset, yOffset);
					break;
				case Bitfield_Size3:
					DRAWFOOT_SIZE3(graphicId, xOffset, yOffset);
					break;
				case Bitfield_Size4:
					DRAWFOOT_SIZE4(graphicId, xOffset, yOffset);
					break;
				case Bitfield_Size5:
					DRAWFOOT_SIZE5(graphicId, xOffset, yOffset);
					break;
			}
		}
	} else if (terrain & (Terrain_Wall | Terrain_Aqueduct)) {
		// display empty land/grass
		int graphicId = GraphicId(ID_Graphic_TerrainGrass1) + (Data_Grid_random[gridOffset] & 7);
		DRAWFOOT_SIZE1(graphicId, xOffset, yOffset);
	} else if ((terrain & Terrain_Building) || Data_Grid_desirability[gridOffset]) {
		int des = Data_Grid_desirability[gridOffset];
		int offset = 0;
		if (des < -10) {
			offset = 0;
		} else if (des < -5) {
			offset = 1;
		} else if (des < 0) {
			offset = 2;
		} else if (des == 1) {
			offset = 3;
		} else if (des < 5) {
			offset = 4;
		} else if (des < 10) {
			offset = 5;
		} else if (des < 15) {
			offset = 6;
		} else if (des < 20) {
			offset = 7;
		} else if (des < 25) {
			offset = 8;
		} else {
			offset = 9;
		}
		DRAWFOOT_SIZE1(GraphicId(ID_Graphic_TerrainDesirability) + offset, xOffset, yOffset);
	} else {
		DRAWFOOT_SIZE1(Data_Grid_graphicIds[gridOffset], xOffset, yOffset);
	}
}

static void drawBuildingTopForDesirabilityOverlay(int gridOffset, int xOffset, int yOffset)
{
	int terrain = Data_Grid_terrain[gridOffset];
	// enum const: Terrain_NaturalElements = 0x1677
	if ((terrain & Terrain_NaturalElements) && !(terrain & Terrain_Building)) {
		// display normal tile
		if (Data_Grid_edge[gridOffset] & Edge_LeftmostTile) {
			int graphicId = Data_Grid_graphicIds[gridOffset];
			switch (Data_Grid_bitfields[gridOffset] & Bitfield_Sizes) {
				case Bitfield_Size1:
					DRAWTOP_SIZE1(graphicId, xOffset, yOffset);
					break;
				case Bitfield_Size2:
					DRAWTOP_SIZE2(graphicId, xOffset, yOffset);
					break;
				case Bitfield_Size3:
					DRAWTOP_SIZE3(graphicId, xOffset, yOffset);
					break;
				case Bitfield_Size4:
					DRAWTOP_SIZE4(graphicId, xOffset, yOffset);
					break;
				case Bitfield_Size5:
					DRAWTOP_SIZE5(graphicId, xOffset, yOffset);
					break;
			}
		}
	} else if (terrain & (Terrain_Wall | Terrain_Aqueduct)) {
		// grass, no top needed
	} else if ((terrain & Terrain_Building) || Data_Grid_desirability[gridOffset]) {
		int des = Data_Grid_desirability[gridOffset];
		int offset;
		if (des < -10) {
			offset = 0;
		} else if (des < -5) {
			offset = 1;
		} else if (des < 0) {
			offset = 2;
		} else if (des == 1) {
			offset = 3;
		} else if (des < 5) {
			offset = 4;
		} else if (des < 10) {
			offset = 5;
		} else if (des < 15) {
			offset = 6;
		} else if (des < 20) {
			offset = 7;
		} else if (des < 25) {
			offset = 8;
		} else {
			offset = 9;
		}
		DRAWTOP_SIZE1(GraphicId(ID_Graphic_TerrainDesirability) + offset, xOffset, yOffset);
	} else {
		DRAWTOP_SIZE1(Data_Grid_graphicIds[gridOffset], xOffset, yOffset);
	}
}

static void drawBuildingTopForFireOverlay(int gridOffset, int buildingId, int xOffset, int yOffset)
{
	int graphicId = Data_Grid_graphicIds[gridOffset];
	if (Data_Buildings[buildingId].type == Building_Prefecture) {
		DRAWTOP_SIZE1(graphicId, xOffset, yOffset);
	} else if (Data_Buildings[buildingId].type == Building_BurningRuin) {
		DRAWTOP_SIZE1(graphicId, xOffset, yOffset);
	} else if (Data_Buildings[buildingId].fireRisk > 0) {
		int draw = 1;
		if (Data_Buildings[buildingId].type >= Building_WheatFarm &&
			Data_Buildings[buildingId].type <= Building_PigFarm) {
			int edge = Data_Grid_edge[gridOffset];
			if ((Data_Settings_Map.orientation == Direction_Top && edge != 0x50) ||
				(Data_Settings_Map.orientation == Direction_Right && edge != 0x40) ||
				(Data_Settings_Map.orientation == Direction_Bottom && edge != 0x42) ||
				(Data_Settings_Map.orientation == Direction_Left && edge != 0x52)) {
				draw = 0;
			}
		}
		if (draw) {
			drawOverlayColumn(
				Data_Buildings[buildingId].fireRisk / 10,
				xOffset, yOffset, 1);
		}
	}
}

static void drawBuildingTopForDamageOverlay(int gridOffset, int buildingId, int xOffset, int yOffset)
{
	int graphicId = Data_Grid_graphicIds[gridOffset];
	if (Data_Buildings[buildingId].type == Building_EngineersPost) {
		DRAWTOP_SIZE1(graphicId, xOffset, yOffset);
	} else if (Data_Buildings[buildingId].damageRisk > 0) {
		int draw = 1;
		if (Data_Buildings[buildingId].type >= Building_WheatFarm &&
			Data_Buildings[buildingId].type <= Building_PigFarm) {
			int edge = Data_Grid_edge[gridOffset];
			if ((Data_Settings_Map.orientation == Direction_Top && edge != 0x50) ||
				(Data_Settings_Map.orientation == Direction_Right && edge != 0x40) ||
				(Data_Settings_Map.orientation == Direction_Bottom && edge != 0x42) ||
				(Data_Settings_Map.orientation == Direction_Left && edge != 0x52)) {
				draw = 0;
			}
		}
		if (draw) {
			drawOverlayColumn(
				Data_Buildings[buildingId].damageRisk / 10,
				xOffset, yOffset, 1);
		}
	}
}

static void drawBuildingTopForCrimeOverlay(int gridOffset, int buildingId, int xOffset, int yOffset)
{
	int graphicId = Data_Grid_graphicIds[gridOffset];
	if (Data_Buildings[buildingId].type == Building_Prefecture) {
		DRAWTOP_SIZE1(graphicId, xOffset, yOffset);
	} else if (Data_Buildings[buildingId].type == Building_BurningRuin) {
		DRAWTOP_SIZE1(graphicId, xOffset, yOffset);
	} else if (Data_Buildings[buildingId].houseSize) {
		int happiness = Data_Buildings[buildingId].sentiment.houseHappiness;
		if (happiness < 50) {
			int colVal;
			if (happiness <= 0) {
				colVal = 10;
			} else if (happiness <= 10) {
				colVal = 8;
			} else if (happiness <= 20) {
				colVal = 6;
			} else if (happiness <= 30) {
				colVal = 4;
			} else if (happiness <= 40) {
				colVal = 2;
			} else {
				colVal = 1;
			}
			drawOverlayColumn(colVal, xOffset, yOffset, 1);
		}
	}
}

static void drawBuildingTopForEntertainmentOverlay(int gridOffset, int buildingId, int xOffset, int yOffset)
{
	int graphicId = Data_Grid_graphicIds[gridOffset];
	switch (Data_Buildings[buildingId].type) {
		case Building_Theater:
			DRAWTOP_SIZE2(graphicId, xOffset, yOffset);
			break;
		case Building_ActorColony:
		case Building_GladiatorSchool:
		case Building_LionHouse:
		case Building_ChariotMaker:
		case Building_Amphitheater:
			DRAWTOP_SIZE3(graphicId, xOffset, yOffset);
			break;
		case Building_Colosseum:
		case Building_Hippodrome:
			DRAWTOP_SIZE5(graphicId, xOffset, yOffset);
			break;
		default:
			if (Data_Buildings[buildingId].houseSize) {
				if (Data_Buildings[buildingId].data.house.entertainment) {
					drawOverlayColumn(
						Data_Buildings[buildingId].data.house.entertainment / 10,
						xOffset, yOffset, 0);
				}
			}
			break;
	}
}

static void drawBuildingTopForEducationOverlay(int gridOffset, int buildingId, int xOffset, int yOffset)
{
	int graphicId = Data_Grid_graphicIds[gridOffset];
	switch (Data_Buildings[buildingId].type) {
		case Building_Academy:
			DRAWTOP_SIZE3(graphicId, xOffset, yOffset);
			break;
		case Building_Library:
		case Building_School:
			DRAWTOP_SIZE2(graphicId, xOffset, yOffset);
			break;
		default:
			if (Data_Buildings[buildingId].houseSize) {
				if (Data_Buildings[buildingId].data.house.education) {
					drawOverlayColumn(
						Data_Buildings[buildingId].data.house.education * 3 - 1,
						xOffset, yOffset, 0);
				}
			}
			break;
	}
}

static void drawBuildingTopForTheaterOverlay(int gridOffset, int buildingId, int xOffset, int yOffset)
{
	int graphicId = Data_Grid_graphicIds[gridOffset];
	switch (Data_Buildings[buildingId].type) {
		case Building_ActorColony:
			DRAWTOP_SIZE3(graphicId, xOffset, yOffset);
			break;
		case Building_Theater:
			DRAWTOP_SIZE2(graphicId, xOffset, yOffset);
			break;
		default:
			if (Data_Buildings[buildingId].houseSize) {
				if (Data_Buildings[buildingId].data.house.theater) {
					drawOverlayColumn(
						Data_Buildings[buildingId].data.house.theater / 10,
						xOffset, yOffset, 0);
				}
			}
			break;
	}
}

static void drawBuildingTopForAmphitheaterOverlay(int gridOffset, int buildingId, int xOffset, int yOffset)
{
	int graphicId = Data_Grid_graphicIds[gridOffset];
	switch (Data_Buildings[buildingId].type) {
		case Building_ActorColony:
		case Building_GladiatorSchool:
		case Building_Amphitheater:
			DRAWTOP_SIZE3(graphicId, xOffset, yOffset);
			break;
		default:
			if (Data_Buildings[buildingId].houseSize) {
				if (Data_Buildings[buildingId].data.house.amphitheaterActor) {
					drawOverlayColumn(
						Data_Buildings[buildingId].data.house.amphitheaterActor / 10,
						xOffset, yOffset, 0);
				}
			}
			break;
	}
}

static void drawBuildingTopForColosseumOverlay(int gridOffset, int buildingId, int xOffset, int yOffset)
{
	int graphicId = Data_Grid_graphicIds[gridOffset];
	switch (Data_Buildings[buildingId].type) {
		case Building_GladiatorSchool:
		case Building_LionHouse:
			DRAWTOP_SIZE3(graphicId, xOffset, yOffset);
			break;
		case Building_Colosseum:
			DRAWTOP_SIZE5(graphicId, xOffset, yOffset);
			break;
		default:
			if (Data_Buildings[buildingId].houseSize) {
				if (Data_Buildings[buildingId].data.house.colosseumGladiator) {
					drawOverlayColumn(
						Data_Buildings[buildingId].data.house.colosseumGladiator / 10,
						xOffset, yOffset, 0);
				}
			}
			break;
	}
}

static void drawBuildingTopForHippodromeOverlay(int gridOffset, int buildingId, int xOffset, int yOffset)
{
	int graphicId = Data_Grid_graphicIds[gridOffset];
	if (Data_Buildings[buildingId].type == Building_Hippodrome) {
		DRAWTOP_SIZE5(graphicId, xOffset, yOffset);
	} else if (Data_Buildings[buildingId].type == Building_ChariotMaker) {
		DRAWTOP_SIZE3(graphicId, xOffset, yOffset);
	} else if (Data_Buildings[buildingId].houseSize) {
		if (Data_Buildings[buildingId].data.house.hippodrome) {
			drawOverlayColumn(
				Data_Buildings[buildingId].data.house.hippodrome / 10,
				xOffset, yOffset, 0);
		}
	}
}

static void drawBuildingTopForFoodStocksOverlay(int gridOffset, int buildingId, int xOffset, int yOffset)
{
	int graphicId = Data_Grid_graphicIds[gridOffset];
	switch (Data_Buildings[buildingId].type) {
		case Building_Market:
		case Building_Wharf:
			DRAWTOP_SIZE2(graphicId, xOffset, yOffset);
			break;
		case Building_Granary:
			DRAWTOP_SIZE3(graphicId, xOffset, yOffset);
			break;
		default:
			if (Data_Buildings[buildingId].houseSize) {
				Data_Building *b = &Data_Buildings[buildingId];
				if (Data_Model_Houses[b->subtype.houseLevel].foodTypes) {
					int pop = b->housePopulation;
					int stocks = 0;
					for (int i = 0; i < 4; i++) {
						stocks += b->data.house.inventory.all[i];
					}
					int pctStocks = Calc_getPercentage(stocks, pop);
					int colVal = 0;
					if (pctStocks <= 0) {
						colVal = 10;
					} else if (pctStocks < 100) {
						colVal = 5;
					} else if (pctStocks <= 200) {
						colVal = 1;
					}
					if (colVal) {
						drawOverlayColumn(colVal, xOffset, yOffset, 1);
					}
				}
			}
			break;
	}
}

static void drawBuildingTopForBathhouseOverlay(int gridOffset, int buildingId, int xOffset, int yOffset)
{
	int graphicId = Data_Grid_graphicIds[gridOffset];
	if (Data_Buildings[buildingId].type == Building_Bathhouse) {
		DRAWTOP_SIZE2(graphicId, xOffset, yOffset);
	} else if (Data_Buildings[buildingId].houseSize) {
		if (Data_Buildings[buildingId].data.house.bathhouse) {
			drawOverlayColumn(
				Data_Buildings[buildingId].data.house.bathhouse / 10,
				xOffset, yOffset, 0);
		}
	}
}

static void drawBuildingTopForReligionOverlay(int gridOffset, int buildingId, int xOffset, int yOffset)
{
	int graphicId = Data_Grid_graphicIds[gridOffset];
	switch (Data_Buildings[buildingId].type) {
		case Building_Oracle:
		case Building_SmallTempleCeres:
		case Building_SmallTempleNeptune:
		case Building_SmallTempleMercury:
		case Building_SmallTempleMars:
		case Building_SmallTempleVenus:
			DRAWTOP_SIZE2(graphicId, xOffset, yOffset);
			break;
		case Building_LargeTempleCeres:
		case Building_LargeTempleNeptune:
		case Building_LargeTempleMercury:
		case Building_LargeTempleMars:
		case Building_LargeTempleVenus:
			DRAWTOP_SIZE3(graphicId, xOffset, yOffset);
			break;
		default:
			if (Data_Buildings[buildingId].houseSize) {
				if (Data_Buildings[buildingId].data.house.numGods) {
					drawOverlayColumn(
						Data_Buildings[buildingId].data.house.numGods * 17 / 10,
						xOffset, yOffset, 0);
				}
			}
			break;
	}
}

static void drawBuildingTopForSchoolOverlay(int gridOffset, int buildingId, int xOffset, int yOffset)
{
	int graphicId = Data_Grid_graphicIds[gridOffset];
	if (Data_Buildings[buildingId].type == Building_School) {
		DRAWTOP_SIZE2(graphicId, xOffset, yOffset);
	} else if (Data_Buildings[buildingId].houseSize) {
		if (Data_Buildings[buildingId].data.house.school) {
			drawOverlayColumn(
				Data_Buildings[buildingId].data.house.school / 10,
				xOffset, yOffset, 0);
		}
	}
}

static void drawBuildingTopForLibraryOverlay(int gridOffset, int buildingId, int xOffset, int yOffset)
{
	int graphicId = Data_Grid_graphicIds[gridOffset];
	if (Data_Buildings[buildingId].type == Building_Library) {
		DRAWTOP_SIZE2(graphicId, xOffset, yOffset);
	} else if (Data_Buildings[buildingId].houseSize) {
		if (Data_Buildings[buildingId].data.house.library) {
			drawOverlayColumn(
				Data_Buildings[buildingId].data.house.library / 10,
				xOffset, yOffset, 0);
		}
	}
}

static void drawBuildingTopForAcademyOverlay(int gridOffset, int buildingId, int xOffset, int yOffset)
{
	int graphicId = Data_Grid_graphicIds[gridOffset];
	if (Data_Buildings[buildingId].type == Building_Academy) {
		DRAWTOP_SIZE3(graphicId, xOffset, yOffset);
	} else if (Data_Buildings[buildingId].houseSize) {
		if (Data_Buildings[buildingId].data.house.academy) {
			drawOverlayColumn(
				Data_Buildings[buildingId].data.house.academy / 10,
				xOffset, yOffset, 0);
		}
	}
}

static void drawBuildingTopForBarberOverlay(int gridOffset, int buildingId, int xOffset, int yOffset)
{
	int graphicId = Data_Grid_graphicIds[gridOffset];
	if (Data_Buildings[buildingId].type == Building_Barber) {
		DRAWTOP_SIZE1(graphicId, xOffset, yOffset);
	} else if (Data_Buildings[buildingId].houseSize) {
		if (Data_Buildings[buildingId].data.house.barber) {
			drawOverlayColumn(
				Data_Buildings[buildingId].data.house.barber / 10,
				xOffset, yOffset, 0);
		}
	}
}

static void drawBuildingTopForClinicsOverlay(int gridOffset, int buildingId, int xOffset, int yOffset)
{
	int graphicId = Data_Grid_graphicIds[gridOffset];
	if (Data_Buildings[buildingId].type == Building_Doctor) {
		DRAWTOP_SIZE1(graphicId, xOffset, yOffset);
	} else if (Data_Buildings[buildingId].houseSize) {
		if (Data_Buildings[buildingId].data.house.clinic) {
			drawOverlayColumn(
				Data_Buildings[buildingId].data.house.clinic / 10,
				xOffset, yOffset, 0);
		}
	}
}

static void drawBuildingTopForHospitalOverlay(int gridOffset, int buildingId, int xOffset, int yOffset)
{
	int graphicId = Data_Grid_graphicIds[gridOffset];
	if (Data_Buildings[buildingId].type == Building_Hospital) {
		DRAWTOP_SIZE3(graphicId, xOffset, yOffset);
	} else if (Data_Buildings[buildingId].houseSize) {
		if (Data_Buildings[buildingId].data.house.hospital) {
			drawOverlayColumn(
				Data_Buildings[buildingId].data.house.hospital / 10,
				xOffset, yOffset, 0);
		}
	}
}

static void drawBuildingTopForTaxIncomeOverlay(int gridOffset, int buildingId, int xOffset, int yOffset)
{
	int graphicId = Data_Grid_graphicIds[gridOffset];
	if (Data_Buildings[buildingId].type == Building_SenateUpgraded) {
		DRAWTOP_SIZE5(graphicId, xOffset, yOffset);
	} else if (Data_Buildings[buildingId].type == Building_Forum) {
		DRAWTOP_SIZE2(graphicId, xOffset, yOffset);
	} else if (Data_Buildings[buildingId].houseSize) {
		int pct = Calc_adjustWithPercentage(
			Data_Buildings[buildingId].taxIncomeOrStorage / 2,
			Data_CityInfo.taxPercentage);
		if (pct > 0) {
			drawOverlayColumn(pct / 25, xOffset, yOffset, 0);
		}
	}
}

static void drawBuildingTopForProblemsOverlay(int gridOffset, int buildingId, int xOffset, int yOffset)
{
	if (Data_Buildings[buildingId].houseSize) {
		return;
	}
	int type = Data_Buildings[buildingId].type;
	if (type == Building_Fountain || type == Building_Bathhouse) {
		if (!Data_Buildings[buildingId].hasWaterAccess) {
			Data_Buildings[buildingId].showOnProblemOverlay = 1;
		}
	} else if (type >= Building_WheatFarm && type <= Building_ClayPit) {
		int walkerId = Data_Buildings[buildingId].walkerId;
		if (walkerId &&
			Data_Walkers[walkerId].actionState == WalkerActionState_20_CartPusher &&
			Data_Walkers[walkerId].minMaxSeen) {
			Data_Buildings[buildingId].showOnProblemOverlay = 1;
		}
	} else if (type >= Building_WineWorkshop && type <= Building_PotteryWorkshop) {
		int walkerId = Data_Buildings[buildingId].walkerId;
		if (walkerId &&
			Data_Walkers[walkerId].actionState == WalkerActionState_20_CartPusher &&
			Data_Walkers[walkerId].minMaxSeen) {
			Data_Buildings[buildingId].showOnProblemOverlay = 1;
		} else if (Data_Buildings[buildingId].rawMaterialsStored <= 0) {
			Data_Buildings[buildingId].showOnProblemOverlay = 1;
		}
	}

	if (Data_Buildings[buildingId].showOnProblemOverlay <= 0) {
		return;
	}

	if (type >= Building_WheatFarm && type <= Building_PigFarm) {
		int isField = 0;
		int edge = Data_Grid_edge[gridOffset];
		if ((Data_Settings_Map.orientation == Direction_Top && edge != 0x48) ||
			(Data_Settings_Map.orientation == Direction_Right && edge != 0x40) ||
			(Data_Settings_Map.orientation == Direction_Bottom && edge != 0x41) ||
			(Data_Settings_Map.orientation == Direction_Left && edge != 0x49)) {
			isField = 1;
		}
		if (isField) {
			if (edge & Edge_LeftmostTile) {
				DRAWTOP_SIZE1(Data_Grid_graphicIds[gridOffset], xOffset, yOffset);
			}
		} else { // farmhouse
			DRAWTOP_SIZE2(Data_Grid_graphicIds[gridOffset], xOffset, yOffset);
		}
		return;
	}
	if (type == Building_Granary) {
		int graphicId = Data_Grid_graphicIds[gridOffset];
		Graphics_drawImage(GraphicId(ID_Graphic_Granary) + 1,
			xOffset + GraphicSpriteOffsetX(graphicId),
			yOffset + GraphicSpriteOffsetY(graphicId) - 30 -
			(GraphicHeight(graphicId) - 90));
		if (Data_Buildings[buildingId].data.granary.spaceLeft < 2400) {
			Graphics_drawImage(GraphicId(ID_Graphic_Granary) + 2,
				xOffset + 32, yOffset - 61);
			if (Data_Buildings[buildingId].data.granary.spaceLeft < 1800) {
				Graphics_drawImage(GraphicId(ID_Graphic_Granary) + 3,
					xOffset + 56, yOffset - 51);
			}
			if (Data_Buildings[buildingId].data.granary.spaceLeft < 1200) {
				Graphics_drawImage(GraphicId(ID_Graphic_Granary) + 4,
					xOffset + 91, yOffset - 51);
			}
			if (Data_Buildings[buildingId].data.granary.spaceLeft < 600) {
				Graphics_drawImage(GraphicId(ID_Graphic_Granary) + 5,
					xOffset + 118, yOffset - 61);
			}
		}
	}
	if (type == Building_Warehouse) {
		Graphics_drawImage(GraphicId(ID_Graphic_Warehouse) + 17, xOffset - 4, yOffset - 42);
	}

	int graphicId = Data_Grid_graphicIds[gridOffset];
	switch (Data_Grid_bitfields[gridOffset] & Bitfield_Sizes) {
		case Bitfield_Size1:
			DRAWTOP_SIZE1(graphicId, xOffset, yOffset);
			break;
		case Bitfield_Size2:
			DRAWTOP_SIZE2(graphicId, xOffset, yOffset);
			break;
		case Bitfield_Size3:
			DRAWTOP_SIZE3(graphicId, xOffset, yOffset);
			break;
		case Bitfield_Size4:
			DRAWTOP_SIZE4(graphicId, xOffset, yOffset);
			break;
		case Bitfield_Size5:
			DRAWTOP_SIZE5(graphicId, xOffset, yOffset);
			break;
	}
}

static void drawOverlayColumn(int height, int xOffset, int yOffset, int isRed)
{
	int graphicId = GraphicId(ID_Graphic_OverlayColumn);
	if (isRed) {
		graphicId += 9;
	}
	if (height > 10) {
		height = 10;
	}
	int capitalHeight = GraphicHeight(graphicId);
	// draw base
	Graphics_drawImage(graphicId + 2, xOffset + 9, yOffset - 8);
	for (int i = 1; i < height; i++) {
		Graphics_drawImage(graphicId + 1, xOffset + 17, yOffset - 8 - 10 * i + 13);
	}
	// top
	Graphics_drawImage(graphicId, xOffset + 5, yOffset - 8 - capitalHeight + 13);
}

static void drawWalker(int walkerId, int xOffset, int yOffset, int selectedWalkerId)
{
	// TODO
}

static void drawBridge(int gridOffset, int xOffset, int yOffset)
{
	// TODO
}

#include "CityBuildings_private.h"

#include "../Building.h"
#include "../CityView.h"
#include "../Sound.h"
#include "../Time.h"
#include "../Data/Mouse.h"

#include <cstdio>

static void drawBuildingFootprints();
static void drawBuildingTopsWalkersAnimation(int selectedWalkerId);

static int lastWaterAnimationTime = 0;
static int advanceWaterAnimation;

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

	advanceWaterAnimation = 0;
	int now = Time_getMillis();
	if (now - lastWaterAnimationTime > 60 || now - lastWaterAnimationTime < 0) {
		lastWaterAnimationTime = now;
		advanceWaterAnimation = 1;
	}

	// TODO overlay and stuff
	//Data_State.currentOverlay = Overlay_Fire;
	if (Data_State.currentOverlay) {
		drawOverlayFootprints();
		drawOverlayTopsWalkersAnimation(Data_State.currentOverlay);
		drawSelectedBuildingGhost();
		//drawTops2(9999);
	} else {
		drawBuildingFootprints();
		drawBuildingTopsWalkersAnimation(0);
		drawSelectedBuildingGhost();
		//drawTops2(0);
	}

	Graphics_resetClipRectangle();
}

static void drawBuildingFootprints()
{
	int graphicIdWaterFirst = GraphicId(ID_Graphic_TerrainWater);
	int graphicIdWaterLast = 5 + graphicIdWaterFirst;

	FOREACH_XY_VIEW({
		int gridOffset = ViewToGridOffset(xView, yView);
		// TODO something related to elevation/draggable reservoir?
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
					colorMask = Color_MaskRed;
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
					if (advanceWaterAnimation &&
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
					if (b->loadsStored >= 2 || b->data.industry.hasFullResource) {
						Graphics_drawImageMasked(GraphicId(ID_Graphic_WorkshopRawMaterial),
							xGraphic + 45, yGraphic + 23, colorMask);
					}
				}
				if (b->type == Building_OilWorkshop) {
					if (b->loadsStored >= 2 || b->data.industry.hasFullResource) {
						Graphics_drawImageMasked(GraphicId(ID_Graphic_WorkshopRawMaterial) + 1,
							xGraphic + 35, yGraphic + 15, colorMask);
					}
				}
				if (b->type == Building_WeaponsWorkshop) {
					if (b->loadsStored >= 2 || b->data.industry.hasFullResource) {
						Graphics_drawImageMasked(GraphicId(ID_Graphic_WorkshopRawMaterial) + 3,
							xGraphic + 46, yGraphic + 24, colorMask);
					}
				}
				if (b->type == Building_FurnitureWorkshop) {
					if (b->loadsStored >= 2 || b->data.industry.hasFullResource) {
						Graphics_drawImageMasked(GraphicId(ID_Graphic_WorkshopRawMaterial) + 2,
							xGraphic + 48, yGraphic + 19, colorMask);
					}
				}
				if (b->type == Building_PotteryWorkshop) {
					if (b->loadsStored >= 2 || b->data.industry.hasFullResource) {
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
						if (b->data.storage.resourceStored[Resource_None] < 2400) {
							Graphics_drawImageMasked(GraphicId(ID_Graphic_Granary) + 2,
								xGraphic + 33, yGraphic - 60, colorMask);
						}
						if (b->data.storage.resourceStored[Resource_None] < 1800) {
							Graphics_drawImageMasked(GraphicId(ID_Graphic_Granary) + 3,
								xGraphic + 56, yGraphic - 50, colorMask);
						}
						if (b->data.storage.resourceStored[Resource_None] < 1200) {
							Graphics_drawImageMasked(GraphicId(ID_Graphic_Granary) + 4,
								xGraphic + 91, yGraphic - 50, colorMask);
						}
						if (b->data.storage.resourceStored[Resource_None] < 600) {
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


void drawWalker(int walkerId, int xOffset, int yOffset, int selectedWalkerId)
{
	// TODO
}

void drawBridge(int gridOffset, int x, int y)
{
	if (!(Data_Grid_terrain[gridOffset] & Terrain_Water)) {
		Data_Grid_spriteOffsets[gridOffset] = 0;
		return;
	}
	if (Data_Grid_terrain[gridOffset] & Terrain_Building) {
		return;
	}
	Color colorMask = 0;
	if (Data_Grid_bitfields[gridOffset] & Bitfield_Deleted) {
		colorMask = Color_MaskRed;
	}
	int graphicId = GraphicId(ID_Graphic_Bridge);
	switch (Data_Grid_spriteOffsets[gridOffset]) {
		case 1:
			Graphics_drawImageMasked(graphicId + 5, x, y - 20, colorMask);
			break;
		case 2:
			Graphics_drawImageMasked(graphicId, x - 1, y - 8, colorMask);
			break;
		case 3:
			Graphics_drawImageMasked(graphicId + 3, x, y - 8, colorMask);
			break;
		case 4:
			Graphics_drawImageMasked(graphicId + 2, x + 7, y - 20, colorMask);
			break;
		case 5:
			Graphics_drawImageMasked(graphicId + 4, x , y - 21, colorMask);
			break;
		case 6:
			Graphics_drawImageMasked(graphicId + 1, x + 5, y - 21, colorMask);
			break;
		case 7:
			Graphics_drawImageMasked(graphicId + 11, x - 3, y - 50, colorMask);
			break;
		case 8:
			Graphics_drawImageMasked(graphicId + 6, x - 1, y - 12, colorMask);
			break;
		case 9:
			Graphics_drawImageMasked(graphicId + 9, x - 30, y - 12, colorMask);
			break;
		case 10:
			Graphics_drawImageMasked(graphicId + 8, x - 23, y - 53, colorMask);
			break;
		case 11:
			Graphics_drawImageMasked(graphicId + 10, x, y - 37, colorMask);
			break;
		case 12:
			Graphics_drawImageMasked(graphicId + 7, x + 7, y - 38, colorMask);
			break;
			// Note: no nr 13
		case 14:
			Graphics_drawImageMasked(graphicId + 13, x, y - 38, colorMask);
			break;
		case 15:
			Graphics_drawImageMasked(graphicId + 12, x + 7, y - 38, colorMask);
			break;
	}
}

// MOUSE HANDLING

static void updateCityViewCoords()
{
	Data_Settings_Map.x = Data_Settings_Map.y = 0;
	int gridOffset = CityView_pixelCoordsToGridOffset(Data_Mouse.x, Data_Mouse.y);
	if (gridOffset) {
		Data_Settings_Map.x = (gridOffset - Data_Settings_Map.gridStartOffset) % 162;
		Data_Settings_Map.y = (gridOffset - Data_Settings_Map.gridStartOffset) / 162;
	}
}

void UI_CityBuildings_handleMouse()
{
	updateCityViewCoords();
}

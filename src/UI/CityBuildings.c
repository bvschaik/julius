#include "CityBuildings_private.h"

#include "Minimap.h"
#include "Warning.h"
#include "Window.h"

#include "../Building.h"
#include "../BuildingPlacement.h"
#include "../CityView.h"
#include "../Formation.h"
#include "../Routing.h"
#include "../Scroll.h"
#include "../Sound.h"
#include "../Undo.h"
#include "../Widget.h"

#include "building/model.h"
#include "core/time.h"
#include "figure/formation.h"

static void drawBuildingFootprints();
static void drawBuildingTopsFiguresAnimation(int selectedFigureId, struct UI_CityPixelCoordinate *coord);
static void drawHippodromeAndElevatedFigures(int selectedFigureId);

static time_millis lastWaterAnimationTime = 0;
static int advanceWaterAnimation;

void UI_CityBuildings_drawForeground(int x, int y)
{
	Data_CityView.xInTiles = x;
	Data_CityView.yInTiles = y;
	Graphics_setClipRectangle(
		Data_CityView.xOffsetInPixels, Data_CityView.yOffsetInPixels,
		Data_CityView.widthInPixels, Data_CityView.heightInPixels);

	advanceWaterAnimation = 0;
	time_millis now = time_get_millis();
	if (now - lastWaterAnimationTime > 60 || now < lastWaterAnimationTime) {
		lastWaterAnimationTime = now;
		advanceWaterAnimation = 1;
	}

	if (Data_State.currentOverlay) {
		UI_CityBuildings_drawOverlayFootprints();
		UI_CityBuildings_drawOverlayTopsFiguresAnimation(Data_State.currentOverlay);
		UI_CityBuildings_drawSelectedBuildingGhost();
		drawHippodromeAndElevatedFigures(9999);
	} else {
		drawBuildingFootprints();
		drawBuildingTopsFiguresAnimation(0, 0);
		UI_CityBuildings_drawSelectedBuildingGhost();
		drawHippodromeAndElevatedFigures(0);
	}

	Graphics_resetClipRectangle();
}

void UI_CityBuildings_drawBuildingCost()
{
	if (!Data_Settings_Map.current.gridOffset) {
		return;
	}
	if (Data_State.isScrollingMap) {
		return;
	}
	if (!Data_State.selectedBuilding.cost) {
		return;
	}
	Graphics_setClipRectangle(
		Data_CityView.xOffsetInPixels, Data_CityView.yOffsetInPixels,
		Data_CityView.widthInPixels, Data_CityView.heightInPixels);
	color_t color;
	if (Data_State.selectedBuilding.cost <= Data_CityInfo.treasury) {
		color = COLOR_ORANGE;
	} else {
		color = COLOR_RED;
	}
	Widget_Text_drawNumberColored(Data_State.selectedBuilding.cost, '@', " ",
		Data_CityView.selectedTile.xOffsetInPixels + 58 + 1,
		Data_CityView.selectedTile.yOffsetInPixels + 1, FONT_NORMAL_PLAIN, COLOR_BLACK);
	Widget_Text_drawNumberColored(Data_State.selectedBuilding.cost, '@', " ",
		Data_CityView.selectedTile.xOffsetInPixels + 58,
		Data_CityView.selectedTile.yOffsetInPixels, FONT_NORMAL_PLAIN, color);
	Graphics_resetClipRectangle();
	Data_State.selectedBuilding.cost = 0;
}

void UI_CityBuildings_drawForegroundForFigure(int x, int y, int figureId, struct UI_CityPixelCoordinate *coord)
{
	Data_CityView.xInTiles = x;
	Data_CityView.yInTiles = y;
	Graphics_setClipRectangle(
		Data_CityView.xOffsetInPixels, Data_CityView.yOffsetInPixels,
		Data_CityView.widthInPixels, Data_CityView.heightInPixels);

	drawBuildingFootprints();
	drawBuildingTopsFiguresAnimation(figureId, coord);
	drawHippodromeAndElevatedFigures(0);

	Graphics_resetClipRectangle();
}

static void drawBuildingFootprints()
{
	int graphicIdWaterFirst = image_group(ID_Graphic_TerrainWater);
	int graphicIdWaterLast = 5 + graphicIdWaterFirst;

	FOREACH_XY_VIEW {
		int gridOffset = ViewToGridOffset(xView, yView);
		if (gridOffset == Data_State.selectedBuilding.gridOffsetStart) {
			Data_State.selectedBuilding.reservoirOffsetX = xGraphic;
			Data_State.selectedBuilding.reservoirOffsetY = yGraphic;
		}
		if (gridOffset < 0) {
			// Outside map: draw black tile
			Graphics_drawIsometricFootprint(image_group(ID_Graphic_TerrainBlack),
				xGraphic, yGraphic, 0);
		} else if (Data_Grid_edge[gridOffset] & Edge_LeftmostTile) {
			// Valid gridOffset and leftmost tile -> draw
			int buildingId = Data_Grid_buildingIds[gridOffset];
			color_t colorMask = 0;
			if (buildingId) {
				if (Data_Buildings[buildingId].isDeleted) {
					colorMask = COLOR_MASK_RED;
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
				graphicId = image_group(ID_Graphic_TerrainOverlay);
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
	} END_FOREACH_XY_VIEW;
}

static void drawBuildingTopsFiguresAnimation(int selectedFigureId, struct UI_CityPixelCoordinate *coord)
{
	FOREACH_Y_VIEW {
		FOREACH_X_VIEW {
			if (Data_Grid_edge[gridOffset] & Edge_LeftmostTile) {
				int buildingId = Data_Grid_buildingIds[gridOffset];
				int graphicId = Data_Grid_graphicIds[gridOffset];
				color_t colorMask = 0;
				if (buildingId && Data_Buildings[buildingId].isDeleted) {
					colorMask = COLOR_MASK_RED;
				}
				switch (Data_Grid_bitfields[gridOffset] & Bitfield_Sizes) {
					case Bitfield_Size1: DRAWTOP_SIZE1_C(graphicId, xGraphic, yGraphic, colorMask); break;
					case Bitfield_Size2: DRAWTOP_SIZE2_C(graphicId, xGraphic, yGraphic, colorMask); break;
					case Bitfield_Size3: DRAWTOP_SIZE3_C(graphicId, xGraphic, yGraphic, colorMask); break;
					case Bitfield_Size4: DRAWTOP_SIZE4_C(graphicId, xGraphic, yGraphic, colorMask); break;
					case Bitfield_Size5: DRAWTOP_SIZE5_C(graphicId, xGraphic, yGraphic, colorMask); break;
				}
				// specific buildings
				struct Data_Building *b = &Data_Buildings[buildingId];
				if (b->type == BUILDING_SENATE_UPGRADED) {
					// rating flags
					graphicId = image_group(ID_Graphic_Senate);
					Graphics_drawImageMasked(graphicId + 1, xGraphic + 138,
						yGraphic + 44 - Data_CityInfo.ratingCulture / 2, colorMask);
					Graphics_drawImageMasked(graphicId + 2, xGraphic + 168,
						yGraphic + 36 - Data_CityInfo.ratingProsperity / 2, colorMask);
					Graphics_drawImageMasked(graphicId + 3, xGraphic + 198,
						yGraphic + 27 - Data_CityInfo.ratingPeace / 2, colorMask);
					Graphics_drawImageMasked(graphicId + 4, xGraphic + 228,
						yGraphic + 19 - Data_CityInfo.ratingFavor / 2, colorMask);
					// unemployed
					graphicId = image_group(ID_Graphic_Figure_Homeless);
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
				if (b->type == BUILDING_AMPHITHEATER && b->numWorkers > 0) {
					Graphics_drawImageMasked(image_group(ID_Graphic_AmphitheaterShow),
						xGraphic + 36, yGraphic - 47, colorMask);
				}
				if (b->type == BUILDING_THEATER && b->numWorkers > 0) {
					Graphics_drawImageMasked(image_group(ID_Graphic_TheaterShow),
						xGraphic + 34, yGraphic - 22, colorMask);
				}
				if (b->type == BUILDING_HIPPODROME &&
					Data_Buildings[Building_getMainBuildingId(buildingId)].numWorkers > 0 &&
					Data_CityInfo.entertainmentHippodromeHasShow) {
					int subtype = b->subtype.orientation;
					if ((subtype == 0 || subtype == 3) && Data_CityInfo.population > 2000) {
						switch (Data_Settings_Map.orientation) {
							case Dir_0_Top:
								Graphics_drawImageMasked(
									image_group(ID_Graphic_Hippodrome2) + 6,
									xGraphic + 147, yGraphic - 72, colorMask);
								break;
							case Dir_2_Right:
								Graphics_drawImageMasked(
									image_group(ID_Graphic_Hippodrome1) + 8,
									xGraphic + 58, yGraphic - 79, colorMask);
								break;
							case Dir_4_Bottom:
								Graphics_drawImageMasked(
									image_group(ID_Graphic_Hippodrome2) + 8,
									xGraphic + 119, yGraphic - 80, colorMask);
								break;
							case Dir_6_Left:
								Graphics_drawImageMasked(
									image_group(ID_Graphic_Hippodrome1) + 6,
									xGraphic, yGraphic - 72, colorMask);
						}
					} else if ((subtype == 1 || subtype == 4) && Data_CityInfo.population > 100) {
						switch (Data_Settings_Map.orientation) {
							case Dir_0_Top:
							case Dir_4_Bottom:
								Graphics_drawImageMasked(
									image_group(ID_Graphic_Hippodrome2) + 7,
									xGraphic + 122, yGraphic - 79, colorMask);
								break;
							case Dir_2_Right:
							case Dir_6_Left:
								Graphics_drawImageMasked(
									image_group(ID_Graphic_Hippodrome1) + 7,
									xGraphic, yGraphic - 80, colorMask);
						}
					} else if ((subtype == 2 || subtype == 5) && Data_CityInfo.population > 1000) {
						switch (Data_Settings_Map.orientation) {
							case Dir_0_Top:
								Graphics_drawImageMasked(
									image_group(ID_Graphic_Hippodrome2) + 8,
									xGraphic + 119, yGraphic - 80, colorMask);
								break;
							case Dir_2_Right:
								Graphics_drawImageMasked(
									image_group(ID_Graphic_Hippodrome1) + 6,
									xGraphic, yGraphic - 72, colorMask);
								break;
							case Dir_4_Bottom:
								Graphics_drawImageMasked(
									image_group(ID_Graphic_Hippodrome2) + 6,
									xGraphic + 147, yGraphic - 72, colorMask);
								break;
							case Dir_6_Left:
								Graphics_drawImageMasked(
									image_group(ID_Graphic_Hippodrome1) + 8,
									xGraphic + 58, yGraphic - 79, colorMask);
						}
					}
				}
				if (b->type == BUILDING_COLOSSEUM && b->numWorkers > 0) {
					Graphics_drawImageMasked(image_group(ID_Graphic_ColosseumShow),
						xGraphic + 70, yGraphic - 90, colorMask);
				}
				// workshops
				if (b->type == BUILDING_WINE_WORKSHOP) {
					if (b->loadsStored >= 2 || b->data.industry.hasFullResource) {
						Graphics_drawImageMasked(image_group(ID_Graphic_WorkshopRawMaterial),
							xGraphic + 45, yGraphic + 23, colorMask);
					}
				}
				if (b->type == BUILDING_OIL_WORKSHOP) {
					if (b->loadsStored >= 2 || b->data.industry.hasFullResource) {
						Graphics_drawImageMasked(image_group(ID_Graphic_WorkshopRawMaterial) + 1,
							xGraphic + 35, yGraphic + 15, colorMask);
					}
				}
				if (b->type == BUILDING_WEAPONS_WORKSHOP) {
					if (b->loadsStored >= 2 || b->data.industry.hasFullResource) {
						Graphics_drawImageMasked(image_group(ID_Graphic_WorkshopRawMaterial) + 3,
							xGraphic + 46, yGraphic + 24, colorMask);
					}
				}
				if (b->type == BUILDING_FURNITURE_WORKSHOP) {
					if (b->loadsStored >= 2 || b->data.industry.hasFullResource) {
						Graphics_drawImageMasked(image_group(ID_Graphic_WorkshopRawMaterial) + 2,
							xGraphic + 48, yGraphic + 19, colorMask);
					}
				}
				if (b->type == BUILDING_POTTERY_WORKSHOP) {
					if (b->loadsStored >= 2 || b->data.industry.hasFullResource) {
						Graphics_drawImageMasked(image_group(ID_Graphic_WorkshopRawMaterial) + 4,
							xGraphic + 47, yGraphic + 24, colorMask);
					}
				}
			}
		} END_FOREACH_X_VIEW;
		// draw figures
		FOREACH_X_VIEW {
			int figureId = Data_Grid_figureIds[gridOffset];
			while (figureId) {
				if (!Data_Figures[figureId].isGhost) {
					UI_CityBuildings_drawFigure(figureId, xGraphic, yGraphic, selectedFigureId, coord);
				}
				figureId = Data_Figures[figureId].nextFigureIdOnSameTile;
			}
		} END_FOREACH_X_VIEW;
		// draw animation
		FOREACH_X_VIEW {
			int graphicId = Data_Grid_graphicIds[gridOffset];
			const image *img = image_get(graphicId);
			if (img->num_animation_sprites) {
				if (Data_Grid_edge[gridOffset] & Edge_LeftmostTile) {
					int buildingId = Data_Grid_buildingIds[gridOffset];
					struct Data_Building *b = &Data_Buildings[buildingId];
					int colorMask = 0;
					if (buildingId && b->isDeleted) {
						colorMask = COLOR_MASK_RED;
					}
					if (b->type == BUILDING_DOCK) {
						int numDockers = BUILDING_DOCK_getNumIdleDockers(buildingId);
						if (numDockers > 0) {
							int graphicIdDock = Data_Grid_graphicIds[b->gridOffset];
							int graphicIdDockers = image_group(ID_Graphic_Dockers);
							if (graphicIdDock == image_group(ID_Graphic_Dock1)) {
								graphicIdDockers += 0;
							} else if (graphicIdDock == image_group(ID_Graphic_Dock2)) {
								graphicIdDockers += 3;
							} else if (graphicIdDock == image_group(ID_Graphic_Dock3)) {
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
								xGraphic + image_get(graphicIdDockers)->sprite_offset_x,
								yGraphic + image_get(graphicIdDockers)->sprite_offset_y,
								colorMask);
						}
					}
					if (b->type == BUILDING_WAREHOUSE) {
						Graphics_drawImageMasked(image_group(ID_Graphic_Warehouse) + 17,
							xGraphic - 4, yGraphic - 42, colorMask);
						if (buildingId == Data_CityInfo.buildingTradeCenterBuildingId) {
							Graphics_drawImageMasked(image_group(ID_Graphic_TradeCenterFlag),
								xGraphic + 19, yGraphic - 56, colorMask);
						}
					}
					if (b->type == BUILDING_GRANARY) {
						Graphics_drawImageMasked(image_group(ID_Graphic_Granary) + 1,
							xGraphic + img->sprite_offset_x,
							yGraphic + 60 + img->sprite_offset_y - img->height,
							colorMask);
						if (b->data.storage.resourceStored[Resource_None] < 2400) {
							Graphics_drawImageMasked(image_group(ID_Graphic_Granary) + 2,
								xGraphic + 33, yGraphic - 60, colorMask);
						}
						if (b->data.storage.resourceStored[Resource_None] < 1800) {
							Graphics_drawImageMasked(image_group(ID_Graphic_Granary) + 3,
								xGraphic + 56, yGraphic - 50, colorMask);
						}
						if (b->data.storage.resourceStored[Resource_None] < 1200) {
							Graphics_drawImageMasked(image_group(ID_Graphic_Granary) + 4,
								xGraphic + 91, yGraphic - 50, colorMask);
						}
						if (b->data.storage.resourceStored[Resource_None] < 600) {
							Graphics_drawImageMasked(image_group(ID_Graphic_Granary) + 5,
								xGraphic + 117, yGraphic - 62, colorMask);
						}
					}
					if (b->type == BUILDING_BURNING_RUIN && b->ruinHasPlague) {
						Graphics_drawImageMasked(image_group(ID_Graphic_PlagueSkull),
							xGraphic + 18, yGraphic - 32, colorMask);
					}
					int animationOffset = Animation_getIndexForCityBuilding(graphicId, gridOffset);
					if (b->type != BUILDING_HIPPODROME && animationOffset > 0) {
						if (animationOffset > img->num_animation_sprites) {
							animationOffset = img->num_animation_sprites;
						}
						if (b->type == BUILDING_GRANARY) {
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
								xGraphic + img->sprite_offset_x,
								yGraphic + ydiff + img->sprite_offset_y - img->height,
								colorMask);
						}
					}
				}
			} else if (Data_Grid_spriteOffsets[gridOffset]) {
				UI_CityBuildings_drawBridge(gridOffset, xGraphic, yGraphic);
			} else if (Data_Buildings[Data_Grid_buildingIds[gridOffset]].type == BUILDING_FORT) {
				if (Data_Grid_edge[gridOffset] & Edge_LeftmostTile) {
					int buildingId = Data_Grid_buildingIds[gridOffset];
					int offset = 0;
					switch (Data_Buildings[buildingId].subtype.fortFigureType) {
						case FIGURE_FORT_LEGIONARY: offset = 4; break;
						case FIGURE_FORT_MOUNTED: offset = 3; break;
						case FIGURE_FORT_JAVELIN: offset = 2; break;
					}
					if (offset) {
						Graphics_drawImage(image_group(ID_Graphic_Fort) + offset,
							xGraphic + 81, yGraphic + 5);
					}
				}
			} else if (Data_Buildings[Data_Grid_buildingIds[gridOffset]].type == BUILDING_GATEHOUSE) {
				int xy = Data_Grid_edge[gridOffset] & Edge_MaskXY;
				if ((Data_Settings_Map.orientation == Dir_0_Top && xy == 9) ||
					(Data_Settings_Map.orientation == Dir_2_Right && xy == 8) ||
					(Data_Settings_Map.orientation == Dir_4_Bottom && xy == 0) ||
					(Data_Settings_Map.orientation == Dir_6_Left && xy == 1)) {
					int buildingId = Data_Grid_buildingIds[gridOffset];
					int graphicId = image_group(ID_Graphic_Gatehouse);
					if (Data_Buildings[buildingId].subtype.orientation == 1) {
						if (Data_Settings_Map.orientation == Dir_0_Top || Data_Settings_Map.orientation == Dir_4_Bottom) {
							Graphics_drawImage(graphicId, xGraphic - 22, yGraphic - 80);
						} else {
							Graphics_drawImage(graphicId + 1, xGraphic - 18, yGraphic - 81);
						}
					} else if (Data_Buildings[buildingId].subtype.orientation == 2) {
						if (Data_Settings_Map.orientation == Dir_0_Top || Data_Settings_Map.orientation == Dir_4_Bottom) {
							Graphics_drawImage(graphicId + 1, xGraphic - 18, yGraphic - 81);
						} else {
							Graphics_drawImage(graphicId, xGraphic - 22, yGraphic - 80);
						}
					}
				}
			}
		} END_FOREACH_X_VIEW;
	} END_FOREACH_Y_VIEW;
}

void UI_CityBuildings_drawBridge(int gridOffset, int x, int y)
{
	if (!(Data_Grid_terrain[gridOffset] & Terrain_Water)) {
		Data_Grid_spriteOffsets[gridOffset] = 0;
		return;
	}
	if (Data_Grid_terrain[gridOffset] & Terrain_Building) {
		return;
	}
	color_t colorMask = 0;
	if (Data_Grid_bitfields[gridOffset] & Bitfield_Deleted) {
		colorMask = COLOR_MASK_RED;
	}
	int graphicId = image_group(ID_Graphic_Bridge);
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

static void drawHippodromeAndElevatedFigures(int selectedFigureId)
{
	FOREACH_Y_VIEW {
		FOREACH_X_VIEW {
			for (int figureId = Data_Grid_figureIds[gridOffset]; figureId > 0; figureId = Data_Figures[figureId].nextFigureIdOnSameTile) {
				if (Data_Figures[figureId].useCrossCountry && !Data_Figures[figureId].isGhost) {
					UI_CityBuildings_drawFigure(figureId, xGraphic, yGraphic, selectedFigureId, 0);
				}
				if (Data_Figures[figureId].heightAdjustedTicks) {
					UI_CityBuildings_drawFigure(figureId, xGraphic, yGraphic, selectedFigureId, 0);
				}
			}
		} END_FOREACH_X_VIEW;
		FOREACH_X_VIEW {
			if (!Data_State.currentOverlay) {
				int graphicId = Data_Grid_graphicIds[gridOffset];
				const image *img = image_get(graphicId);
				if (img->num_animation_sprites &&
					Data_Grid_edge[gridOffset] & Edge_LeftmostTile &&
					Data_Buildings[Data_Grid_buildingIds[gridOffset]].type == BUILDING_HIPPODROME) {
					switch (Data_Grid_bitfields[gridOffset] & Bitfield_Sizes) {
						case Bitfield_Size1:
							Graphics_drawImage(graphicId + 1,
								xGraphic + img->sprite_offset_x,
								yGraphic + img->sprite_offset_y - img->height + 30);
							break;
						case Bitfield_Size2:
							Graphics_drawImage(graphicId + 1,
								xGraphic + img->sprite_offset_x,
								yGraphic + img->sprite_offset_y - img->height + 45);
							break;
						case Bitfield_Size3:
							Graphics_drawImage(graphicId + 1,
								xGraphic + img->sprite_offset_x,
								yGraphic + img->sprite_offset_y - img->height + 60);
							break;
						case Bitfield_Size4:
							Graphics_drawImage(graphicId + 1,
								xGraphic + img->sprite_offset_x,
								yGraphic + img->sprite_offset_y - img->height + 75);
							break;
						case Bitfield_Size5:
							Graphics_drawImage(graphicId + 1,
								xGraphic + img->sprite_offset_x,
								yGraphic + img->sprite_offset_y - img->height + 90);
							break;
					}
				}
			}
		} END_FOREACH_X_VIEW;
	} END_FOREACH_Y_VIEW;
}

// MOUSE HANDLING

static void updateCityViewCoords(const mouse *m)
{
	Data_Settings_Map.current.x = Data_Settings_Map.current.y = 0;
	int gridOffset = Data_Settings_Map.current.gridOffset =
		CityView_pixelCoordsToGridOffset(m->x, m->y);
	if (gridOffset) {
		Data_Settings_Map.current.x = (gridOffset - Data_Settings_Map.gridStartOffset) % 162;
		Data_Settings_Map.current.y = (gridOffset - Data_Settings_Map.gridStartOffset) / 162;
	}
}

static int handleRightClickAllowBuildingInfo()
{
	int allow = 1;
	if (UI_Window_getId() != Window_City) {
		allow = 0;
	}
	if (Data_State.selectedBuilding.type) {
		allow = 0;
	}
	Data_State.selectedBuilding.type = 0;
	Data_State.selectedBuilding.placementInProgress = 0;
	Data_State.selectedBuilding.xStart = 0;
	Data_State.selectedBuilding.yStart = 0;
	Data_State.selectedBuilding.xEnd = 0;
	Data_State.selectedBuilding.yEnd = 0;
	UI_Window_goTo(Window_City);

	if (!Data_Settings_Map.current.gridOffset) {
		allow = 0;
	}
	if (UI_Warning_hasWarnings()) {
		UI_Warning_clearAll();
		allow = 0;
	}
	return allow;
}

static int isLegionClick()
{
	if (Data_Settings_Map.current.gridOffset) {
		int formationId = Formation_getLegionFormationAtGridOffset(
			GridOffset(Data_Settings_Map.current.x, Data_Settings_Map.current.y));
		if (formationId > 0 && !formation_get(formationId)->in_distant_battle) {
			Data_State.selectedLegionFormationId = formationId;
			UI_Window_goTo(Window_CityMilitary);
			return 1;
		}
	}
	return 0;
}

static void buildStart()
{
	if (Data_Settings_Map.current.gridOffset /*&& !Data_Settings.gamePaused*/) { // TODO FIXME
		Data_State.selectedBuilding.xEnd = Data_State.selectedBuilding.xStart = Data_Settings_Map.current.x;
		Data_State.selectedBuilding.yEnd = Data_State.selectedBuilding.yStart = Data_Settings_Map.current.y;
		Data_State.selectedBuilding.gridOffsetStart = Data_Settings_Map.current.gridOffset;
		if (Undo_recordBeforeBuild()) {
			Data_State.selectedBuilding.placementInProgress = 1;
			switch (Data_State.selectedBuilding.type) {
				case BUILDING_ROAD:
					Routing_getDistanceForBuildingRoadOrAqueduct(
						Data_State.selectedBuilding.xStart,
						Data_State.selectedBuilding.yStart, 0);
					break;
				case BUILDING_AQUEDUCT:
				case BUILDING_DRAGGABLE_RESERVOIR:
					Routing_getDistanceForBuildingRoadOrAqueduct(
						Data_State.selectedBuilding.xStart,
						Data_State.selectedBuilding.yStart, 1);
					break;
				case BUILDING_WALL:
					Routing_getDistanceForBuildingWall(
						Data_State.selectedBuilding.xStart,
						Data_State.selectedBuilding.yStart);
					break;
			}
		}
	}
}

static void buildMove()
{
	if (!Data_State.selectedBuilding.placementInProgress ||
		!Data_Settings_Map.current.gridOffset) {
		return;
	}
	Data_State.selectedBuilding.gridOffsetEnd = Data_Settings_Map.current.gridOffset;
	Data_State.selectedBuilding.xEnd = Data_Settings_Map.current.x;
	Data_State.selectedBuilding.yEnd = Data_Settings_Map.current.y;
	BuildingPlacement_update(
		Data_State.selectedBuilding.xStart, Data_State.selectedBuilding.yStart,
		Data_State.selectedBuilding.xEnd, Data_State.selectedBuilding.yEnd,
		Data_State.selectedBuilding.type);
}

static void buildEnd()
{
	if (Data_State.selectedBuilding.placementInProgress) {
		Data_State.selectedBuilding.placementInProgress = 0;
		if (!Data_Settings_Map.current.gridOffset) {
			Data_Settings_Map.current.gridOffset = Data_State.selectedBuilding.gridOffsetEnd;
		}
		if (Data_State.selectedBuilding.type > 0) {
			Sound_Effects_playChannel(SoundChannel_Build);
		}
		BuildingPlacement_place(Data_Settings_Map.orientation,
			Data_State.selectedBuilding.xStart, Data_State.selectedBuilding.yStart,
			Data_State.selectedBuilding.xEnd, Data_State.selectedBuilding.yEnd,
			Data_State.selectedBuilding.type);
	}
}

void UI_CityBuildings_handleMouse(const mouse *m)
{
	UI_CityBuildings_scrollMap(Scroll_getDirection(m));
	updateCityViewCoords(m);
	Data_State.selectedBuilding.drawAsOverlay = 0;
	if (m->left.went_down) {
		if (!isLegionClick()) {
			buildStart();
		}
	} else if (m->left.is_down) {
		buildMove();
	} else if (m->left.went_up) {
		buildEnd();
	} else if (m->right.went_up) {
		if (handleRightClickAllowBuildingInfo()) {
			UI_Window_goTo(Window_BuildingInfo);
		}
	}
}

void UI_CityBuildings_getTooltip(struct TooltipContext *c)
{
	if (!Data_Settings.mouseTooltips || mouse_get()->right.is_down) {
		return;
	}
	if (UI_Window_getId() != Window_City) {
		return;
	}
	if (Data_Settings_Map.current.gridOffset == 0) {
		return;
	}
	int gridOffset = Data_Settings_Map.current.gridOffset;
	int buildingId = Data_Grid_buildingIds[gridOffset];
	int overlay = Data_State.currentOverlay;
	// regular tooltips
	if (overlay == Overlay_None && buildingId && Data_Buildings[buildingId].type == BUILDING_SENATE_UPGRADED) {
		c->type = TooltipType_Senate;
		c->priority = TooltipPriority_High;
		return;
	}
	// overlay tooltips
	if (overlay != Overlay_Water && overlay != Overlay_Desirability && !buildingId) {
		return;
	}
	int overlayRequiresHouse =
		overlay != Overlay_Water && overlay != Overlay_Fire &&
		overlay != Overlay_Damage && overlay != Overlay_Native;
	int overlayForbidsHouse = overlay == Overlay_Native;
	struct Data_Building *b = &Data_Buildings[buildingId];
	if (overlayRequiresHouse && !b->houseSize) {
		return;
	}
	if (overlayForbidsHouse && b->houseSize) {
		return;
	}
	c->textGroup = 66;
	c->textId = 0;
	c->hasNumericPrefix = 0;
	switch (overlay) {
		case Overlay_Water:
			if (Data_Grid_terrain[gridOffset] & Terrain_ReservoirRange) {
				if (Data_Grid_terrain[gridOffset] & Terrain_FountainRange) {
					c->textId = 2;
				} else {
					c->textId = 1;
				}
			} else if (Data_Grid_terrain[gridOffset] & Terrain_FountainRange) {
				c->textGroup = 66;
				c->textId = 3;
			} else {
				return;
			}
			break;
		case Overlay_Religion:
			if (b->data.house.numGods <= 0) {
				c->textId = 12;
			} else if (b->data.house.numGods == 1) {
				c->textId = 13;
			} else if (b->data.house.numGods == 2) {
				c->textId = 14;
			} else if (b->data.house.numGods == 3) {
				c->textId = 15;
			} else if (b->data.house.numGods == 4) {
				c->textId = 16;
			} else if (b->data.house.numGods == 5) {
				c->textId = 17;
			} else {
				c->textId = 18; // >5 gods, shouldn't happen...
			}
			break;
		case Overlay_Fire:
			if (b->fireRisk <= 0) {
				c->textId = 46;
			} else if (b->fireRisk <= 20) {
				c->textId = 47;
			} else if (b->fireRisk <= 40) {
				c->textId = 48;
			} else if (b->fireRisk <= 60) {
				c->textId = 49;
			} else if (b->fireRisk <= 80) {
				c->textId = 50;
			} else {
				c->textId = 51;
			}
			break;
		case Overlay_Damage:
			if (b->damageRisk <= 0) {
				c->textId = 52;
			} else if (b->damageRisk <= 40) {
				c->textId = 53;
			} else if (b->damageRisk <= 80) {
				c->textId = 54;
			} else if (b->damageRisk <= 120) {
				c->textId = 55;
			} else if (b->damageRisk <= 160) {
				c->textId = 56;
			} else {
				c->textId = 57;
			}
			break;
		case Overlay_Crime:
			if (b->sentiment.houseHappiness <= 0) {
				c->textId = 63;
			} else if (b->sentiment.houseHappiness <= 10) {
				c->textId = 62;
			} else if (b->sentiment.houseHappiness <= 20) {
				c->textId = 61;
			} else if (b->sentiment.houseHappiness <= 30) {
				c->textId = 60;
			} else if (b->sentiment.houseHappiness < 50) {
				c->textId = 59;
			} else {
				c->textId = 58;
			}
			break;
		case Overlay_Entertainment:
			if (b->data.house.entertainment <= 0) {
				c->textId = 64;
			} else if (b->data.house.entertainment < 10) {
				c->textId = 65;
			} else if (b->data.house.entertainment < 20) {
				c->textId = 66;
			} else if (b->data.house.entertainment < 30) {
				c->textId = 67;
			} else if (b->data.house.entertainment < 40) {
				c->textId = 68;
			} else if (b->data.house.entertainment < 50) {
				c->textId = 69;
			} else if (b->data.house.entertainment < 60) {
				c->textId = 70;
			} else if (b->data.house.entertainment < 70) {
				c->textId = 71;
			} else if (b->data.house.entertainment < 80) {
				c->textId = 72;
			} else if (b->data.house.entertainment < 90) {
				c->textId = 73;
			} else {
				c->textId = 74;
			}
			break;
		case Overlay_Theater:
			if (b->data.house.theater <= 0) {
				c->textId = 75;
			} else if (b->data.house.theater >= 80) {
				c->textId = 76;
			} else if (b->data.house.theater >= 20) {
				c->textId = 77;
			} else {
				c->textId = 78;
			}
			break;
		case Overlay_Amphitheater:
			if (b->data.house.amphitheaterActor <= 0) {
				c->textId = 79;
			} else if (b->data.house.amphitheaterActor >= 80) {
				c->textId = 80;
			} else if (b->data.house.amphitheaterActor >= 20) {
				c->textId = 81;
			} else {
				c->textId = 82;
			}
			break;
		case Overlay_Colosseum:
			if (b->data.house.colosseumGladiator <= 0) {
				c->textId = 83;
			} else if (b->data.house.colosseumGladiator >= 80) {
				c->textId = 84;
			} else if (b->data.house.colosseumGladiator >= 20) {
				c->textId = 85;
			} else {
				c->textId = 86;
			}
			break;
		case Overlay_Hippodrome:
			if (b->data.house.hippodrome <= 0) {
				c->textId = 87;
			} else if (b->data.house.hippodrome >= 80) {
				c->textId = 88;
			} else if (b->data.house.hippodrome >= 20) {
				c->textId = 89;
			} else {
				c->textId = 90;
			}
			break;
		case Overlay_Education:
			switch (b->data.house.education) {
				case 0: c->textId = 100; break;
				case 1: c->textId = 101; break;
				case 2: c->textId = 102; break;
				case 3: c->textId = 103; break;
			}
			break;
		case Overlay_School:
			if (b->data.house.school <= 0) {
				c->textId = 19;
			} else if (b->data.house.school >= 80) {
				c->textId = 20;
			} else if (b->data.house.school >= 20) {
				c->textId = 21;
			} else {
				c->textId = 22;
			}
			break;
		case Overlay_Library:
			if (b->data.house.library <= 0) {
				c->textId = 23;
			} else if (b->data.house.library >= 80) {
				c->textId = 24;
			} else if (b->data.house.library >= 20) {
				c->textId = 25;
			} else {
				c->textId = 26;
			}
			break;
		case Overlay_Academy:
			if (b->data.house.academy <= 0) {
				c->textId = 27;
			} else if (b->data.house.academy >= 80) {
				c->textId = 28;
			} else if (b->data.house.academy >= 20) {
				c->textId = 29;
			} else {
				c->textId = 30;
			}
			break;
		case Overlay_Barber:
			if (b->data.house.barber <= 0) {
				c->textId = 31;
			} else if (b->data.house.barber >= 80) {
				c->textId = 32;
			} else if (b->data.house.barber < 20) {
				c->textId = 33;
			} else {
				c->textId = 34;
			}
			break;
		case Overlay_Bathhouse:
			if (b->data.house.bathhouse <= 0) {
				c->textId = 8;
			} else if (b->data.house.bathhouse >= 80) {
				c->textId = 9;
			} else if (b->data.house.bathhouse >= 20) {
				c->textId = 10;
			} else {
				c->textId = 11;
			}
			break;
		case Overlay_Clinic:
			if (b->data.house.clinic <= 0) {
				c->textId = 35;
			} else if (b->data.house.clinic >= 80) {
				c->textId = 36;
			} else if (b->data.house.clinic >= 20) {
				c->textId = 37;
			} else {
				c->textId = 38;
			}
			break;
		case Overlay_Hospital:
			if (b->data.house.hospital <= 0) {
				c->textId = 39;
			} else if (b->data.house.hospital >= 80) {
				c->textId = 40;
			} else if (b->data.house.hospital >= 20) {
				c->textId = 41;
			} else {
				c->textId = 42;
			}
			break;
		case Overlay_TaxIncome: {
			int denarii = calc_adjust_with_percentage(
				b->taxIncomeOrStorage / 2, Data_CityInfo.taxPercentage);
			if (denarii > 0) {
				c->hasNumericPrefix = 1;
				c->numericPrefix = denarii;
				c->textId = 45;
			} else if (b->houseTaxCoverage > 0) {
				c->textId = 44;
			} else {
				c->textId = 43;
			}
			break;
		}
		case Overlay_FoodStocks:
			if (b->housePopulation <= 0) {
				return;
			}
			if (!model_get_house(b->subtype.houseLevel)->food_types) {
				c->textId = 104;
			} else {
				int stocksPresent = 0;
				for (int i = Inventory_MinFood; i <= Inventory_MaxFood; i++) {
					stocksPresent += b->data.house.inventory[i];
				}
				int stocksPerPop = calc_percentage(stocksPresent, b->housePopulation);
				if (stocksPerPop <= 0) {
					c->textId = 4;
				} else if (stocksPerPop < 100) {
					c->textId = 5;
				} else if (stocksPerPop <= 200) {
					c->textId = 6;
				} else {
					c->textId = 7;
				}
			}
			break;
		case Overlay_Desirability:
			if (Data_Grid_desirability[gridOffset] < 0) {
				c->textId = 91;
			} else if (Data_Grid_desirability[gridOffset] == 0) {
				c->textId = 92;
			} else {
				c->textId = 93;
			}
			break;
		default:
			return;
	}
	c->type = TooltipType_Overlay;
	c->priority = TooltipPriority_High;
}

static void militaryMapClick()
{
	if (!Data_Settings_Map.current.gridOffset) {
		UI_Window_goTo(Window_City);
		return;
	}
	int formationId = Data_State.selectedLegionFormationId;
    const formation *m = formation_get(formationId);
	if (m->in_distant_battle || m->cursed_by_mars) {
		return;
	}
	int otherFormationId = Formation_getFormationForBuilding(
		GridOffset(Data_Settings_Map.current.x, Data_Settings_Map.current.y));
	if (otherFormationId && otherFormationId == formationId) {
		Formation_legionReturnHome(formationId);
	} else {
		Formation_legionMoveTo(formationId,
			Data_Settings_Map.current.x, Data_Settings_Map.current.y);
		Sound_Speech_playFile("wavs/cohort5.wav");
	}
	UI_Window_goTo(Window_City);
}

void UI_CityBuildings_handleMouseMilitary(const mouse *m)
{
	updateCityViewCoords(m);
	if (!Data_State.sidebarCollapsed && UI_Minimap_handleClick(m)) {
		return;
	}
	UI_CityBuildings_scrollMap(Scroll_getDirection(m));
	if (m->right.went_up) {
		UI_Warning_clearAll();
		UI_Window_goTo(Window_City);
	} else {
		updateCityViewCoords(m);
		if (m->left.went_down) {
			militaryMapClick();
		}
	}
}

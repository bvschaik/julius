#include "CityBuildings_private.h"

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
static void drawBuildingTopForProblemsOverlay(int gridOffset, int buildingId, int xOffset, int yOffset);
static void drawOverlayColumn(int height, int xOffset, int yOffset, int isRed);

void UI_CityBuildings_drawOverlayFootprints()
{
	FOREACH_XY_VIEW({
		int gridOffset = ViewToGridOffset(xView, yView);
		// TODO something related to elevation/draggable reservoir?
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

void UI_CityBuildings_drawOverlayTopsWalkersAnimation(int overlay)
{
	FOREACH_Y_VIEW(
		// draw walkers
		FOREACH_X_VIEW(
			int walkerId = Data_Grid_walkerIds[gridOffset];
			while (walkerId) {
				if (!Data_Walkers[walkerId].isGhost) {
					UI_CityBuildings_drawWalker(walkerId, xGraphic, yGraphic, 9999, 0);
				}
				walkerId = Data_Walkers[walkerId].nextWalkerIdOnSameTile;
			}
		);
		// draw animation
		FOREACH_X_VIEW(
			if (overlay == Overlay_Desirability) {
				drawBuildingTopForDesirabilityOverlay(gridOffset, xGraphic, yGraphic);
			} else if (Data_Grid_edge[gridOffset] & Edge_LeftmostTile) {
				if (overlay == Overlay_Water) {
					drawTopForWaterOverlay(gridOffset, xGraphic, yGraphic);
				} else if (overlay == Overlay_Native) {
					drawTopForNativeOverlay(gridOffset, xGraphic, yGraphic);
				} else if (!(Data_Grid_terrain[gridOffset] & 0x4140)) { // wall, aqueduct, road
					if ((Data_Grid_terrain[gridOffset] & Terrain_Building) && Data_Grid_buildingIds[gridOffset]) {
						int buildingId = Data_Grid_buildingIds[gridOffset];
						switch (overlay) {
							case Overlay_Fire:
								drawBuildingTopForFireOverlay(gridOffset, buildingId, xGraphic, yGraphic);
								break;
							case Overlay_Damage:
								drawBuildingTopForDamageOverlay(gridOffset, buildingId, xGraphic, yGraphic);
								break;
							case Overlay_Crime:
								drawBuildingTopForCrimeOverlay(gridOffset, buildingId, xGraphic, yGraphic);
								break;
							case Overlay_Entertainment:
								drawBuildingTopForEntertainmentOverlay(gridOffset, buildingId, xGraphic, yGraphic);
								break;
							case Overlay_Theater:
								drawBuildingTopForTheaterOverlay(gridOffset, buildingId, xGraphic, yGraphic);
								break;
							case Overlay_Amphitheater:
								drawBuildingTopForAmphitheaterOverlay(gridOffset, buildingId, xGraphic, yGraphic);
								break;
							case Overlay_Colosseum:
								drawBuildingTopForColosseumOverlay(gridOffset, buildingId, xGraphic, yGraphic);
								break;
							case Overlay_Hippodrome:
								drawBuildingTopForHippodromeOverlay(gridOffset, buildingId, xGraphic, yGraphic);
								break;
							case Overlay_Religion:
								drawBuildingTopForReligionOverlay(gridOffset, buildingId, xGraphic, yGraphic);
								break;
							case Overlay_Education:
								drawBuildingTopForEducationOverlay(gridOffset, buildingId, xGraphic, yGraphic);
								break;
							case Overlay_School:
								drawBuildingTopForSchoolOverlay(gridOffset, buildingId, xGraphic, yGraphic);
								break;
							case Overlay_Library:
								drawBuildingTopForLibraryOverlay(gridOffset, buildingId, xGraphic, yGraphic);
								break;
							case Overlay_Academy:
								drawBuildingTopForAcademyOverlay(gridOffset, buildingId, xGraphic, yGraphic);
								break;
							case Overlay_Barber:
								drawBuildingTopForBarberOverlay(gridOffset, buildingId, xGraphic, yGraphic);
								break;
							case Overlay_Bathhouse:
								drawBuildingTopForBathhouseOverlay(gridOffset, buildingId, xGraphic, yGraphic);
								break;
							case Overlay_Clinic:
								drawBuildingTopForClinicsOverlay(gridOffset, buildingId, xGraphic, yGraphic);
								break;
							case Overlay_Hospital:
								drawBuildingTopForHospitalOverlay(gridOffset, buildingId, xGraphic, yGraphic);
								break;
							case Overlay_FoodStocks:
								drawBuildingTopForFoodStocksOverlay(gridOffset, buildingId, xGraphic, yGraphic);
								break;
							case Overlay_TaxIncome:
								drawBuildingTopForTaxIncomeOverlay(gridOffset, buildingId, xGraphic, yGraphic);
								break;
							case Overlay_Problems:
								drawBuildingTopForProblemsOverlay(gridOffset, buildingId, xGraphic, yGraphic);
								break;
						}
					} else if (!(Data_Grid_terrain[gridOffset] & Terrain_Building)) {
						// terrain
						int graphicId = Data_Grid_graphicIds[gridOffset];
						switch (Data_Grid_bitfields[gridOffset] & Bitfield_Sizes) {
							case 0: DRAWTOP_SIZE1(graphicId, xGraphic, yGraphic); break;
							case 1: DRAWTOP_SIZE2(graphicId, xGraphic, yGraphic); break;
							case 2: DRAWTOP_SIZE3(graphicId, xGraphic, yGraphic); break;
							case 4: DRAWTOP_SIZE4(graphicId, xGraphic, yGraphic); break;
							case 8: DRAWTOP_SIZE5(graphicId, xGraphic, yGraphic); break;
						}
					}
				}
			}
		);
		FOREACH_X_VIEW(
			int draw = 0;
			if (Data_Grid_buildingIds[gridOffset]) {
				int btype = Data_Buildings[Data_Grid_buildingIds[gridOffset]].type;
				switch (overlay) {
					case Overlay_Fire:
					case Overlay_Crime:
						if (btype == Building_Prefecture || btype == Building_BurningRuin) {
							draw = 1;
						}
						break;
					case Overlay_Damage:
						if (btype == Building_EngineersPost) {
							draw = 1;
						}
						break;
					case Overlay_Water:
						if (btype == Building_Reservoir || btype == Building_Fountain) {
							draw = 1;
						}
						break;
					case Overlay_FoodStocks:
						if (btype == Building_Market || btype == Building_Granary) {
							draw = 1;
						}
						break;
				}
			}

			int graphicId = Data_Grid_graphicIds[gridOffset];
			if (GraphicNumAnimationSprites(graphicId) && draw) {
				if (Data_Grid_edge[gridOffset] & Edge_LeftmostTile) {
					int buildingId = Data_Grid_buildingIds[gridOffset];
					struct Data_Building *b = &Data_Buildings[buildingId];
					int colorMask = 0;
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
					} else {
						int animationOffset = Animation_getIndexForCityBuilding(graphicId, gridOffset);
						if (animationOffset > 0) {
							if (animationOffset > GraphicNumAnimationSprites(graphicId)) {
								animationOffset = GraphicNumAnimationSprites(graphicId);
							}
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
			} else if (Data_Grid_spriteOffsets[gridOffset] && (Data_Grid_terrain[gridOffset] & Terrain_Water)) {
				UI_CityBuildings_drawBridge(gridOffset, xGraphic, yGraphic);
			}
		);
	);
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
		if (buildingId && Data_Buildings[buildingId].hasWellAccess == 1) {
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
			Data_Walkers[walkerId].actionState == WalkerActionState_20_CartpusherInitial &&
			Data_Walkers[walkerId].minMaxSeen) {
			Data_Buildings[buildingId].showOnProblemOverlay = 1;
		}
	} else if (BuildingIsWorkshop(type)) {
		int walkerId = Data_Buildings[buildingId].walkerId;
		if (walkerId &&
			Data_Walkers[walkerId].actionState == WalkerActionState_20_CartpusherInitial &&
			Data_Walkers[walkerId].minMaxSeen) {
			Data_Buildings[buildingId].showOnProblemOverlay = 1;
		} else if (Data_Buildings[buildingId].loadsStored <= 0) {
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
		if (Data_Buildings[buildingId].data.storage.resourceStored[Resource_None] < 2400) {
			Graphics_drawImage(GraphicId(ID_Graphic_Granary) + 2,
				xOffset + 32, yOffset - 61);
			if (Data_Buildings[buildingId].data.storage.resourceStored[Resource_None] < 1800) {
				Graphics_drawImage(GraphicId(ID_Graphic_Granary) + 3,
					xOffset + 56, yOffset - 51);
			}
			if (Data_Buildings[buildingId].data.storage.resourceStored[Resource_None] < 1200) {
				Graphics_drawImage(GraphicId(ID_Graphic_Granary) + 4,
					xOffset + 91, yOffset - 51);
			}
			if (Data_Buildings[buildingId].data.storage.resourceStored[Resource_None] < 600) {
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
	if (height) {
		for (int i = 1; i < height; i++) {
			Graphics_drawImage(graphicId + 1, xOffset + 17, yOffset - 8 - 10 * i + 13);
		}
		// top
		Graphics_drawImage(graphicId,
			xOffset + 5, yOffset - 8 - capitalHeight - 10 * (height - 1) + 13);
	}
}

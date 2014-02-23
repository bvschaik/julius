#include "../Data/Building.h"
#include "../Data/CityInfo.h"
#include "../Data/CityView.h"
#include "../Data/Constants.h"
#include "../Data/Graphics.h"
#include "../Data/Grid.h"
#include "../Data/Model.h"
#include "../Data/State.h"
#include "../Time.h"
#include "../Calc.h"
#include "../Graphics.h"
#include "../CityView.h"
#include "../Sound.h"

#include <cstdio>

#define DRAWTOP_SIZE1(g,x,y) Graphics_drawIsometricTop(g, x, y, 0)
#define DRAWTOP_SIZE2(g,x,y) Graphics_drawIsometricTop(g, x + 30, y - 15, 0)
#define DRAWTOP_SIZE3(g,x,y) Graphics_drawIsometricTop(g, x + 60, y - 30, 0)
#define DRAWTOP_SIZE4(g,x,y) Graphics_drawIsometricTop(g, x + 90, y - 45, 0)
#define DRAWTOP_SIZE5(g,x,y) Graphics_drawIsometricTop(g, x + 120, y - 60, 0)

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


static void drawBuildingFootprints();
static void drawOverlayColumn(int height, int xOffset, int yOffset, int isRed);


static int lastAnimationTime = 0;
static int advanceAnimation;

void UI_CityBuildings_drawForeground(int x, int y)
{
	printf("Drawing city at %d %d\n", x, y);
	Data_CityView.xInTiles = x;
	Data_CityView.yInTiles = y;
	Graphics_setClipRectangle(
		Data_CityView.xOffsetInPixels, Data_CityView.yOffsetInPixels,
		Data_CityView.widthInPixels, Data_CityView.heightInPixels);
	printf("Cliprectangle set to %d, %d, %d, %d\n",
		Data_CityView.xOffsetInPixels, Data_CityView.yOffsetInPixels,
		Data_CityView.widthInPixels, Data_CityView.heightInPixels);

	advanceAnimation = 0;
	int now = Time_getMillis();
	if (now - lastAnimationTime > 60 || now - lastAnimationTime < 0) {
		lastAnimationTime = now;
		advanceAnimation = 1;
	}

	// TODO overlay and stuff
	if (Data_State.currentOverlay) {
	} else {
		drawBuildingFootprints();
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
		} else if (Data_Grid_edge[gridOffset] & Edge_leftmostTile) {
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

static void drawOverlayFootprints()
{
	FOREACH_XY_VIEW({
		int gridOffset = ViewToGridOffset(xView, yView);
		// TODO something related to elevation/reservoirSimon?
		if (gridOffset < 0) {
			// Outside map: draw black tile
			Graphics_drawIsometricFootprint(GraphicId(ID_Graphic_TerrainBlack),
				xGraphic, yGraphic, 0);
		} else if (Data_State.currentOverlay == Overlay_Desirability) {
			//drawOverlayFootprintDesirability(xGraphic, yGraphic);
		} else if (Data_Grid_edge[gridOffset] & Edge_leftmostTile) {
			if (Data_State.currentOverlay == Overlay_Water) {
				//drawOverlayFootprintWater(xGraphic, yGraphic);
			} else if (Data_State.currentOverlay == Overlay_Native) {
				//drawOverlayFootprintNative(xGraphic, yGraphic);
			} else {

			}
		}
	});
}

/*
terrain = grid_terrain[drawCity_currentTileGridOffset];
v0 = grid_bitfields[drawCity_currentTileGridOffset] & 0xF;
if ( terrain & T_Aqueduct )
{
  graphic_currentGraphicId = (grid_random[drawCity_currentTileGridOffset] & 7)
                           + graphic_terrain_grass1;
  j_fun_drawGraphic(graphic_currentGraphicId, v5, v3);
}
else
{
  if ( !(terrain & T_Road) || terrain & T_Building )
  {
    if ( terrain & T_Wall )
    {
      graphic_currentGraphicId = (grid_random[drawCity_currentTileGridOffset] & 7)
                               + graphic_terrain_grass1;
      j_fun_drawGraphic(graphic_currentGraphicId, v5, v3);
    }
    else
    {
      if ( terrain & T_Building )
      {
        j_fun_drawBuildingWithOverlay(
          grid_buildingIds[drawCity_currentTileGridOffset],
          xGraphic,
          yGraphic,
          0);
      }
      else
      {
        graphic_currentGraphicId = grid_graphicIds[drawCity_currentTileGridOffset];
		switch (bitfieldSize) {
		  case 0:
			j_fun_drawGraphic(graphic_currentGraphicId, xGraphic, yGraphic);
			break;
          case 1:
            j_fun_drawGraphic(graphic_currentGraphicId, xGraphic + 30, yGraphic - 15);
            break;
          case 2:
            j_fun_drawGraphic(graphic_currentGraphicId, xGraphic + 60, yGraphic - 30);
            break;
          case 4:
            j_fun_drawGraphic(graphic_currentGraphicId, xGraphic + 90, yGraphic - 45);
            break;
          case 8:
            j_fun_drawGraphic(graphic_currentGraphicId, xGraphic + 120, yGraphic - 60);
            break;
        }
      }
    }
  }
  else
  {
    graphic_currentGraphicId = grid_graphicIds[drawCity_currentTileGridOffset];
    if ( v0 )
    {
      if ( v0 == 1 )
        j_fun_drawGraphic(
          graphic_currentGraphicId,
          v5 + iso_tile_half_width_29 + 1,
          v3 - iso_tile_half_height_15);
    }
    else
    {
      j_fun_drawGraphic(graphic_currentGraphicId, v5, v3);
    }
  }
}
*/

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

static void drawOverlayColumn(int height, int xOffset, int yOffset, int isRed)
{
	// TODO
}

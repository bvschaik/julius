#include "buildinginfo.h"
#include "core/calc.h"
#include "graphics.h"
#include "widget_text.h"
#include "game/resource.h"
#include "graphics/image.h"

#include <sound>
#include <data>

static void drawFarm(BuildingInfoContext *c, int helpId, const char *soundFile, int groupId, int resourceId)
{
    c->helpId = helpId;
    PLAY_SOUND(soundFile);

    Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    Graphics_drawImage(image_group(ID_Graphic_ResourceIcons) + resourceId,
                       c->xOffset + 10, c->yOffset + 10);
    Widget_GameText_drawCentered(groupId, 0, c->xOffset, c->yOffset + 10,
                                 16 * c->widthBlocks, FONT_LARGE_BLACK);

    int pctGrown = calc_percentage(Data_Buildings[c->buildingId].data.industry.progress, 200);
    int width = Widget_GameText_draw(groupId, 2, c->xOffset + 32, c->yOffset + 44, FONT_NORMAL_BLACK);
    width += Widget::Text::drawPercentage(pctGrown,
                                          c->xOffset + 32 + width, c->yOffset + 44, FONT_NORMAL_BLACK);
    Widget_GameText_draw(groupId, 3, c->xOffset + 32 + width, c->yOffset + 44, FONT_NORMAL_BLACK);

    if (!c->hasRoadAccess)
    {
        DRAW_DESC_AT(70, 69, 25);
    }
    else if (Data_CityInfo.resourceIndustryMothballed[resourceId])
    {
        DRAW_DESC_AT(70, groupId, 4);
    }
    else if (Data_Buildings[c->buildingId].data.industry.curseDaysLeft > 4)
    {
        DRAW_DESC_AT(70, groupId, 11);
    }
    else if (Data_Buildings[c->buildingId].numWorkers <= 0)
    {
        DRAW_DESC_AT(70, groupId, 5);
    }
    else if (c->workerPercentage >= 100)
    {
        DRAW_DESC_AT(70, groupId, 6);
    }
    else if (c->workerPercentage >= 75)
    {
        DRAW_DESC_AT(70, groupId, 7);
    }
    else if (c->workerPercentage >= 50)
    {
        DRAW_DESC_AT(70, groupId, 8);
    }
    else if (c->workerPercentage >= 25)
    {
        DRAW_DESC_AT(70, groupId, 9);
    }
    else
    {
        DRAW_DESC_AT(70, groupId, 10);
    }

    Widget_Panel_drawInnerPanel(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
    UI_BuildingInfo_drawEmploymentInfo(c, c->yOffset + 142);
    Widget_GameText_drawMultiline(groupId, 1,
                                  c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 113,
                                  16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
}

void UI_BuildingInfo_drawWheatFarm(BuildingInfoContext *c)
{
    drawFarm(c, 89, "wavs/wheat_farm.wav", 112, RESOURCE_WHEAT);
}

void UI_BuildingInfo_drawVegetableFarm(BuildingInfoContext *c)
{
    drawFarm(c, 90, "wavs/veg_farm.wav", 113, RESOURCE_VEGETABLES);
}

void UI_BuildingInfo_drawFruitFarm(BuildingInfoContext *c)
{
    drawFarm(c, 90, "wavs/figs_farm.wav", 114, RESOURCE_FRUIT);
}

void UI_BuildingInfo_drawOliveFarm(BuildingInfoContext *c)
{
    drawFarm(c, 91, "wavs/olives_farm.wav", 115, RESOURCE_OLIVES);
}

void UI_BuildingInfo_drawVinesFarm(BuildingInfoContext *c)
{
    drawFarm(c, 91, "wavs/vines_farm.wav", 116, RESOURCE_VINES);
}

void UI_BuildingInfo_drawPigFarm(BuildingInfoContext *c)
{
    drawFarm(c, 90, "wavs/meat_farm.wav", 117, RESOURCE_MEAT);
}

static void drawRawMaterial(BuildingInfoContext *c, int helpId, const char *soundFile, int groupId, int resourceId)
{
    c->helpId = helpId;
    PLAY_SOUND(soundFile);

    Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    Graphics_drawImage(image_group(ID_Graphic_ResourceIcons) + resourceId,
                       c->xOffset + 10, c->yOffset + 10);
    Widget_GameText_drawCentered(groupId, 0, c->xOffset, c->yOffset + 10,
                                 16 * c->widthBlocks, FONT_LARGE_BLACK);

    int pctDone = calc_percentage(Data_Buildings[c->buildingId].data.industry.progress, 200);
    int width = Widget_GameText_draw(groupId, 2, c->xOffset + 32, c->yOffset + 44, FONT_NORMAL_BLACK);
    width += Widget::Text::drawPercentage(pctDone,
                                          c->xOffset + 32 + width, c->yOffset + 44, FONT_NORMAL_BLACK);
    Widget_GameText_draw(groupId, 3, c->xOffset + 32 + width, c->yOffset + 44, FONT_NORMAL_BLACK);

    if (!c->hasRoadAccess)
    {
        DRAW_DESC_AT(70, 69, 25);
    }
    else if (Data_CityInfo.resourceIndustryMothballed[resourceId])
    {
        DRAW_DESC_AT(70, groupId, 4);
    }
    else if (Data_Buildings[c->buildingId].numWorkers <= 0)
    {
        DRAW_DESC_AT(70, groupId, 5);
    }
    else if (c->workerPercentage >= 100)
    {
        DRAW_DESC_AT(70, groupId, 6);
    }
    else if (c->workerPercentage >= 75)
    {
        DRAW_DESC_AT(70, groupId, 7);
    }
    else if (c->workerPercentage >= 50)
    {
        DRAW_DESC_AT(70, groupId, 8);
    }
    else if (c->workerPercentage >= 25)
    {
        DRAW_DESC_AT(70, groupId, 9);
    }
    else
    {
        DRAW_DESC_AT(70, groupId, 10);
    }

    Widget_Panel_drawInnerPanel(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
    UI_BuildingInfo_drawEmploymentInfo(c, c->yOffset + 142);
    Widget_GameText_drawMultiline(groupId, 1,
                                  c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 113,
                                  16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
}

void UI_BuildingInfo_drawMarbleQuarry(BuildingInfoContext *c)
{
    drawRawMaterial(c, 95, "wavs/quarry.wav", 118, RESOURCE_MARBLE);
}

void UI_BuildingInfo_drawIronMine(BuildingInfoContext *c)
{
    drawRawMaterial(c, 93, "wavs/mine.wav", 119, RESOURCE_IRON);
}

void UI_BuildingInfo_drawTimberYard(BuildingInfoContext *c)
{
    drawRawMaterial(c, 94, "wavs/timber.wav", 120, RESOURCE_TIMBER);
}

void UI_BuildingInfo_drawClayPit(BuildingInfoContext *c)
{
    drawRawMaterial(c, 92, "wavs/clay.wav", 121, RESOURCE_CLAY);
}

static void drawWorkshop(BuildingInfoContext *c, int helpId, const char *soundFile, int groupId, int resourceId, int inputResourceId)
{
    c->helpId = helpId;
    PLAY_SOUND(soundFile);

    Widget_Panel_drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    Graphics_drawImage(image_group(ID_Graphic_ResourceIcons) + resourceId,
                       c->xOffset + 10, c->yOffset + 10);
    Widget_GameText_drawCentered(groupId, 0, c->xOffset, c->yOffset + 10,
                                 16 * c->widthBlocks, FONT_LARGE_BLACK);

    int pctDone = calc_percentage(Data_Buildings[c->buildingId].data.industry.progress, 400);
    int width = Widget_GameText_draw(groupId, 2, c->xOffset + 32, c->yOffset + 40, FONT_NORMAL_BLACK);
    width += Widget::Text::drawPercentage(pctDone,
                                          c->xOffset + 32 + width, c->yOffset + 40, FONT_NORMAL_BLACK);
    Widget_GameText_draw(groupId, 3, c->xOffset + 32 + width, c->yOffset + 40, FONT_NORMAL_BLACK);

    Graphics_drawImage(image_group(ID_Graphic_ResourceIcons) + inputResourceId,
                       c->xOffset + 32, c->yOffset + 56);
    width = Widget_GameText_draw(groupId, 12, c->xOffset + 60, c->yOffset + 60, FONT_NORMAL_BLACK);
    if (Data_Buildings[c->buildingId].loadsStored < 1)
    {
        Widget_GameText_drawNumberWithDescription(8, 10, 0,
                c->xOffset + 60 + width, c->yOffset + 60, FONT_NORMAL_BLACK);
    }
    else
    {
        Widget_GameText_drawNumberWithDescription(8, 10,
                Data_Buildings[c->buildingId].loadsStored,
                c->xOffset + 60 + width, c->yOffset + 60, FONT_NORMAL_BLACK);
    }

    if (!c->hasRoadAccess)
    {
        DRAW_DESC_AT(86, 69, 25);
    }
    else if (Data_CityInfo.resourceIndustryMothballed[resourceId])
    {
        DRAW_DESC_AT(86, groupId, 4);
    }
    else if (Data_Buildings[c->buildingId].numWorkers <= 0)
    {
        DRAW_DESC_AT(86, groupId, 5);
    }
    else if (Data_Buildings[c->buildingId].loadsStored <= 0)
    {
        DRAW_DESC_AT(86, groupId, 11);
    }
    else if (c->workerPercentage >= 100)
    {
        DRAW_DESC_AT(86, groupId, 6);
    }
    else if (c->workerPercentage >= 75)
    {
        DRAW_DESC_AT(86, groupId, 7);
    }
    else if (c->workerPercentage >= 50)
    {
        DRAW_DESC_AT(86, groupId, 8);
    }
    else if (c->workerPercentage >= 25)
    {
        DRAW_DESC_AT(86, groupId, 9);
    }
    else
    {
        DRAW_DESC_AT(86, groupId, 10);
    }

    Widget_Panel_drawInnerPanel(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
    UI_BuildingInfo_drawEmploymentInfo(c, c->yOffset + 142);
}

void UI_BuildingInfo_drawWineWorkshop(BuildingInfoContext *c)
{
    drawWorkshop(c, 96, "wavs/wine_workshop.wav", 122, RESOURCE_WINE, RESOURCE_VINES);
}

void UI_BuildingInfo_drawOilWorkshop(BuildingInfoContext *c)
{
    drawWorkshop(c, 97, "wavs/oil_workshop.wav", 123, RESOURCE_OIL, RESOURCE_OLIVES);
}

void UI_BuildingInfo_drawWeaponsWorkshop(BuildingInfoContext *c)
{
    drawWorkshop(c, 98, "wavs/weapons_workshop.wav", 124, RESOURCE_WEAPONS, RESOURCE_IRON);
}

void UI_BuildingInfo_drawFurnitureWorkshop(BuildingInfoContext *c)
{
    drawWorkshop(c, 99, "wavs/furniture_workshop.wav", 125, RESOURCE_FURNITURE, RESOURCE_TIMBER);
}

void UI_BuildingInfo_drawPotteryWorkshop(BuildingInfoContext *c)
{
    drawWorkshop(c, 1, "wavs/pottery_workshop.wav", 126, RESOURCE_POTTERY, RESOURCE_CLAY);
}

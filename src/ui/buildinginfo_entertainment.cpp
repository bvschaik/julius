#include "buildinginfo.h"
#include "widget.h"
#include "data/building.hpp"

#include <sound>
#include <ui>

void UI_BuildingInfo_drawAmphitheater(BuildingInfoContext *c)
{
    c->helpId = 72;
    PLAY_SOUND("wavs/amphitheatre.wav");
    Widget::Panel::drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    Widget_GameText_drawCentered(71, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
    struct Data_Building *b = &Data_Buildings[c->buildingId];
    if (!c->hasRoadAccess)
    {
        DRAW_DESC(69, 25);
    }
    else if (b->numWorkers <= 0)
    {
        DRAW_DESC(71, 6);
    }
    else if (!b->data.entertainment.numShows)
    {
        DRAW_DESC(71, 2);
    }
    else if (b->data.entertainment.numShows == 2)
    {
        DRAW_DESC(71, 3);
    }
    else if (b->data.entertainment.days1)
    {
        DRAW_DESC(71, 4);
    }
    else if (b->data.entertainment.days2)
    {
        DRAW_DESC(71, 5);
    }

    Widget::Panel::drawInnerPanel(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 7);
    UI_BuildingInfo_drawEmploymentInfo(c, c->yOffset + 138);
    if (b->data.entertainment.days1 > 0)
    {
        int width = Widget_GameText_draw(71, 8, c->xOffset + 32, c->yOffset + 182, FONT_SMALL_BLACK);
        Widget_GameText_drawNumberWithDescription(8, 44, 2 * b->data.entertainment.days1,
                c->xOffset + width + 32, c->yOffset + 182, FONT_SMALL_BLACK);
    }
    else
    {
        Widget_GameText_draw(71, 7, c->xOffset + 32, c->yOffset + 182, FONT_SMALL_BLACK);
    }
    if (b->data.entertainment.days2 > 0)
    {
        int width = Widget_GameText_draw(71, 10, c->xOffset + 32, c->yOffset + 202, FONT_SMALL_BLACK);
        Widget_GameText_drawNumberWithDescription(8, 44, 2 * b->data.entertainment.days2,
                c->xOffset + width + 32, c->yOffset + 202, FONT_SMALL_BLACK);
        Widget_GameText_draw(72, 7 + b->data.entertainment.play,
                             c->xOffset + 32, c->yOffset + 222, FONT_SMALL_BLACK);
    }
    else
    {
        Widget_GameText_draw(71, 9, c->xOffset + 32, c->yOffset + 202, FONT_SMALL_BLACK);
    }
}

void UI_BuildingInfo_drawTheater(BuildingInfoContext *c)
{
    c->helpId = 71;
    PLAY_SOUND("wavs/theatre.wav");
    Widget::Panel::drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    Widget_GameText_drawCentered(72, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
    struct Data_Building *b = &Data_Buildings[c->buildingId];
    if (!c->hasRoadAccess)
    {
        DRAW_DESC(69, 25);
    }
    else if (b->numWorkers <= 0)
    {
        DRAW_DESC(72, 4);
    }
    else if (!b->data.entertainment.numShows)
    {
        DRAW_DESC(72, 2);
    }
    else if (b->data.entertainment.days1)
    {
        DRAW_DESC(72, 3);
    }

    Widget::Panel::drawInnerPanel(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 6);
    UI_BuildingInfo_drawEmploymentInfo(c, c->yOffset + 138);
    if (b->data.entertainment.days1 > 0)
    {
        int width = Widget_GameText_draw(72, 6, c->xOffset + 32, c->yOffset + 182, FONT_SMALL_BLACK);
        Widget_GameText_drawNumberWithDescription(8, 44, 2 * b->data.entertainment.days1,
                c->xOffset + width + 32, c->yOffset + 182, FONT_SMALL_BLACK);
        Widget_GameText_draw(72, 7 + b->data.entertainment.play,
                             c->xOffset + 32, c->yOffset + 202, FONT_SMALL_BLACK);
    }
    else
    {
        Widget_GameText_draw(72, 5, c->xOffset + 32, c->yOffset + 182, FONT_SMALL_BLACK);
    }
}

void UI_BuildingInfo_drawHippodrome(BuildingInfoContext *c)
{
    c->helpId = 74;
    PLAY_SOUND("wavs/hippodrome.wav");
    Widget::Panel::drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    Widget_GameText_drawCentered(73, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
    struct Data_Building *b = &Data_Buildings[c->buildingId];
    if (!c->hasRoadAccess)
    {
        DRAW_DESC(69, 25);
    }
    else if (b->numWorkers <= 0)
    {
        DRAW_DESC(73, 4);
    }
    else if (!b->data.entertainment.numShows)
    {
        DRAW_DESC(73, 2);
    }
    else if (b->data.entertainment.days1)
    {
        DRAW_DESC(73, 3);
    }

    Widget::Panel::drawInnerPanel(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 6);
    UI_BuildingInfo_drawEmploymentInfo(c, c->yOffset + 138);
    if (b->data.entertainment.days1 > 0)
    {
        int width = Widget_GameText_draw(73, 6, c->xOffset + 32, c->yOffset + 202, FONT_SMALL_BLACK);
        Widget_GameText_drawNumberWithDescription(8, 44, 2 * b->data.entertainment.days1,
                c->xOffset + width + 32, c->yOffset + 202, FONT_SMALL_BLACK);
    }
    else
    {
        Widget_GameText_draw(73, 5, c->xOffset + 32, c->yOffset + 202, FONT_SMALL_BLACK);
    }
}

void UI_BuildingInfo_drawColosseum(BuildingInfoContext *c)
{
    c->helpId = 73;
    PLAY_SOUND("wavs/colloseum.wav");
    Widget::Panel::drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    Widget_GameText_drawCentered(74, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
    struct Data_Building *b = &Data_Buildings[c->buildingId];
    if (!c->hasRoadAccess)
    {
        DRAW_DESC(69, 25);
    }
    else if (b->numWorkers <= 0)
    {
        DRAW_DESC(74, 6);
    }
    else if (!b->data.entertainment.numShows)
    {
        DRAW_DESC(74, 2);
    }
    else if (b->data.entertainment.numShows == 2)
    {
        DRAW_DESC(74, 3);
    }
    else if (b->data.entertainment.days1)
    {
        DRAW_DESC(74, 5);
    }
    else if (b->data.entertainment.days2)
    {
        DRAW_DESC(74, 4);
    }

    Widget::Panel::drawInnerPanel(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 6);
    UI_BuildingInfo_drawEmploymentInfo(c, c->yOffset + 138);
    if (b->data.entertainment.days1 > 0)
    {
        int width = Widget_GameText_draw(74, 8, c->xOffset + 32, c->yOffset + 182, FONT_SMALL_BLACK);
        Widget_GameText_drawNumberWithDescription(8, 44, 2 * b->data.entertainment.days1,
                c->xOffset + width + 32, c->yOffset + 182, FONT_SMALL_BLACK);
    }
    else
    {
        Widget_GameText_draw(74, 7, c->xOffset + 32, c->yOffset + 182, FONT_SMALL_BLACK);
    }
    if (b->data.entertainment.days2 > 0)
    {
        int width = Widget_GameText_draw(74, 10, c->xOffset + 32, c->yOffset + 202, FONT_SMALL_BLACK);
        Widget_GameText_drawNumberWithDescription(8, 44, 2 * b->data.entertainment.days2,
                c->xOffset + width + 32, c->yOffset + 202, FONT_SMALL_BLACK);
    }
    else
    {
        Widget_GameText_draw(74, 9, c->xOffset + 32, c->yOffset + 202, FONT_SMALL_BLACK);
    }
}

static void drawEntertainmentSchool(BuildingInfoContext *c, int groupId, const char *soundFile)
{
    c->helpId = 75;
    PLAY_SOUND(soundFile);

    Widget::Panel::drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    Widget_GameText_drawCentered(groupId, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
    if (!c->hasRoadAccess)
    {
        DRAW_DESC(69, 25);
    }
    else if (Data_Buildings[c->buildingId].numWorkers <= 0)
    {
        DRAW_DESC(groupId, 7);
    }
    else if (c->workerPercentage >= 100)
    {
        DRAW_DESC(groupId, 2);
    }
    else if (c->workerPercentage >= 75)
    {
        DRAW_DESC(groupId, 3);
    }
    else if (c->workerPercentage >= 50)
    {
        DRAW_DESC(groupId, 4);
    }
    else if (c->workerPercentage >= 25)
    {
        DRAW_DESC(groupId, 5);
    }
    else
    {
        DRAW_DESC(groupId, 6);
    }
    Widget::Panel::drawInnerPanel(c->xOffset + 16, c->yOffset + 136, c->widthBlocks - 2, 4);
    UI_BuildingInfo_drawEmploymentInfo(c, c->yOffset + 142);
}

void UI_BuildingInfo_drawGladiatorSchool(BuildingInfoContext *c)
{
    drawEntertainmentSchool(c, 75, "wavs/glad_pit.wav");
}

void UI_BuildingInfo_drawLionHouse(BuildingInfoContext *c)
{
    drawEntertainmentSchool(c, 76, "wavs/lion_pit.wav");
}

void UI_BuildingInfo_drawActorColony(BuildingInfoContext *c)
{
    drawEntertainmentSchool(c, 77, "wavs/art_pit.wav");
}

void UI_BuildingInfo_drawChariotMaker(BuildingInfoContext *c)
{
    drawEntertainmentSchool(c, 78, "wavs/char_pit.wav");
}

#include "buildinginfo.h"
#include "graphics.h"
#include "widget.h"

#include <sound>
#include <ui>

void UI_BuildingInfo_drawGarden(BuildingInfoContext *c)
{
    c->helpId = 80;
    PLAY_SOUND("wavs/park.wav");
    Widget::Panel::drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    Widget_GameText_drawCentered(79, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
    Widget_GameText_drawMultiline(79, 1,
                                  c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 158,
                                  16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
}

void UI_BuildingInfo_drawPlaza(BuildingInfoContext *c)
{
    c->helpId = 80;
    PLAY_SOUND("wavs/plaza.wav");
    UI_BuildingInfo_drawFigureImagesLocal(c);
    Widget::Panel::drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    Widget_GameText_drawCentered(137, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
    UI_BuildingInfo_drawFigureList(c);
    Widget_GameText_drawMultiline(137, 1,
                                  c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 113,
                                  16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
}

void UI_BuildingInfo_drawStatue(BuildingInfoContext *c)
{
    c->helpId = 79;
    PLAY_SOUND("wavs/statue.wav");
    UI_BuildingInfo_drawFigureImagesLocal(c);
    Widget::Panel::drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    Widget_GameText_drawCentered(80, 0, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
    Widget_GameText_drawMultiline(80, 1,
                                  c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 158,
                                  16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
}

void UI_BuildingInfo_drawTriumphalArch(BuildingInfoContext *c)
{
    c->helpId = 79;
    PLAY_SOUND("wavs/statue.wav");
    UI_BuildingInfo_drawFigureImagesLocal(c);
    Widget::Panel::drawOuterPanel(c->xOffset, c->yOffset, c->widthBlocks, c->heightBlocks);
    Widget_GameText_drawCentered(80, 2, c->xOffset, c->yOffset + 10, 16 * c->widthBlocks, FONT_LARGE_BLACK);
    Widget_GameText_drawMultiline(80, 3,
                                  c->xOffset + 32, c->yOffset + 16 * c->heightBlocks - 158,
                                  16 * (c->widthBlocks - 4), FONT_NORMAL_BLACK);
}

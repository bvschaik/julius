#ifndef WIDGET_PANEL_H
#define WIDGET_PANEL_H

namespace Widget
{

struct Panel
{
    static void drawOuterPanel(int xOffset, int yOffset, int widthInBlocks, int heightInBlocks);
    static void drawUnborderedPanel(int xOffset, int yOffset, int widthInBlocks, int heightInBlocks);
    static void drawInnerPanel(int xOffset, int yOffset, int widthInBlocks, int heightInBlocks);
    static void drawInnerPanelBottom(int xOffset, int yOffset, int widthInBlocks);
    static void drawButtonBorder(int xOffset, int yOffset, int widthInPixels, int heightInPixels, int hasFocus);
    static void drawSmallLabelButton(int xOffset, int yOffset, int widthInBlocks, int type);
    static void drawLargeLabelButton(int xOffset, int yOffset, int widthInBlocks, int type);
};

}
#endif

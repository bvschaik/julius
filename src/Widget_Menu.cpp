#include "Widget.h"

void Widget_Menu_drawMenuBar(MenuBarItem *items, int numItems)
{
	int xOffset = items[0].xOffset;
	for (int i = 0; i < numItems; i++) {
		
	}
}
/*
void __cdecl fun_drawMenuBar(struct C3Menu *menu, signed int numItems)
{
  int y; // [sp+54h] [bp-10h]@3
  int x; // [sp+58h] [bp-Ch]@3
  int group; // [sp+5Ch] [bp-8h]@3
  signed int i; // [sp+60h] [bp-4h]@1

  text_xoffset = menu->xOffset;
  for ( i = 1; i <= numItems; ++i )
  {
    group = menu->textGroup;
    menu->xOffset = text_xoffset;
    x = (signed __int16)text_xoffset;
    y = menu->yOffset;
    j_fun_getGameTextString(group, 0);
    if ( i == selectedMainMenuItemId )
    {
      drawtext_forceColor = 1;
      j_fun_getStringWidth(gametext_result, graphic_font);
      j_fun_drawGameText(group, 0, x, y, graphic_font + F_SmallBrown, 0xFAC1u);
      drawtext_forceColor = 0;
    }
    else
    {
      j_fun_drawGameText(group, 0, x, y, graphic_font + F_SmallBrown, 0);
    }
    menu->xEnd = text_xoffset;
    text_xoffset += 32;                         // spacing
    ++menu;
  }
}
*/

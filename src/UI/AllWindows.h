#ifndef UI_ALLWINDOWS_H
#define UI_ALLWINDOWS_H

void UI_MainMenu_drawBackground();
void UI_MainMenu_drawForeground();
void UI_MainMenu_handleMouse();

void UI_DifficultyOptions_drawForeground();
void UI_DifficultyOptions_handleMouse();

void UI_DisplayOptions_drawForeground();

void UI_SoundOptions_init();
void UI_SoundOptions_drawForeground();
void UI_SoundOptions_handleMouse();

void UI_SpeedOptions_init();
void UI_SpeedOptions_drawForeground();
void UI_SpeedOptions_handleMouse();

void UI_Advisors_drawBackground();
void UI_Advisors_drawForeground();
void UI_Advisors_handleMouse();

void UI_SetSalaryDialog_drawBackground();
void UI_SetSalaryDialog_drawForeground();
void UI_SetSalaryDialog_handleMouse();

void UI_DonateToCityDialog_init();
void UI_DonateToCityDialog_drawBackground();
void UI_DonateToCityDialog_drawForeground();
void UI_DonateToCityDialog_handleMouse();

void UI_SendGiftToCaesarDialog_init();
void UI_SendGiftToCaesarDialog_drawBackground();
void UI_SendGiftToCaesarDialog_drawForeground();
void UI_SendGiftToCaesarDialog_handleMouse();


#endif

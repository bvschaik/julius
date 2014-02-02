#ifndef UI_CONFIRMDIALOG_H
#define UI_CONFIRMDIALOG_H

void UI_PopupDialog_show(int msgId, void (*okFunc)(int), int hasOkCancelButtons);

void UI_PopupDialog_drawBackground();
void UI_PopupDialog_drawForeground();
void UI_PopupDialog_handleMouse();

#endif

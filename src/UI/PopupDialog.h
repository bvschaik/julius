#ifndef UI_POPUPDIALOG_H
#define UI_POPUPDIALOG_H

void UI_PopupDialog_show(int msgId, void (*okFunc)(int), int hasOkCancelButtons);

void UI_PopupDialog_drawBackground();
void UI_PopupDialog_drawForeground();
void UI_PopupDialog_handleMouse();

#endif

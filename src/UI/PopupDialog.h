#ifndef UI_POPUPDIALOG_H
#define UI_POPUPDIALOG_H

enum {
	PopupDialog_OpenTrade = 2,
	PopupDialog_RequestSendGoods = 4,
	PopupDialog_RequestNotEnoughGoods = 6,
	PopupDialog_NoLegionsAvailable = 8,
	PopupDialog_NoLegionsSelected = 10,
	PopupDialog_RequestSendTroops = 12,
};

void UI_PopupDialog_show(int msgId, void (*okFunc)(int), int hasOkCancelButtons);

void UI_PopupDialog_drawBackground();
void UI_PopupDialog_drawForeground();
void UI_PopupDialog_handleMouse();

#endif

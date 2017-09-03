#ifndef UI_FILEDIALOG_H
#define UI_FILEDIALOG_H

#include "graphics/mouse.h"

enum {
	FileDialogType_Save = 0,
	FileDialogType_Load = 1,
	FileDialogType_Delete = 2
};

void UI_FileDialog_show(int type);
void UI_FileDialog_drawBackground();
void UI_FileDialog_drawForeground();
void UI_FileDialog_handleMouse(const mouse *m);

#endif

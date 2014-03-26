#ifndef UI_MESSAGEDIALOG_H
#define UI_MESSAGEDIALOG_H

enum {
	MessageDialog_About = 0,
	MessageDialog_Help = 10,
	MessageDialog_EntertainmentAdvisor = 28,
	MessageDialog_EmpireMap = 32,
	MessageDialog_Messages = 34,
	MessageDialog_Industry = 46,
	MessageDialog_Theft = 251,
};

void UI_MessageDialog_show(int messageId, int backgroundIsProvided);

void UI_MessageDialog_setPlayerMessage(int year, int month,
									   int param1, int param2,
									   int messageAdvisor, int usePopup);

void UI_MessageDialog_drawBackground();
void UI_MessageDialog_drawForeground();
void UI_MessageDialog_handleMouse();

#endif

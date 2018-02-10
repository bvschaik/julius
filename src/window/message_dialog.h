#ifndef WINDOW_MESSAGE_DIALOG_H
#define WINDOW_MESSAGE_DIALOG_H

enum {
    MessageDialog_About = 0,
    MessageDialog_Help = 10,
    MessageDialog_EntertainmentAdvisor = 28,
    MessageDialog_EmpireMap = 32,
    MessageDialog_Messages = 34,
    MessageDialog_Industry = 46,
    MessageDialog_Theft = 251,
};

void window_message_dialog_show(int text_id, int background_is_provided);

void window_message_dialog_show_city_message(int text_id, int year, int month,
                                             int param1, int param2, int message_advisor, int use_popup);

#endif // WINDOW_MESSAGE_DIALOG_H

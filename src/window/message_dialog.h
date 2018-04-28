#ifndef WINDOW_MESSAGE_DIALOG_H
#define WINDOW_MESSAGE_DIALOG_H

enum {
    MESSAGE_DIALOG_ABOUT = 0,
    MESSAGE_DIALOG_HELP = 10,
    MESSAGE_DIALOG_ENTERTAINMENT_ADVISOR = 28,
    MESSAGE_DIALOG_EMPIRE_MAP = 32,
    MESSAGE_DIALOG_MESSAGES = 34,
    MESSAGE_DIALOG_INDUSTRY = 46,
    MESSAGE_DIALOG_THEFT = 251,
};

void window_message_dialog_show(int text_id, int background_is_provided);

void window_message_dialog_show_city_message(int text_id, int year, int month,
                                             int param1, int param2, int message_advisor, int use_popup);

#endif // WINDOW_MESSAGE_DIALOG_H

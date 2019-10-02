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
    MESSAGE_DIALOG_EDITOR_ABOUT = 331,
    MESSAGE_DIALOG_EDITOR_HELP = 332,
};

void window_message_dialog_show(int text_id, void (*background_callback)(void));

void window_message_dialog_show_city_message(int text_id, int year, int month,
                                             int param1, int param2, int message_advisor, int use_popup);

#endif // WINDOW_MESSAGE_DIALOG_H

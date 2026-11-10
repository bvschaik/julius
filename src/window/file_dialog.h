#ifndef WINDOW_FILE_DIALOG_H
#define WINDOW_FILE_DIALOG_H

typedef enum {
    FILE_DIALOG_SAVE = 0,
    FILE_DIALOG_LOAD = 1,
    FILE_DIALOG_DELETE = 2
} file_dialog_type;

typedef enum {
    FILE_TYPE_SAVED_GAME = 0,
    FILE_TYPE_SCENARIO = 1
} file_type;

void window_file_dialog_show(file_type type, file_dialog_type dialog_type);

/**
 * Shows the file dialog with the file name input pre-filled with the given name
 */
void window_file_dialog_show_with_name(file_type type, file_dialog_type dialog_type, const char *name_utf8);

#endif // WINDOW_FILE_DIALOG_H

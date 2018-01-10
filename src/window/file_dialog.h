#ifndef WINDOW_FILE_DIALOG_H
#define WINDOW_FILE_DIALOG_H

typedef enum {
    FILE_DIALOG_SAVE = 0,
    FILE_DIALOG_LOAD = 1,
    FILE_DIALOG_DELETE = 2
} file_dialog_type;

void window_file_dialog_show(file_dialog_type type);

#endif // WINDOW_FILE_DIALOG_H

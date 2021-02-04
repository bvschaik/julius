#ifndef WINDOW_LOGO_H
#define WINDOW_LOGO_H

enum {
    MESSAGE_NONE = 0,
    MESSAGE_MISSING_PATCH = 1,
    MESSAGE_MISSING_FONTS = 2,
    MESSAGE_MISSING_EXTRA_ASSETS = 3,
};

void window_logo_show(int show_patch_message);

#endif // WINDOW_LOGO_H

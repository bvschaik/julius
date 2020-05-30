#ifndef WIDGET_SIDEBAR_SLIDE_H
#define WIDGET_SIDEBAR_SLIDE_H

typedef void (*collapsed_draw_function)(void);
typedef void (*expanded_draw_function)(int x_offset);

void sidebar_slide(collapsed_draw_function collapsed_callback, expanded_draw_function expanded_callback);

#endif // WIDGET_SIDEBAR_SLIDE_H

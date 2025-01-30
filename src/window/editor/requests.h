#ifndef WINDOW_EDITOR_REQUESTS_H
#define WINDOW_EDITOR_REQUESTS_H

void window_editor_requests_show(void);

void window_editor_requests_show_with_callback(void (*on_select_callback)(int));

#endif // WINDOW_EDITOR_REQUESTS_H

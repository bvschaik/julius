#include "window.h"

int window_is(WindowId id)
{
    return UI_Window_getId() == id;
}

void window_invalidate()
{
    UI_Window_requestRefresh();
}

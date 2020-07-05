#ifndef WINDOW_RESOURCE_SETTINGS_H
#define WINDOW_RESOURCE_SETTINGS_H

#include "game/resource.h"

/**
    Display a dialog to modify a resources trading status

    @param resource which resource to manipulate
    @param override_draw_background_fn caller may provide their own background drawing function.
           this can be necessary as certain windows (like Help) can be larger than the dialog.
           if override_draw_background_fn is NULL, the standard trade advisor screen background
           will be shown instead
*/
void window_resource_settings_show(resource_type resource, void (*override_draw_background_fn)(void));

#endif // WINDOW_RESOURCE_SETTINGS_H

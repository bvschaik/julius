#include "platform/touch.h"

#include "core/time.h"
#include "game/settings.h"
#include "graphics/screen.h"
#include "input/touch.h"

static SDL_FingerID touch_id[MAX_ACTIVE_TOUCHES];

#ifdef __APPLE__
static SDL_TouchID trackpad_id;
#endif

static touch_coords get_touch_coordinates(float x, float y)
{
    touch_coords coords;
    coords.x = (int)(x * screen_width());
    coords.y = (int)(y * screen_height());
    return coords;
}

static int get_touch_index(SDL_FingerID id)
{
    for (int i = 0; i < MAX_ACTIVE_TOUCHES; ++i) {
        if (touch_id[i] == id) {
            return i;
        }
    }
    return MAX_ACTIVE_TOUCHES;
}

void platform_touch_start(SDL_TouchFingerEvent *event)
{
#ifdef __APPLE__
    // Attempt to disable trackpad touches on MacOS
    if (!trackpad_id) {
        trackpad_id = SDL_GetTouchDevice(0);
    }
    if (event->touchId == trackpad_id) {
        return;
    }
#endif
    int index = touch_create(get_touch_coordinates(event->x, event->y), event->timestamp);
    if (index != MAX_ACTIVE_TOUCHES) {
        touch_id[index] = event->fingerId;
    }
}

void platform_touch_update(SDL_TouchFingerEvent *event, int has_ended)
{
    touch_update(get_touch_index(event->fingerId), get_touch_coordinates(event->x, event->y), get_touch_coordinates(event->dx, event->dy), event->timestamp, has_ended);
}

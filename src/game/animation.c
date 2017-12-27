#include "animation.h"

#include "core/time.h"

#define MAX_ANIM_TIMERS 51

static struct {
    time_millis last_update;
    int should_update;
} timers[MAX_ANIM_TIMERS];

void game_animation_init()
{
    for (int i = 0; i < MAX_ANIM_TIMERS; i++) {
        timers[i].last_update = 0;
        timers[i].should_update = 0;
    }
}

void game_animation_update()
{
    time_millis now_millis = time_get_millis();
    for (int i = 0; i < MAX_ANIM_TIMERS; i++) {
        timers[i].should_update = 0;
    }
    int delay_millis = 0;
    for (int i = 0; i < MAX_ANIM_TIMERS; i++) {
        if (now_millis >= delay_millis + timers[i].last_update) {
            timers[i].should_update = 1;
            timers[i].last_update = now_millis;
        }
        delay_millis += 20;
    }
}

int game_animation_should_advance(int speed)
{
    return timers[speed].should_update;
}

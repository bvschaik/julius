#include "game.h"

#include "building/construction.h"
#include "building/model.h"
#include "core/image.h"
#include "core/lang.h"
#include "core/log.h"
#include "core/random.h"
#include "core/time.h"
#include "figure/type.h"
#include "game/animation.h"
#include "game/file.h"
#include "game/settings.h"
#include "game/state.h"
#include "game/system.h"
#include "game/tick.h"
#include "graphics/video.h"
#include "graphics/window.h"
#include "input/scroll.h"
#include "scenario/property.h"
#include "scenario/scenario.h"
#include "sound/city.h"
#include "sound/system.h"
#include "window/logo.h"

static const time_millis MILLIS_PER_TICK_PER_SPEED[] = {
    0, 20, 35, 55, 80, 110, 160, 240, 350, 500, 700
};

static time_millis last_update;

static void errlog(const char *msg)
{
    log_error(msg, 0, 0);
}

int game_pre_init()
{
    settings_load();
    scenario_settings_init();
    game_state_unpause();

    if (!lang_load("c3.eng", "c3_mm.eng")) {
        errlog("'c3.eng' or 'c3_mm.eng' files not found or too large.");
        return 0;
    }
    scenario_set_player_name(lang_get_string(9, 5));
    random_init();
    return 1;
}

int game_init()
{
    system_init_cursors();
    if (!image_init()) {
        errlog("unable to init graphics");
        return 0;
    }
    
    if (!image_load_climate(CLIMATE_CENTRAL)) {
        errlog("unable to load main graphics");
        return 0;
    }
    if (!image_load_enemy(ENEMY_0_BARBARIAN)) {
        errlog("unable to load enemy graphics");
        return 0;
    }

    if (!model_load()) {
        errlog("unable to load c3_model.txt");
        return 0;
    }

    sound_system_init();
    game_state_init();
    window_logo_show();
    return 1;
}

static int get_elapsed_ticks()
{
    time_millis now = time_get_millis();
    time_millis diff = now - last_update;

    int game_speed_index = (100 - setting_game_speed()) / 10;
    int ticks_per_frame = 1;
    if (game_speed_index >= 10) {
        return 0;
    } else if (game_speed_index < 0) {
        ticks_per_frame = setting_game_speed() / 100;
        game_speed_index = 0;
    }

    if (game_state_is_paused()) {
        return 0;
    }
    switch (window_get_id()) {
        default:
            return 0;
        case WINDOW_CITY:
        case WINDOW_CITY_MILITARY:
        case WINDOW_SLIDING_SIDEBAR:
        case WINDOW_OVERLAY_MENU:
            break;
    }
    if (building_construction_in_progress()) {
        return 0;
    }
    if (scroll_in_progress()) {
        return 0;
    }
    if (diff < MILLIS_PER_TICK_PER_SPEED[game_speed_index] + 2) {
        return 0;
    }
    last_update = now;
    return ticks_per_frame;
}

void game_run()
{
    game_animation_update();
    int num_ticks = get_elapsed_ticks();
    for (int i = 0; i < num_ticks; i++) {
        game_tick_run();
        game_file_write_mission_saved_game();
    }
}

void game_draw()
{
    window_draw(0);
    sound_city_play();
}

void game_exit()
{
    video_shutdown();
    settings_save();
    sound_system_shutdown();
}

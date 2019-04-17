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
#include "graphics/font.h"
#include "graphics/video.h"
#include "graphics/window.h"
#include "input/scroll.h"
#include "scenario/property.h"
#include "scenario/scenario.h"
#include "sound/city.h"
#include "sound/system.h"
#include "window/editor/map.h"
#include "window/logo.h"
#include "window/main_menu.h"

static const time_millis MILLIS_PER_TICK_PER_SPEED[] = {
    0, 20, 35, 55, 80, 110, 160, 240, 350, 500, 700
};

static time_millis last_update;

static void errlog(const char *msg)
{
    log_error(msg, 0, 0);
}

int game_pre_init(void)
{
    settings_load();
    scenario_settings_init();
    game_state_unpause();

    if (!lang_load("c3.eng", "c3_mm.eng") && !lang_load("c3.rus", "c3_mm.rus")) {
        errlog("'c3.eng' or 'c3_mm.eng' files not found or too large.");
        return 0;
    }
    encoding_type encoding = encoding_determine();
    log_info("Detected encoding:", 0, encoding);
    font_set_encoding(encoding);
    scenario_set_player_name(lang_get_string(9, 5));
    random_init();
    return 1;
}

static int has_patch()
{
    const uint8_t *difficulty_option = lang_get_string(2, 6);
    const uint8_t *help_menu = lang_get_string(3, 0);
    // Without patch, the difficulty option string does not exist and
    // getting it "falls through" to the next text group
    return difficulty_option != help_menu;
}

int game_init(void)
{
    int with_fonts = encoding_get() == ENCODING_CYRILLIC;
    system_init_cursors();
    if (!image_init(with_fonts)) {
        errlog("unable to init graphics");
        return GAME_INIT_ERROR;
    }
    
    if (!image_load_climate(CLIMATE_CENTRAL, 0)) {
        errlog("unable to load main graphics");
        return GAME_INIT_ERROR;
    }
    if (!image_load_enemy(ENEMY_0_BARBARIAN)) {
        errlog("unable to load enemy graphics");
        return GAME_INIT_ERROR;
    }
    if (with_fonts && !image_load_fonts()) {
        errlog("unable to load fonts graphics");
        return GAME_INIT_ERROR;
    }
    image_enable_fonts(with_fonts);

    if (!model_load()) {
        errlog("unable to load c3_model.txt");
        return GAME_INIT_ERROR;
    }

    sound_system_init();
    game_state_init();
    window_logo_show();

    return has_patch() ? GAME_INIT_OK : GAME_INIT_NO_PATCH;
}

int game_init_editor(void)
{
    if (!lang_load("c3_map.eng", "c3_map_mm.eng")) {
        errlog("'c3_map.eng' or 'c3_map_mm.eng' files not found or too large.");
        return 0;
    }
    encoding_type encoding = encoding_determine();
    log_info("Detected encoding:", 0, encoding);
    font_set_encoding(encoding);
    image_enable_fonts(encoding == ENCODING_CYRILLIC);

    if (!image_load_climate(CLIMATE_CENTRAL, 1)) {
        errlog("unable to load main graphics");
        return 0;
    }

    window_editor_map_show();
    return 1;
}

void game_exit_editor(void)
{
    if (!lang_load("c3.eng", "c3_mm.eng") && !lang_load("c3.rus", "c3_mm.rus")) {
        errlog("'c3.eng' or 'c3_mm.eng' files not found or too large.");
        return;
    }
    encoding_type encoding = encoding_determine();
    log_info("Detected encoding:", 0, encoding);
    font_set_encoding(encoding);
    image_enable_fonts(encoding == ENCODING_CYRILLIC);

    if (!image_load_climate(CLIMATE_CENTRAL, 0)) {
        errlog("unable to load main graphics");
        return;
    }

    window_main_menu_show();
}

static int get_elapsed_ticks(void)
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
        case WINDOW_BUILD_MENU:
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

void game_run(void)
{
    game_animation_update();
    int num_ticks = get_elapsed_ticks();
    for (int i = 0; i < num_ticks; i++) {
        game_tick_run();
        game_file_write_mission_saved_game();

        if (window_is_invalid()) {
            break;
        }
    }
}

void game_draw(void)
{
    window_draw(0);
    sound_city_play();
}

void game_exit(void)
{
    video_shutdown();
    settings_save();
    sound_system_shutdown();
}

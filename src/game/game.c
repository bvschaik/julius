#include "game.h"

#include "assets/assets.h"
#include "building/model.h"
#include "building/properties.h"
#include "city/view.h"
#include "core/config.h"
#include "core/hotkey_config.h"
#include "core/image.h"
#include "core/lang.h"
#include "core/locale.h"
#include "core/log.h"
#include "core/random.h"
#include "editor/editor.h"
#include "figure/type.h"
#include "game/animation.h"
#include "game/file.h"
#include "game/file_editor.h"
#include "game/settings.h"
#include "game/speed.h"
#include "game/state.h"
#include "game/tick.h"
#include "graphics/font.h"
#include "graphics/video.h"
#include "graphics/window.h"
#include "scenario/property.h"
#include "scenario/scenario.h"
#include "sound/city.h"
#include "sound/system.h"
#include "translation/translation.h"
#include "window/editor/map.h"
#include "window/logo.h"
#include "window/main_menu.h"

static void errlog(const char *msg)
{
    log_error(msg, 0, 0);
}

static encoding_type update_encoding(void)
{
    language_type language = locale_determine_language();
    encoding_type encoding = encoding_determine(language);
    log_info("Detected encoding:", 0, encoding);
    font_set_encoding(encoding);
    translation_load(language);
    return encoding;
}

int game_pre_init(void)
{
    settings_load();
    config_load();
    hotkey_config_load();
    scenario_settings_init();
    game_state_unpause();

    if (!lang_load(0)) {
        errlog("'c3.eng' or 'c3_mm.eng' files not found or too large.");
        return 0;
    }
    update_encoding();
    random_init();
    return 1;
}

static int is_unpatched(void)
{
    const uint8_t *delete_game = lang_get_string(1, 6);
    const uint8_t *option_menu = lang_get_string(2, 0);
    const uint8_t *difficulty_option = lang_get_string(2, 6);
    const uint8_t *help_menu = lang_get_string(3, 0);
    // Without patch, the difficulty option string does not exist and
    // getting it "falls through" to the next text group, or, for some
    // languages (pt_BR): delete game falls through to option menu
    return difficulty_option == help_menu || delete_game == option_menu;
}

int game_init(void)
{
    if (!image_load_climate(CLIMATE_CENTRAL, 0, 1, 0)) {
        errlog("unable to load main graphics");
        return 0;
    }
    if (!image_load_enemy(ENEMY_0_BARBARIAN)) {
        errlog("unable to load enemy graphics");
        return 0;
    }
    int missing_fonts = 0;
    if (!image_load_fonts(encoding_get())) {
        errlog("unable to load font graphics");
        if (encoding_get() == ENCODING_KOREAN || encoding_get() == ENCODING_JAPANESE) {
            missing_fonts = 1;
        } else {
            return 0;
        }
    }

    if (!model_load()) {
        errlog("unable to load c3_model.txt");
        return 0;
    }

    init_augustus_building_properties();
    load_augustus_messages();
    sound_system_init();
    game_state_init();
    resource_init();
    int missing_assets = !assets_get_image_id("Logistics", "roadblock"); // If can't find roadblocks asset, extra assets not installed properly
    window_logo_show(missing_fonts ? MESSAGE_MISSING_FONTS : (is_unpatched() ? MESSAGE_MISSING_PATCH : (missing_assets ? MESSAGE_MISSING_EXTRA_ASSETS : MESSAGE_NONE)));
    return 1;
}

static int reload_language(int is_editor, int reload_images)
{
    if (!lang_load(is_editor)) {
        if (is_editor) {
            errlog("'c3_map.eng' or 'c3_map_mm.eng' files not found or too large.");
        } else {
            errlog("'c3.eng' or 'c3_mm.eng' files not found or too large.");
        }
        return 0;
    }
    encoding_type encoding = update_encoding();
    if (!is_editor) {
        load_augustus_messages();
    }

    if (!image_load_fonts(encoding)) {
        errlog("unable to load font graphics");
        return 0;
    }
    if (!image_load_climate(scenario_property_climate(), is_editor, reload_images, 0)) {
        errlog("unable to load main graphics");
        return 0;
    }

    resource_init();

    return 1;
}

int game_init_editor(void)
{
    if (!reload_language(1, 0)) {
        return 0;
    }

    game_file_editor_clear_data();
    game_file_editor_create_scenario(2);

    if (city_view_is_sidebar_collapsed()) {
        city_view_toggle_sidebar();
    }

    editor_set_active(1);
    window_editor_map_show();
    return 1;
}

void game_exit_editor(void)
{
    if (!reload_language(0, 0)) {
        return;
    }
    editor_set_active(0);
    window_main_menu_show(1);
}

int game_reload_language(void)
{
    return reload_language(0, 1);
}

void game_run(void)
{
    game_animation_update();
    int num_ticks = game_speed_get_elapsed_ticks();
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
    config_save();
    sound_system_shutdown();
}

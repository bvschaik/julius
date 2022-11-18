#include "config.h"

#include "core/calc.h"
#include "core/config.h"
#include "core/dir.h"
#include "core/image_group.h"
#include "core/lang.h"
#include "core/log.h"
#include "core/string.h"
#include "game/game.h"
#include "game/settings.h"
#include "game/system.h"
#include "graphics/button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/panel.h"
#include "graphics/screen.h"
#include "graphics/scrollbar.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "sound/city.h"
#include "sound/effect.h"
#include "sound/music.h"
#include "sound/speech.h"
#include "window/hotkey_config.h"
#include "window/main_menu.h"
#include "window/numeric_input.h"
#include "window/plain_message_dialog.h"
#include "window/select_list.h"
#include "translation/translation.h"

#include <stdlib.h>
#include <string.h>

#define NUM_CHECKBOXES 28
#define MAX_LANGUAGE_DIRS 20

#define FIRST_BUTTON_Y 72
#define BUTTON_SPACING 24
#define TEXT_Y_OFFSET 4

#define MAX_WIDGETS 30

#define NUM_VISIBLE_ITEMS 13

#define ITEM_Y_OFFSET 100
#define ITEM_HEIGHT 24

#define NUM_BOTTOM_BUTTONS 5
#define MAX_LANGUAGE_DIRS 20
#define CHECKBOX_CHECK_SIZE 20
#define CHECKBOX_HEIGHT 20
#define CHECKBOX_WIDTH 560
#define CHECKBOX_TEXT_WIDTH CHECKBOX_WIDTH - CHECKBOX_CHECK_SIZE - 15

#define NUMERICAL_RANGE_X 20
#define NUMERICAL_SLIDER_PADDING 2
#define NUMERICAL_DOT_SIZE 20

static void on_scroll(void);

static void toggle_switch(int key);
static void button_language_select(int param1, int param2);
static void button_reset_defaults(int param1, int param2);
static void button_hotkeys(int param1, int param2);
static void button_close(int save, int param2);
static void button_page(int page, int param2);

static const uint8_t *display_text_language(void);
static const uint8_t *display_text_game_speed(void);
static const uint8_t *display_text_resolution(void);
static const uint8_t *display_text_display_scale(void);
static const uint8_t *display_text_cursor_scale(void);
static const uint8_t *display_text_master_volume(void);
static const uint8_t *display_text_music_volume(void);
static const uint8_t *display_text_speech_volume(void);
static const uint8_t *display_text_sound_effects_volume(void);
static const uint8_t *display_text_city_sounds_volume(void);
static const uint8_t *display_text_video_volume(void);
static const uint8_t *display_text_scroll_speed(void);
static const uint8_t *display_text_difficulty(void);
static const uint8_t *display_text_max_grand_temples(void);

static scrollbar_type scrollbar = {
    580, ITEM_Y_OFFSET, ITEM_HEIGHT * NUM_VISIBLE_ITEMS, CHECKBOX_WIDTH, NUM_VISIBLE_ITEMS, on_scroll, 0, 4
};

enum {
    TYPE_NONE,
    TYPE_SPACE,
    TYPE_HEADER,
    TYPE_CHECKBOX,
    TYPE_SELECT,
    TYPE_NUMERICAL_DESC,
    TYPE_NUMERICAL_RANGE
};

enum {
    SELECT_LANGUAGE
};

enum {
    RANGE_GAME_SPEED,
    RANGE_RESOLUTION,
    RANGE_DISPLAY_SCALE,
    RANGE_CURSOR_SCALE,
    RANGE_MASTER_VOLUME,
    RANGE_MUSIC_VOLUME,
    RANGE_SPEECH_VOLUME,
    RANGE_SOUND_EFFECTS_VOLUME,
    RANGE_CITY_SOUNDS_VOLUME,
    RANGE_VIDEO_VOLUME,
    RANGE_SCROLL_SPEED,
    RANGE_DIFFICULTY,
    RANGE_MAX_GRAND_TEMPLES
};

enum {
    CONFIG_ORIGINAL_GAME_SPEED = CONFIG_MAX_ENTRIES,
    CONFIG_ORIGINAL_FULLSCREEN,
    CONFIG_ORIGINAL_WINDOWED_RESOLUTION,
    CONFIG_ORIGINAL_ENABLE_MUSIC,
    CONFIG_ORIGINAL_MUSIC_VOLUME,
    CONFIG_ORIGINAL_ENABLE_SPEECH,
    CONFIG_ORIGINAL_SPEECH_VOLUME,
    CONFIG_ORIGINAL_ENABLE_SOUND_EFFECTS,
    CONFIG_ORIGINAL_SOUND_EFFECTS_VOLUME,
    CONFIG_ORIGINAL_ENABLE_CITY_SOUNDS,
    CONFIG_ORIGINAL_CITY_SOUNDS_VOLUME,
    CONFIG_ORIGINAL_SCROLL_SPEED,
    CONFIG_ORIGINAL_DIFFICULTY,
    CONFIG_ORIGINAL_GODS_EFFECTS,
    CONFIG_MAX_ALL
};

typedef struct {
    int width;
    int height;
} resolution;

typedef struct {
    int x;
    int width_blocks;
    int min;
    int max;
    int step;
    int *value;
} numerical_range_widget;

typedef struct {
    int type;
    int subtype;
    translation_key description;
    const uint8_t *(*get_display_text)(void);
    int y_offset;
    int enabled;
} config_widget;

static uint8_t display_text[10];
static uint8_t volume_text[64];
static uint8_t *volume_offset;

static config_widget all_widgets[CONFIG_PAGES][MAX_WIDGETS] = {
    { // General Settings
        {TYPE_SELECT, SELECT_LANGUAGE, TR_CONFIG_LANGUAGE_LABEL, display_text_language},
        {TYPE_SPACE},
        {TYPE_NUMERICAL_DESC, RANGE_GAME_SPEED, TR_CONFIG_GAME_SPEED},
        {TYPE_NUMERICAL_RANGE, RANGE_GAME_SPEED, 0, display_text_game_speed},
        {TYPE_SPACE, TR_CONFIG_VIDEO},
        {TYPE_HEADER, TR_CONFIG_VIDEO},
        {TYPE_CHECKBOX, CONFIG_ORIGINAL_FULLSCREEN, TR_CONFIG_FULLSCREEN },
        {TYPE_NUMERICAL_DESC, RANGE_RESOLUTION, TR_CONFIG_WINDOWED_RESOLUTION},
        {TYPE_NUMERICAL_RANGE, RANGE_RESOLUTION, 0, display_text_resolution},
        {TYPE_NUMERICAL_DESC, RANGE_DISPLAY_SCALE, TR_CONFIG_DISPLAY_SCALE},
        {TYPE_NUMERICAL_RANGE, RANGE_DISPLAY_SCALE, 0, display_text_display_scale},
        {TYPE_NUMERICAL_DESC, RANGE_CURSOR_SCALE, TR_CONFIG_CURSOR_SCALE},
        {TYPE_NUMERICAL_RANGE, RANGE_CURSOR_SCALE, 0, display_text_cursor_scale},
        {TYPE_CHECKBOX, CONFIG_SCREEN_COLOR_CURSORS, TR_CONFIG_USE_COLOR_CURSORS, 0, 5},
        {TYPE_SPACE, TR_CONFIG_AUDIO},
        {TYPE_HEADER, TR_CONFIG_AUDIO, 0, 0, 5},
        {TYPE_CHECKBOX, CONFIG_GENERAL_ENABLE_AUDIO, TR_CONFIG_ENABLE_AUDIO, 0, 5},
        {TYPE_NUMERICAL_RANGE, RANGE_MASTER_VOLUME, 0, display_text_master_volume, 1},
        {TYPE_CHECKBOX, CONFIG_ORIGINAL_ENABLE_MUSIC, TR_CONFIG_MUSIC, 0, 5},
        {TYPE_NUMERICAL_RANGE, RANGE_MUSIC_VOLUME, 0, display_text_music_volume, 1},
        {TYPE_CHECKBOX, CONFIG_ORIGINAL_ENABLE_SPEECH, TR_CONFIG_SPEECH, 0, 5},
        {TYPE_NUMERICAL_RANGE, RANGE_SPEECH_VOLUME, 0, display_text_speech_volume, 1},
        {TYPE_CHECKBOX, CONFIG_ORIGINAL_ENABLE_SOUND_EFFECTS, TR_CONFIG_EFFECTS, 0, 5},
        {TYPE_NUMERICAL_RANGE, RANGE_SOUND_EFFECTS_VOLUME, 0, display_text_sound_effects_volume, 1},
        {TYPE_CHECKBOX, CONFIG_ORIGINAL_ENABLE_CITY_SOUNDS, TR_CONFIG_CITY_SOUNDS, 0, 5},
        {TYPE_NUMERICAL_RANGE, RANGE_CITY_SOUNDS_VOLUME, 0, display_text_city_sounds_volume, 1},
        {TYPE_CHECKBOX, CONFIG_GENERAL_ENABLE_VIDEO_SOUND, TR_CONFIG_VIDEO_SOUND, 0, 5},
        {TYPE_NUMERICAL_RANGE, RANGE_VIDEO_VOLUME, 0, display_text_video_volume, 1}
    },
    { // UI
        {TYPE_NUMERICAL_DESC, RANGE_SCROLL_SPEED, TR_CONFIG_SCROLL_SPEED},
        {TYPE_NUMERICAL_RANGE, RANGE_SCROLL_SPEED, 0, display_text_scroll_speed},
        {TYPE_SPACE},
        {TYPE_CHECKBOX, CONFIG_UI_SHOW_INTRO_VIDEO, TR_CONFIG_SHOW_INTRO_VIDEO},
        {TYPE_CHECKBOX, CONFIG_UI_SIDEBAR_INFO, TR_CONFIG_SIDEBAR_INFO},
        {TYPE_CHECKBOX, CONFIG_UI_SMOOTH_SCROLLING, TR_CONFIG_SMOOTH_SCROLLING},
        {TYPE_CHECKBOX, CONFIG_UI_DISABLE_MOUSE_EDGE_SCROLLING, TR_CONFIG_DISABLE_MOUSE_EDGE_SCROLLING},
        {TYPE_CHECKBOX, CONFIG_UI_SHOW_WATER_STRUCTURE_RANGE, TR_CONFIG_SHOW_WATER_STRUCTURE_RANGE},
        {TYPE_CHECKBOX, CONFIG_UI_SHOW_WATER_STRUCTURE_RANGE_HOUSES, TR_CONFIG_SHOW_WATER_STRUCTURE_RANGE_HOUSES},
        {TYPE_CHECKBOX, CONFIG_UI_SHOW_CONSTRUCTION_SIZE, TR_CONFIG_SHOW_CONSTRUCTION_SIZE},
        {TYPE_CHECKBOX, CONFIG_UI_HIGHLIGHT_LEGIONS, TR_CONFIG_HIGHLIGHT_LEGIONS},
        {TYPE_CHECKBOX, CONFIG_UI_SHOW_MILITARY_SIDEBAR, TR_CONFIG_SHOW_MILITARY_SIDEBAR},
        {TYPE_CHECKBOX, CONFIG_UI_DISABLE_RIGHT_CLICK_MAP_DRAG, TR_CONFIG_DISABLE_RIGHT_CLICK_MAP_DRAG},
        {TYPE_CHECKBOX, CONFIG_UI_SHOW_MAX_PROSPERITY, TR_CONFIG_SHOW_MAX_POSSIBLE_PROSPERITY},
        {TYPE_CHECKBOX, CONFIG_UI_DIGIT_SEPARATOR, TR_CONFIG_DIGIT_SEPARATOR},
        {TYPE_CHECKBOX, CONFIG_UI_INVERSE_MAP_DRAG, TR_CONFIG_UI_INVERSE_MAP_DRAG},
        {TYPE_CHECKBOX, CONFIG_UI_MESSAGE_ALERTS, TR_CONFIG_UI_MESSAGE_ALERTS},
        {TYPE_CHECKBOX, CONFIG_UI_SHOW_GRID, TR_CONFIG_UI_SHOW_GRID},
        {TYPE_CHECKBOX, CONFIG_UI_SHOW_PARTIAL_GRID_AROUND_CONSTRUCTION, TR_CONFIG_UI_SHOW_PARTIAL_GRID_AROUND_CONSTRUCTION},
        {TYPE_CHECKBOX, CONFIG_UI_ALWAYS_SHOW_ROTATION_BUTTONS, TR_CONFIG_UI_ALWAYS_SHOW_ROTATION_BUTTONS},
        {TYPE_CHECKBOX, CONFIG_UI_SHOW_ROAMING_PATH, TR_CONFIG_SHOW_ROAMING_PATH },
    },
    { // Difficulty
        {TYPE_NUMERICAL_DESC, RANGE_DIFFICULTY, TR_CONFIG_DIFFICULTY},
        {TYPE_NUMERICAL_RANGE, RANGE_DIFFICULTY, 0, display_text_difficulty},
        {TYPE_SPACE},
        {TYPE_CHECKBOX, CONFIG_ORIGINAL_GODS_EFFECTS, TR_CONFIG_GODS_EFFECTS },
        {TYPE_CHECKBOX, CONFIG_GP_CH_JEALOUS_GODS, TR_CONFIG_JEALOUS_GODS },
        {TYPE_CHECKBOX, CONFIG_GP_CH_GLOBAL_LABOUR, TR_CONFIG_GLOBAL_LABOUR },
        {TYPE_CHECKBOX, CONFIG_GP_CH_RETIRE_AT_60, TR_CONFIG_RETIRE_AT_60 },
        {TYPE_CHECKBOX, CONFIG_GP_CH_FIXED_WORKERS, TR_CONFIG_FIXED_WORKERS },
        {TYPE_CHECKBOX, CONFIG_GP_CH_WOLVES_BLOCK, TR_CONFIG_WOLVES_BLOCK },
        {TYPE_CHECKBOX, CONFIG_GP_CH_MULTIPLE_BARRACKS, TR_CONFIG_MULTIPLE_BARRACKS },
        {TYPE_CHECKBOX, CONFIG_GP_CH_DISABLE_INFINITE_WOLVES_SPAWNING, TR_CONFIG_GP_CH_DISABLE_INFINITE_WOLVES_SPAWNING },
        {TYPE_NUMERICAL_DESC, RANGE_MAX_GRAND_TEMPLES, TR_CONFIG_MAX_GRAND_TEMPLES},
        {TYPE_NUMERICAL_RANGE, RANGE_MAX_GRAND_TEMPLES, 0, display_text_max_grand_temples},
    },
    { // City Management
        {TYPE_CHECKBOX, CONFIG_GP_CH_NO_SUPPLIER_DISTRIBUTION, TR_CONFIG_NO_SUPPLIER_DISTRIBUTION },
        {TYPE_CHECKBOX, CONFIG_GP_CH_GETTING_GRANARIES_GO_OFFROAD, TR_CONFIG_GETTING_GRANARIES_GO_OFFROAD },
        {TYPE_CHECKBOX, CONFIG_GP_CH_GRANARIES_GET_DOUBLE, TR_CONFIG_GRANARIES_GET_DOUBLE },
        {TYPE_CHECKBOX, CONFIG_GP_CH_ALLOW_EXPORTING_FROM_GRANARIES, TR_CONFIG_ALLOW_EXPORTING_FROM_GRANARIES },
        {TYPE_CHECKBOX, CONFIG_GP_CH_TOWER_SENTRIES_GO_OFFROAD, TR_CONFIG_TOWER_SENTRIES_GO_OFFROAD },
        {TYPE_CHECKBOX, CONFIG_GP_CH_FARMS_DELIVER_CLOSE, TR_CONFIG_FARMS_DELIVER_CLOSE },
        {TYPE_CHECKBOX, CONFIG_GP_CH_DELIVER_ONLY_TO_ACCEPTING_GRANARIES, TR_CONFIG_DELIVER_ONLY_TO_ACCEPTING_GRANARIES },
        {TYPE_CHECKBOX, CONFIG_GP_CH_ALL_HOUSES_MERGE, TR_CONFIG_ALL_HOUSES_MERGE },
        {TYPE_CHECKBOX, CONFIG_GP_CH_RANDOM_COLLAPSES_TAKE_MONEY, TR_CONFIG_RANDOM_COLLAPSES_TAKE_MONEY },
        {TYPE_CHECKBOX, CONFIG_GP_CH_WAREHOUSES_DONT_ACCEPT, TR_CONFIG_NOT_ACCEPTING_WAREHOUSES },
        {TYPE_CHECKBOX, CONFIG_GP_CH_HOUSES_DONT_EXPAND_INTO_GARDENS, TR_CONFIG_HOUSES_DONT_EXPAND_INTO_GARDENS },
        {TYPE_CHECKBOX, CONFIG_GP_CH_ROAMERS_DONT_SKIP_CORNERS, TR_CONFIG_ROAMERS_DONT_SKIP_CORNERS },
        {TYPE_CHECKBOX, CONFIG_GP_CH_AUTO_KILL_ANIMALS, TR_CONFIG_AUTO_KILL_ANIMALS},
    }
};

static const resolution resolutions[] = {
    {  640,  480 }, {  800,  600 }, { 1024,  768 }, { 1280,  720 },
    { 1280,  800 }, { 1280, 1024 }, { 1360,  768 }, { 1366,  768 },
    { 1440,  900 }, { 1536,  864 }, { 1600,  900 }, { 1680, 1050 },
    { 1920, 1080 }, { 1920, 1200 }, { 2048, 1152 }, { 2560, 1080 },
    { 2560, 1440 }, { 3440, 1440 }, { 3840, 2160 }
};

static const int game_speeds[] = { 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 200, 300, 400, 500 };

static resolution available_resolutions[sizeof(resolutions) / sizeof(resolution) + 2];

static generic_button select_buttons[] = {
    {150, 0, 200, 24, button_language_select, button_none},
};

static numerical_range_widget ranges[] = {
    { 50, 30,   0,  13,  1, 0},
    { 98, 27,   0,   0,  1, 0},
    { 50, 30,  50, 500,  5, 0},
    { 50, 30, 100, 200, 50, 0},
    {130, 25,   0, 100,  1, 0},
    {130, 25,   0, 100,  1, 0},
    {130, 25,   0, 100,  1, 0},
    {130, 25,   0, 100,  1, 0},
    {130, 25,   0, 100,  1, 0},
    {130, 25,   0, 100,  1, 0},
    { 50, 30,   0, 100, 10, 0},
    {146, 24,   0,   4,  1, 0},
    { 50, 30,   0,   5,  1, 0}
};

static generic_button bottom_buttons[NUM_BOTTOM_BUTTONS] = {
    {  20, 436,  120, 30, button_hotkeys, button_none, 0, TR_BUTTON_CONFIGURE_HOTKEYS },
    { 170, 436, 150, 30, button_reset_defaults, button_none, 0, TR_BUTTON_RESET_DEFAULTS },
    { 330, 436,  90, 30, button_close, button_none, 0, TR_BUTTON_CANCEL },
    { 430, 436,  90, 30, button_close, button_none, 1, TR_BUTTON_OK },
    { 530, 436,  90, 30, button_close, button_none, 2, TR_OPTION_MENU_APPLY }
};

static generic_button page_buttons[] = {
    { 0, 48, 0, 30, button_page, button_none, 0 },
    { 0, 48, 0, 30, button_page, button_none, 1 },
    { 0, 48, 0, 30, button_page, button_none, 2 },
    { 0, 48, 0, 30, button_page, button_none, 3 },
};

static translation_key page_names[CONFIG_PAGES] = {
    TR_CONFIG_HEADER_GENERAL,
    TR_CONFIG_HEADER_UI_CHANGES,
    TR_CONFIG_HEADER_GAMEPLAY_CHANGES,
    TR_CONFIG_HEADER_CITY_MANAGEMENT_CHANGES
};

static struct {
    config_widget *widgets[MAX_WIDGETS * CONFIG_PAGES];
    int num_widgets;
    int focus_button;
    int bottom_focus_button;
    int page_focus_button;
    int page;
    int widgets_per_page[CONFIG_PAGES];
    int starting_option;
    struct {
        int original_value;
        int new_value;
        int (*change_action)(config_key key);
    } config_values[CONFIG_MAX_ALL];
    struct {
        char original_value[CONFIG_STRING_VALUE_MAX];
        char new_value[CONFIG_STRING_VALUE_MAX];
        int (*change_action)(config_string_key key);
    } config_string_values[CONFIG_STRING_MAX_ENTRIES];
    uint8_t language_options_data[MAX_LANGUAGE_DIRS][CONFIG_STRING_VALUE_MAX];
    const uint8_t *language_options[MAX_LANGUAGE_DIRS];
    char language_options_utf8[MAX_LANGUAGE_DIRS][CONFIG_STRING_VALUE_MAX];
    int num_language_options;
    int selected_language_option;
    int active_numerical_range;
    int show_background_image;
    int has_changes;
    int reload_cursors;
    int graphics_behind_tab[CONFIG_PAGES];
} data;

static int config_change_basic(config_key key);
static int config_change_string_basic(config_string_key key);

static int config_change_string_language(config_string_key key);

static int config_change_game_speed(config_key key);
static int config_change_fullscreen(config_key key);
static int config_change_display_resolution(config_key key);
static int config_change_display_scale(config_key key);
static int config_change_cursors(config_key key);

static int config_enable_audio(config_key key);
static int config_set_master_volume(config_key key);
static int config_enable_music(config_key key);
static int config_set_music_volume(config_key key);
static int config_enable_speech(config_key key);
static int config_set_speech_volume(config_key key);
static int config_enable_effects(config_key key);
static int config_set_effects_volume(config_key key);
static int config_enable_city_sounds(config_key key);
static int config_set_city_sounds_volume(config_key key);

static int config_change_scroll_speed(config_key key);

static int config_set_difficulty(config_key key);
static int config_enable_gods_effects(config_key key);

static inline void set_custom_config_changes(void)
{
    data.config_string_values[CONFIG_STRING_UI_LANGUAGE_DIR].change_action = config_change_string_language;

    data.config_values[CONFIG_ORIGINAL_FULLSCREEN].change_action = config_change_fullscreen;
    data.config_values[CONFIG_ORIGINAL_WINDOWED_RESOLUTION].change_action = config_change_display_resolution;
    data.config_values[CONFIG_SCREEN_DISPLAY_SCALE].change_action = config_change_display_scale;
    data.config_values[CONFIG_SCREEN_CURSOR_SCALE].change_action = config_change_cursors;
    data.config_values[CONFIG_SCREEN_COLOR_CURSORS].change_action = config_change_cursors;

    data.config_values[CONFIG_ORIGINAL_GAME_SPEED].change_action = config_change_game_speed;
    data.config_values[CONFIG_GENERAL_ENABLE_AUDIO].change_action = config_enable_audio;
    data.config_values[CONFIG_GENERAL_MASTER_VOLUME].change_action = config_set_master_volume;
    data.config_values[CONFIG_ORIGINAL_ENABLE_MUSIC].change_action = config_enable_music;
    data.config_values[CONFIG_ORIGINAL_MUSIC_VOLUME].change_action = config_set_music_volume;
    data.config_values[CONFIG_ORIGINAL_ENABLE_SPEECH].change_action = config_enable_speech;
    data.config_values[CONFIG_ORIGINAL_SPEECH_VOLUME].change_action = config_set_speech_volume;
    data.config_values[CONFIG_ORIGINAL_ENABLE_SOUND_EFFECTS].change_action = config_enable_effects;
    data.config_values[CONFIG_ORIGINAL_SOUND_EFFECTS_VOLUME].change_action = config_set_effects_volume;
    data.config_values[CONFIG_ORIGINAL_ENABLE_CITY_SOUNDS].change_action = config_enable_city_sounds;
    data.config_values[CONFIG_ORIGINAL_CITY_SOUNDS_VOLUME].change_action = config_set_city_sounds_volume;

    data.config_values[CONFIG_ORIGINAL_SCROLL_SPEED].change_action = config_change_scroll_speed;

    data.config_values[CONFIG_ORIGINAL_DIFFICULTY].change_action = config_set_difficulty;
    data.config_values[CONFIG_ORIGINAL_GODS_EFFECTS].change_action = config_enable_gods_effects;
}

static inline void set_range_values(void)
{
    ranges[RANGE_GAME_SPEED].value = &data.config_values[CONFIG_ORIGINAL_GAME_SPEED].new_value;
    ranges[RANGE_RESOLUTION].value = &data.config_values[CONFIG_ORIGINAL_WINDOWED_RESOLUTION].new_value;
    ranges[RANGE_DISPLAY_SCALE].value = &data.config_values[CONFIG_SCREEN_DISPLAY_SCALE].new_value;
    ranges[RANGE_CURSOR_SCALE].value = &data.config_values[CONFIG_SCREEN_CURSOR_SCALE].new_value;

    ranges[RANGE_MASTER_VOLUME].value = &data.config_values[CONFIG_GENERAL_MASTER_VOLUME].new_value;
    ranges[RANGE_MUSIC_VOLUME].value = &data.config_values[CONFIG_ORIGINAL_MUSIC_VOLUME].new_value;
    ranges[RANGE_SPEECH_VOLUME].value = &data.config_values[CONFIG_ORIGINAL_SPEECH_VOLUME].new_value;
    ranges[RANGE_SOUND_EFFECTS_VOLUME].value = &data.config_values[CONFIG_ORIGINAL_SOUND_EFFECTS_VOLUME].new_value;
    ranges[RANGE_CITY_SOUNDS_VOLUME].value = &data.config_values[CONFIG_ORIGINAL_CITY_SOUNDS_VOLUME].new_value;
    ranges[RANGE_VIDEO_VOLUME].value = &data.config_values[CONFIG_GENERAL_VIDEO_VOLUME].new_value;

    ranges[RANGE_SCROLL_SPEED].value = &data.config_values[CONFIG_ORIGINAL_SCROLL_SPEED].new_value;
    ranges[RANGE_DIFFICULTY].value = &data.config_values[CONFIG_ORIGINAL_DIFFICULTY].new_value;

    ranges[RANGE_MAX_GRAND_TEMPLES].value = &data.config_values[CONFIG_GP_CH_MAX_GRAND_TEMPLES].new_value;
}

static inline void fetch_original_config_values(void)
{
    int game_speed_index = 0;
    while (game_speed_index < sizeof(game_speeds) / sizeof(int)) {
        if (setting_game_speed() == game_speeds[game_speed_index]) {
            break;
        }
        game_speed_index++;
    }
    data.config_values[CONFIG_ORIGINAL_GAME_SPEED].original_value = game_speed_index;
    data.config_values[CONFIG_ORIGINAL_GAME_SPEED].new_value = game_speed_index;

    data.config_values[CONFIG_ORIGINAL_ENABLE_MUSIC].original_value = setting_sound(SOUND_MUSIC)->enabled;
    data.config_values[CONFIG_ORIGINAL_ENABLE_MUSIC].new_value = setting_sound(SOUND_MUSIC)->enabled;
    data.config_values[CONFIG_ORIGINAL_MUSIC_VOLUME].original_value = setting_sound(SOUND_MUSIC)->volume;
    data.config_values[CONFIG_ORIGINAL_MUSIC_VOLUME].new_value = setting_sound(SOUND_MUSIC)->volume;

    data.config_values[CONFIG_ORIGINAL_ENABLE_SPEECH].original_value = setting_sound(SOUND_SPEECH)->enabled;
    data.config_values[CONFIG_ORIGINAL_ENABLE_SPEECH].new_value = setting_sound(SOUND_SPEECH)->enabled;
    data.config_values[CONFIG_ORIGINAL_SPEECH_VOLUME].original_value = setting_sound(SOUND_SPEECH)->volume;
    data.config_values[CONFIG_ORIGINAL_SPEECH_VOLUME].new_value = setting_sound(SOUND_SPEECH)->volume;

    data.config_values[CONFIG_ORIGINAL_ENABLE_SOUND_EFFECTS].original_value = setting_sound(SOUND_EFFECTS)->enabled;
    data.config_values[CONFIG_ORIGINAL_ENABLE_SOUND_EFFECTS].new_value = setting_sound(SOUND_EFFECTS)->enabled;
    data.config_values[CONFIG_ORIGINAL_SOUND_EFFECTS_VOLUME].original_value = setting_sound(SOUND_EFFECTS)->volume;
    data.config_values[CONFIG_ORIGINAL_SOUND_EFFECTS_VOLUME].new_value = setting_sound(SOUND_EFFECTS)->volume;

    data.config_values[CONFIG_ORIGINAL_ENABLE_CITY_SOUNDS].original_value = setting_sound(SOUND_CITY)->enabled;
    data.config_values[CONFIG_ORIGINAL_ENABLE_CITY_SOUNDS].new_value = setting_sound(SOUND_CITY)->enabled;
    data.config_values[CONFIG_ORIGINAL_CITY_SOUNDS_VOLUME].original_value = setting_sound(SOUND_CITY)->volume;
    data.config_values[CONFIG_ORIGINAL_CITY_SOUNDS_VOLUME].new_value = setting_sound(SOUND_CITY)->volume;

    data.config_values[CONFIG_ORIGINAL_SCROLL_SPEED].original_value = setting_scroll_speed();
    data.config_values[CONFIG_ORIGINAL_SCROLL_SPEED].new_value = setting_scroll_speed();

    data.config_values[CONFIG_ORIGINAL_DIFFICULTY].original_value = setting_difficulty();
    data.config_values[CONFIG_ORIGINAL_DIFFICULTY].new_value = setting_difficulty();

    data.config_values[CONFIG_ORIGINAL_GODS_EFFECTS].original_value = setting_gods_enabled();
    data.config_values[CONFIG_ORIGINAL_GODS_EFFECTS].new_value = setting_gods_enabled();
}

static void init_config_values(void)
{
    for (int i = 0; i < CONFIG_MAX_ALL; ++i) {
        data.config_values[i].change_action = config_change_basic;
    }
    for (int i = 0; i < CONFIG_STRING_MAX_ENTRIES; ++i) {
        data.config_string_values[i].change_action = config_change_string_basic;
    }
    set_custom_config_changes();
    set_range_values();
}

static void enable_all_widgets(void)
{
    for (int p = 0; p < CONFIG_PAGES; p++) {
        for (int i = 0; i < MAX_WIDGETS; i++) {
            if (all_widgets[p][i].type) {
                all_widgets[p][i].enabled = 1;
            }
        }
    }
}

static void disable_widget(int type, int subtype)
{
    for (int p = 0; p < CONFIG_PAGES; p++) {
        for (int i = 0; i < MAX_WIDGETS; i++) {
            if (all_widgets[p][i].type == type && all_widgets[p][i].subtype == subtype) {
                all_widgets[p][i].enabled = 0;
            }
        }
    }
}

static void install_widgets(void)
{
    data.num_widgets = 0;
    for (int p = 0; p < CONFIG_PAGES; p++) {
        data.widgets_per_page[p] = 0;
        for (int i = 0; i < MAX_WIDGETS; i++) {
            if (all_widgets[p][i].enabled) {
                data.widgets[data.num_widgets++] = &all_widgets[p][i];
                data.widgets_per_page[p]++;
            }
        }
    }
}

static void set_page(int page)
{
    data.page = page;
    data.starting_option = 0;
    for (int i = 0; i < data.page; i++) {
        data.starting_option += data.widgets_per_page[i];
    }
    scrollbar_init(&scrollbar, 0, data.widgets_per_page[page]);
}

static void init(int page, int show_background_image)
{
    if (!data.config_values[0].change_action) {
        init_config_values();
    }
    if (!volume_text[0]) {
        volume_offset = string_copy(translation_for(TR_CONFIG_VOLUME), volume_text, 63);
        volume_offset = string_copy(string_from_ascii(" "), volume_offset, (int) (volume_offset - volume_text - 1));
    }

    for (int i = 0; i < CONFIG_MAX_ENTRIES; i++) {
        data.config_values[i].original_value = config_get(i);
        data.config_values[i].new_value = config_get(i);
    }
    for (int i = 0; i < CONFIG_STRING_MAX_ENTRIES; i++) {
        const char *value = config_get_string(i);
        strncpy(data.config_string_values[i].original_value, value, CONFIG_STRING_VALUE_MAX - 1);
        strncpy(data.config_string_values[i].new_value, value, CONFIG_STRING_VALUE_MAX - 1);
    }
    fetch_original_config_values();

    data.show_background_image = show_background_image;
    string_copy(translation_for(TR_CONFIG_LANGUAGE_DEFAULT), data.language_options_data[0], CONFIG_STRING_VALUE_MAX);
    data.language_options[0] = data.language_options_data[0];
    data.num_language_options = 1;
    const dir_listing *subdirs = dir_find_all_subdirectories();
    const char *original_value = data.config_string_values[CONFIG_STRING_UI_LANGUAGE_DIR].original_value;
    for (int i = 0; i < subdirs->num_files; i++) {
        if (data.num_language_options < MAX_LANGUAGE_DIRS && lang_dir_is_valid(subdirs->files[i])) {
            int opt_id = data.num_language_options;
            strncpy(data.language_options_utf8[opt_id], subdirs->files[i], CONFIG_STRING_VALUE_MAX - 1);
            encoding_from_utf8(subdirs->files[i], data.language_options_data[opt_id], CONFIG_STRING_VALUE_MAX);
            data.language_options[opt_id] = data.language_options_data[opt_id];
            if (strcmp(original_value, subdirs->files[i]) == 0) {
                data.selected_language_option = opt_id;
            }
            data.num_language_options++;
        }
    }

    enable_all_widgets();
    if (system_is_fullscreen_only()) {
        disable_widget(TYPE_SPACE, TR_CONFIG_VIDEO);
        disable_widget(TYPE_HEADER, TR_CONFIG_VIDEO);
        disable_widget(TYPE_CHECKBOX, CONFIG_ORIGINAL_FULLSCREEN);
        disable_widget(TYPE_NUMERICAL_DESC, RANGE_RESOLUTION);
        disable_widget(TYPE_NUMERICAL_RANGE, RANGE_RESOLUTION);
        disable_widget(TYPE_NUMERICAL_DESC, RANGE_DISPLAY_SCALE);
        disable_widget(TYPE_NUMERICAL_RANGE, RANGE_DISPLAY_SCALE);
        disable_widget(TYPE_NUMERICAL_DESC, RANGE_CURSOR_SCALE);
        disable_widget(TYPE_NUMERICAL_RANGE, RANGE_CURSOR_SCALE);
    }
    install_widgets();
    set_page(page);
}

static void checkbox_draw_text(int x, int y, int value_key, translation_key description)
{
    if (data.config_values[value_key].new_value) {
        text_draw(string_from_ascii("x"), x + 6, y + 3, FONT_NORMAL_BLACK, 0);
    }
    int text_width = CHECKBOX_TEXT_WIDTH;
    if (data.widgets_per_page[data.page] <= NUM_VISIBLE_ITEMS) {
        text_width += 32;
    }
    text_draw_ellipsized(translation_for(description), x + 30, y + 5, text_width, FONT_NORMAL_BLACK, 0);
}

static void checkbox_draw(int x, int y, int has_focus)
{
    button_border_draw(x, y, CHECKBOX_CHECK_SIZE, CHECKBOX_CHECK_SIZE, has_focus);
}

static void numerical_range_draw(const numerical_range_widget *w, int x, int y, const uint8_t *value_text)
{
    int extra_width = data.widgets_per_page[data.page] > NUM_VISIBLE_ITEMS ? 0 : 64;
    text_draw(value_text, x, y + 6, FONT_NORMAL_BLACK, 0);
    inner_panel_draw(x + w->x, y + 4, w->width_blocks + extra_width / 16, 1);

    int width = w->width_blocks * BLOCK_SIZE + extra_width - NUMERICAL_SLIDER_PADDING * 2 - NUMERICAL_DOT_SIZE;
    int scroll_position;
    if (w->min != w->max) {
        scroll_position = (*w->value - w->min) * width / (w->max - w->min);
    } else {
        scroll_position = width / 2;
    }
    image_draw(image_group(GROUP_PANEL_BUTTON) + 37,
        x + w->x + NUMERICAL_SLIDER_PADDING + scroll_position, y + 2, COLOR_MASK_NONE, SCALE_NONE);
}

static uint8_t *percentage_string(uint8_t *string, int percentage)
{
    int offset = string_from_int(string, percentage, 0);
    string[offset] = '%';
    string[offset + 1] = 0;
    return string;
}

static const uint8_t *display_text_language(void)
{
    return data.language_options[data.selected_language_option];
}

static const uint8_t *display_text_game_speed(void)
{
    return percentage_string(display_text, game_speeds[data.config_values[CONFIG_ORIGINAL_GAME_SPEED].new_value]);
}

static const uint8_t *display_text_resolution(void)
{
    uint8_t *str = display_text;
    resolution *r = &available_resolutions[data.config_values[CONFIG_ORIGINAL_WINDOWED_RESOLUTION].new_value];
    str += string_from_int(str, r->width, 0);
    str = string_copy(string_from_ascii("x"), str, 5);
    string_from_int(str, r->height, 0);
    return display_text;
}

static const uint8_t *display_text_display_scale(void)
{
    return percentage_string(display_text, data.config_values[CONFIG_SCREEN_DISPLAY_SCALE].new_value);
}

static const uint8_t *display_text_cursor_scale(void)
{
    return percentage_string(display_text, data.config_values[CONFIG_SCREEN_CURSOR_SCALE].new_value);
}

static const uint8_t *display_text_master_volume(void)
{
    percentage_string(volume_offset, data.config_values[CONFIG_GENERAL_MASTER_VOLUME].new_value);
    return volume_text;
}

static const uint8_t *display_text_music_volume(void)
{
    percentage_string(volume_offset, data.config_values[CONFIG_ORIGINAL_MUSIC_VOLUME].new_value);
    return volume_text;
}

static const uint8_t *display_text_speech_volume(void)
{
    percentage_string(volume_offset, data.config_values[CONFIG_ORIGINAL_SPEECH_VOLUME].new_value);
    return volume_text;
}

static const uint8_t *display_text_sound_effects_volume(void)
{
    percentage_string(volume_offset, data.config_values[CONFIG_ORIGINAL_SOUND_EFFECTS_VOLUME].new_value);
    return volume_text;
}

static const uint8_t *display_text_video_volume(void)
{
    percentage_string(volume_offset, data.config_values[CONFIG_GENERAL_VIDEO_VOLUME].new_value);
    return volume_text;
}

static const uint8_t *display_text_city_sounds_volume(void)
{
    percentage_string(volume_offset, data.config_values[CONFIG_ORIGINAL_CITY_SOUNDS_VOLUME].new_value);
    return volume_text;
}

static const uint8_t *display_text_scroll_speed(void)
{
    return percentage_string(display_text, data.config_values[CONFIG_ORIGINAL_SCROLL_SPEED].new_value);
}

static const uint8_t *display_text_difficulty(void)
{
    return lang_get_string(153, data.config_values[CONFIG_ORIGINAL_DIFFICULTY].new_value + 1);
}

static const uint8_t *display_text_max_grand_temples(void)
{
    string_from_int(display_text, data.config_values[CONFIG_GP_CH_MAX_GRAND_TEMPLES].new_value, 0);
    return display_text;
}

static void update_scale(void)
{
    int max_scale = system_get_max_display_scale();
    ranges[RANGE_DISPLAY_SCALE].max = max_scale;
    if (*ranges[RANGE_DISPLAY_SCALE].value > max_scale) {
        *ranges[RANGE_DISPLAY_SCALE].value = max_scale;
    }
}

static void calculate_available_resolutions_and_fullscreen(void)
{
    if (system_is_fullscreen_only()) {
        return;
    }
    memset(available_resolutions, 0, sizeof(available_resolutions));
    int resolution_index = 0;
    int display_resolution_index = -1;
    int current_resolution_index = -1;
    resolution max;
    system_get_max_resolution(&max.width, &max.height);
    static int old_width;
    static int old_height;
    static int old_fullscreen = -1;
    int width = screen_width();
    int height = screen_height();
    for (int i = 0; i < sizeof(resolutions) / sizeof(resolution); i++) {
        if (resolutions[i].width == width && resolutions[i].height == height) {
            current_resolution_index = resolution_index;
        } else if (current_resolution_index == -1 && (resolutions[i].width > width ||
            (resolutions[i].width == width && resolutions[i].height > height))) {
            available_resolutions[resolution_index].width = width;
            available_resolutions[resolution_index].height = height;
            current_resolution_index = resolution_index;
            resolution_index++;
        }
        if (resolutions[i].width == max.width && resolutions[i].height == max.height) {
            display_resolution_index = resolution_index;
        } else if (resolutions[i].width > max.width || resolutions[i].height > max.height) {
            continue;
        }
        available_resolutions[resolution_index].width = resolutions[i].width;
        available_resolutions[resolution_index].height = resolutions[i].height;
        resolution_index++;
    }
    if (display_resolution_index == -1) {
        available_resolutions[resolution_index].width = max.width;
        available_resolutions[resolution_index].height = max.height;
        resolution_index++;
    }

    ranges[RANGE_RESOLUTION].max = resolution_index - 1;
    if (!setting_fullscreen() && (old_width != width || old_height != height)) {
        data.config_values[CONFIG_ORIGINAL_WINDOWED_RESOLUTION].original_value = current_resolution_index;
        data.config_values[CONFIG_ORIGINAL_WINDOWED_RESOLUTION].new_value = current_resolution_index;
        old_width = width;
        old_height = height;
    }
    if (setting_fullscreen() != old_fullscreen) {
        data.config_values[CONFIG_ORIGINAL_FULLSCREEN].original_value = setting_fullscreen();
        data.config_values[CONFIG_ORIGINAL_FULLSCREEN].new_value = setting_fullscreen();
        old_fullscreen = setting_fullscreen();
    }
}

static inline int config_changed(config_key key)
{
    return data.config_values[key].original_value != data.config_values[key].new_value;
}

static inline int config_string_changed(config_string_key key)
{
    return strcmp(data.config_string_values[key].original_value, data.config_string_values[key].new_value) != 0;
}

static inline void update_widgets(void)
{
    update_scale();
    calculate_available_resolutions_and_fullscreen();
    data.has_changes = 0;
    for (int i = 0; i < CONFIG_STRING_MAX_ENTRIES && !data.has_changes; i++) {
        if (config_string_changed(i)) {
            data.has_changes = 1;
        }
    }
    for (int i = 0; i < CONFIG_MAX_ALL && !data.has_changes; i++) {
        if (config_changed(i)) {
            data.has_changes = 1;
        }
    }
}

static void draw_background(void)
{
    update_widgets();

    if (data.show_background_image) {
        image_draw_fullscreen_background(image_group(GROUP_INTERMEZZO_BACKGROUND) + 5);
    } else {
        window_draw_underlying_window();
    }

    graphics_in_dialog();

    outer_panel_draw(0, 0, 40, 30);

    text_draw_centered(translation_for(TR_CONFIG_TITLE), 16, 16, 608, FONT_LARGE_BLACK, 0);

    int page_x_offset = 30;
    int open_tab_width = text_get_width(translation_for(page_names[data.page]), FONT_NORMAL_BLACK) + 6;
    int max_closed_tab_width = (600 - page_x_offset * CONFIG_PAGES - open_tab_width) / (CONFIG_PAGES - 1);
    for (int i = 0; i < CONFIG_PAGES; ++i) {
        page_x_offset += 15;
        int width = 0;
        if (data.page == i) {
            width = text_draw(translation_for(page_names[i]), page_x_offset, 58, FONT_NORMAL_BLACK, 0);
        } else {
            width = text_draw_ellipsized(translation_for(page_names[i]),
                page_x_offset, 58, max_closed_tab_width, FONT_NORMAL_BLACK, 0);
        }
        page_buttons[i].x = page_x_offset - 10;
        page_buttons[i].width = width + 15;
        data.graphics_behind_tab[i] = graphics_save_to_image(data.graphics_behind_tab[i],
            page_buttons[i].x, 75, page_buttons[i].width, 3);
        page_x_offset += width;
    }

    button_border_draw(10, 75, 620, 355, 0);

    for (int i = 0; i < NUM_VISIBLE_ITEMS && i < data.widgets_per_page[data.page]; i++) {
        config_widget *w = data.widgets[i + data.starting_option + scrollbar.scroll_position];
        int y = ITEM_Y_OFFSET + ITEM_HEIGHT * i;
        if (w->type == TYPE_HEADER) {
            text_draw(translation_for(w->subtype), 20, y + w->y_offset, FONT_NORMAL_BLACK, 0);
        } else if (w->type == TYPE_CHECKBOX) {
            checkbox_draw_text(20, y + w->y_offset, w->subtype, w->description);
        } else if (w->type == TYPE_SELECT) {
            text_draw(translation_for(w->description), 20, y + 6 + w->y_offset, FONT_NORMAL_BLACK, 0);
            const generic_button *btn = &select_buttons[w->subtype];
            text_draw_centered(w->get_display_text(), btn->x, y + btn->y + 6 + w->y_offset,
                btn->width, FONT_NORMAL_BLACK, 0);
        } else if (w->type == TYPE_NUMERICAL_RANGE) {
            numerical_range_draw(&ranges[w->subtype], NUMERICAL_RANGE_X, y + w->y_offset, w->get_display_text());
        } else if (w->type == TYPE_NUMERICAL_DESC) {
            text_draw(translation_for(w->description), 20, y + 10 + w->y_offset, FONT_NORMAL_BLACK, 0);
        }
    }

    for (int i = 0; i < sizeof(bottom_buttons) / sizeof(*bottom_buttons); i++) {
        int disabled = i == NUM_BOTTOM_BUTTONS - 1 && !data.has_changes;
        text_draw_centered(translation_for(bottom_buttons[i].parameter2),
            bottom_buttons[i].x, bottom_buttons[i].y + 9, bottom_buttons[i].width,
            disabled ? FONT_NORMAL_PLAIN : FONT_NORMAL_BLACK,
            disabled ? COLOR_FONT_LIGHT_GRAY : 0);
    }

    graphics_reset_dialog();
}


static void draw_foreground(void)
{
    graphics_in_dialog();

    for (int i = 0; i < CONFIG_PAGES; ++i) {
        button_border_draw(page_buttons[i].x, page_buttons[i].y,
            page_buttons[i].width, page_buttons[i].height,
            data.page_focus_button == i + 1);
        if (data.page == i) {
            graphics_draw_from_image(data.graphics_behind_tab[i], page_buttons[i].x, 75);
        } else {
            graphics_fill_rect(page_buttons[i].x, 75, page_buttons[i].width, 3, COLOR_WHITE);
        }
    }

    for (int i = 0; i < NUM_VISIBLE_ITEMS && i < data.widgets_per_page[data.page]; i++) {
        config_widget *w = data.widgets[i + data.starting_option + scrollbar.scroll_position];
        int y = ITEM_Y_OFFSET + ITEM_HEIGHT * i;
        if (w->type == TYPE_CHECKBOX) {
            checkbox_draw(20, y + w->y_offset, data.focus_button == i + 1);
        } else if (w->type == TYPE_SELECT) {
            const generic_button *btn = &select_buttons[w->subtype];
            button_border_draw(btn->x, y + btn->y + w->y_offset,
                btn->width, btn->height, data.focus_button == i + 1);
        }
    }
    for (int i = 0; i < sizeof(bottom_buttons) / sizeof(*bottom_buttons); i++) {
        button_border_draw(bottom_buttons[i].x, bottom_buttons[i].y,
            bottom_buttons[i].width, bottom_buttons[i].height, data.bottom_focus_button == i + 1);
    }

    if (data.widgets_per_page[data.page] > NUM_VISIBLE_ITEMS) {
        inner_panel_draw(scrollbar.x + 4, scrollbar.y + 28, 2, scrollbar.height / BLOCK_SIZE - 3);
        scrollbar_draw(&scrollbar);
    }

    graphics_reset_dialog();
}

static int is_checkbox(const mouse *m, int x, int y)
{
    if (x <= m->x && x + CHECKBOX_WIDTH > m->x &&
        y <= m->y && y + CHECKBOX_HEIGHT > m->y) {
        return 1;
    }
    return 0;
}

static int checkbox_handle_mouse(const mouse *m, int x, int y, int value_key, int *focus)
{
    if (!is_checkbox(m, x, y)) {
        return 0;
    }
    *focus = 1;
    if (m->left.went_up) {
        toggle_switch(value_key);
        return 1;
    } else {
        return 0;
    }
}

static int is_numerical_range(const numerical_range_widget *w, const mouse *m, int x, int y)
{
    int extra_width = data.widgets_per_page[data.page] > NUM_VISIBLE_ITEMS ? 0 : 64;
    if (x + w->x <= m->x && x + w->width_blocks * 16 + w->x + extra_width >= m->x &&
        y <= m->y && y + 16 > m->y) {
        return 1;
    }
    return 0;
}

static int numerical_range_handle_mouse(const mouse *m, int x, int y, int numerical_range_id)
{
    const numerical_range_widget *w = &ranges[numerical_range_id - 1];

    if (data.active_numerical_range) {
        if (data.active_numerical_range != numerical_range_id) {
            return 0;
        }
        if (!m->left.is_down) {
            data.active_numerical_range = 0;
            return 0;
        }
    } else if (!m->left.went_down || !is_numerical_range(w, m, x, y)) {
        return 0;
    }
    if (w->min == w->max) {
        return 1;
    }
    int extra_width = data.widgets_per_page[data.page] > NUM_VISIBLE_ITEMS ? 0 : 64;
    int slider_width = w->width_blocks * BLOCK_SIZE - NUMERICAL_SLIDER_PADDING * 2 - NUMERICAL_DOT_SIZE + extra_width;
    int pixels_per_pct = slider_width / (w->max - w->min);
    int dot_position = m->x - x - w->x - NUMERICAL_DOT_SIZE / 2 + pixels_per_pct / 2;

    int exact_value = calc_bound(w->min + dot_position * (w->max - w->min) / slider_width, w->min, w->max);
    int left_step_value = (exact_value / w->step) * w->step;
    int right_step_value = calc_bound(left_step_value + w->step, w->min, w->max);
    int closest_step_value = (exact_value - left_step_value) < (right_step_value - exact_value) ?
        left_step_value : right_step_value;
    if (closest_step_value != *w->value) {
        *w->value = closest_step_value;
        window_request_refresh();
    }
    data.active_numerical_range = numerical_range_id;
    return 1;
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    data.focus_button = 0;

    if (data.active_numerical_range) {
        numerical_range_handle_mouse(m_dialog, NUMERICAL_RANGE_X, 0, data.active_numerical_range);
        return;
    }

    if (scrollbar_handle_mouse(&scrollbar, m_dialog, 1)) {
        data.page_focus_button = 0;
        data.bottom_focus_button = 0;
        return;
    }

    int handled = 0;
    for (int i = 0; i < NUM_VISIBLE_ITEMS && i < data.widgets_per_page[data.page]; i++) {
        config_widget *w = data.widgets[i + data.starting_option + scrollbar.scroll_position];
        int y = ITEM_Y_OFFSET + ITEM_HEIGHT * i;
        if (w->type == TYPE_CHECKBOX) {
            int focus = 0;
            handled |= checkbox_handle_mouse(m_dialog, 20, y + w->y_offset, w->subtype, &focus);
            if (focus) {
                data.focus_button = i + 1;
            }
        } else if (w->type == TYPE_SELECT) {
            generic_button *btn = &select_buttons[w->subtype];
            btn->parameter1 = y + w->y_offset;
            int focus = 0;
            handled |= generic_buttons_handle_mouse(m_dialog, 0, y + w->y_offset, btn, 1, &focus);
            if (focus) {
                data.focus_button = i + 1;
            }
        } else if (w->type == TYPE_NUMERICAL_RANGE) {
            handled |= numerical_range_handle_mouse(m_dialog, NUMERICAL_RANGE_X, y + w->y_offset, w->subtype + 1);
        }
    }

    handled |= generic_buttons_handle_mouse(m_dialog, 0, 0, bottom_buttons,
        data.has_changes ? NUM_BOTTOM_BUTTONS : NUM_BOTTOM_BUTTONS - 1, &data.bottom_focus_button);

    handled |= generic_buttons_handle_mouse(m_dialog, 0, 0, page_buttons, CONFIG_PAGES, &data.page_focus_button);

    if (!handled && (m->right.went_up || h->escape_pressed)) {
        window_go_back();
    }
}

static void on_scroll(void)
{
    window_invalidate();
}

static void toggle_switch(int key)
{
    data.config_values[key].new_value = 1 - data.config_values[key].new_value;
    window_invalidate();
}

static void set_language(int index)
{
    const char *dir = index == 0 ? "" : data.language_options_utf8[index];
    strncpy(data.config_string_values[CONFIG_STRING_UI_LANGUAGE_DIR].new_value, dir, CONFIG_STRING_VALUE_MAX - 1);

    data.selected_language_option = index;
}

static void button_hotkeys(int param1, int param2)
{
    window_hotkey_config_show();
}

static void button_language_select(int height, int param2)
{
    const generic_button *btn = &select_buttons[SELECT_LANGUAGE];
    window_select_list_show_text(
        screen_dialog_offset_x() + btn->x,
        screen_dialog_offset_y() + height + btn->height,
        data.language_options, data.num_language_options, set_language
    );
}

static void button_reset_defaults(int param1, int param2)
{
    for (int i = 0; i < CONFIG_MAX_ENTRIES; ++i) {
        data.config_values[i].new_value = config_get_default_value(i);
    }
    for (int i = 0; i < CONFIG_STRING_MAX_ENTRIES; ++i) {
        strncpy(data.config_string_values[i].new_value,
            config_get_default_string_value(i), CONFIG_STRING_VALUE_MAX - 1);
    }
    set_language(0);
    window_invalidate();

}

static void cancel_values(void)
{
    for (int i = 0; i < CONFIG_MAX_ALL; i++) {
        data.config_values[i].new_value = data.config_values[i].original_value;
    }
    for (int i = 0; i < CONFIG_STRING_MAX_ENTRIES; i++) {
        // memcpy required to fix warning on Switch build
        memcpy(data.config_string_values[i].new_value,
            data.config_string_values[i].original_value, CONFIG_STRING_VALUE_MAX - 1);
    }
}

static int config_change_basic(config_key key)
{
    config_set(key, data.config_values[key].new_value);
    data.config_values[key].original_value = data.config_values[key].new_value;
    return 1;
}

static int config_change_game_speed(config_key key)
{
    config_change_basic(key);

    int game_speed = game_speeds[data.config_values[key].new_value];

    while (setting_game_speed() > game_speed) {
        setting_decrease_game_speed();
    }
    while (setting_game_speed() < game_speed) {
        setting_increase_game_speed();
    }
    return 1;
}

static int config_change_fullscreen(config_key key)
{
    if (!system_is_fullscreen_only()) {
        system_set_fullscreen(data.config_values[key].new_value);
        // Force internal fullscreen setting to prevent cases where fullscreen wouldn't trigger
        // if the user also changed the windowed resolution at the same time
        if (data.config_values[key].new_value) {
            setting_set_display(1, screen_width(), screen_height());
        }
        config_change_basic(key);
    }
    return 1;
}

static int config_change_display_resolution(config_key key)
{
    if (!system_is_fullscreen_only()) {
        const resolution *r = &available_resolutions[data.config_values[key].new_value];
        if (!setting_fullscreen()) {
            system_resize(r->width, r->height);
        } else {
            // Force internal window size to the specified size even though the game stays at fullscreen
            // If the user then disables fullscreen, the game will switch to the newly set size
            setting_set_display(0, r->width, r->height);
            setting_set_display(1, r->width, r->height);
        }
        config_change_basic(key);
    }
    return 1;
}

static int config_change_display_scale(config_key key)
{
    data.config_values[key].new_value = system_scale_display(data.config_values[key].new_value);
    config_change_basic(key);
    return 1;
}

static void restart_cursors(void)
{
    if (data.reload_cursors) {
        system_init_cursors(config_get(CONFIG_SCREEN_CURSOR_SCALE));
        data.reload_cursors = 0;
    }
}

static int config_change_cursors(config_key key)
{
    config_change_basic(key);
    data.reload_cursors = 1;
    return 1;
}

static int config_enable_audio(config_key key)
{
    config_change_basic(key);
    if (data.config_values[key].new_value) {
        if (data.show_background_image) {
            sound_music_play_intro();
        } else {
            sound_music_stop();
            sound_music_update(1);
        }
    } else {
        sound_music_stop();
        sound_speech_stop();
    }
    return 1;
}

static int config_set_master_volume(config_key key)
{
    config_change_basic(key);
    sound_music_set_volume(setting_sound(SOUND_MUSIC)->volume);
    sound_speech_set_volume(setting_sound(SOUND_SPEECH)->volume);
    sound_effect_set_volume(setting_sound(SOUND_EFFECTS)->volume);
    sound_city_set_volume(setting_sound(SOUND_CITY)->volume);
    return 1;
}

static int config_enable_music(config_key key)
{
    config_change_basic(key);

    if (setting_sound_is_enabled(SOUND_MUSIC) != data.config_values[key].new_value) {
        setting_toggle_sound_enabled(SOUND_MUSIC);
    }
    if (data.config_values[key].new_value) {
        if (data.show_background_image) {
            sound_music_play_intro();
        } else {
            sound_music_stop();
            sound_music_update(1);
        }
    } else {
        sound_music_stop();
    }
    return 1;
}

static int config_set_music_volume(config_key key)
{
    config_change_basic(key);
    setting_set_sound_volume(SOUND_MUSIC, data.config_values[key].new_value);
    sound_music_set_volume(setting_sound(SOUND_MUSIC)->volume);
    return 1;
}

static int config_enable_speech(config_key key)
{
    config_change_basic(key);

    if (setting_sound_is_enabled(SOUND_SPEECH) != data.config_values[key].new_value) {
        setting_toggle_sound_enabled(SOUND_SPEECH);
    }
    if (!data.config_values[key].new_value) {
        sound_speech_stop();
    }
    return 1;
}

static int config_set_speech_volume(config_key key)
{
    config_change_basic(key);
    setting_set_sound_volume(SOUND_SPEECH, data.config_values[key].new_value);
    sound_speech_set_volume(setting_sound(SOUND_SPEECH)->volume);
    return 1;
}

static int config_enable_effects(config_key key)
{
    config_change_basic(key);

    if (setting_sound_is_enabled(SOUND_EFFECTS) != data.config_values[key].new_value) {
        setting_toggle_sound_enabled(SOUND_EFFECTS);
    }
    return 1;
}

static int config_set_effects_volume(config_key key)
{
    config_change_basic(key);
    setting_set_sound_volume(SOUND_EFFECTS, data.config_values[key].new_value);
    sound_effect_set_volume(setting_sound(SOUND_EFFECTS)->volume);
    return 1;
}

static int config_enable_city_sounds(config_key key)
{
    config_change_basic(key);

    if (setting_sound_is_enabled(SOUND_CITY) != data.config_values[key].new_value) {
        setting_toggle_sound_enabled(SOUND_CITY);
    }
    return 1;
}

static int config_set_city_sounds_volume(config_key key)
{
    config_change_basic(key);
    setting_set_sound_volume(SOUND_CITY, data.config_values[key].new_value);
    sound_city_set_volume(setting_sound(SOUND_CITY)->volume);
    return 1;
}

static int config_change_scroll_speed(config_key key)
{
    config_change_basic(key);

    while (setting_scroll_speed() > data.config_values[key].new_value) {
        setting_decrease_scroll_speed();
    }
    while (setting_scroll_speed() < data.config_values[key].new_value) {
        setting_increase_scroll_speed();
    }
    return 1;
}

static int config_set_difficulty(config_key key)
{
    config_change_basic(key);

    while (setting_difficulty() > data.config_values[key].new_value) {
        setting_decrease_difficulty();
    }
    while (setting_difficulty() < data.config_values[key].new_value) {
        setting_increase_difficulty();
    }
    return 1;
}

static int config_enable_gods_effects(config_key key)
{
    config_change_basic(key);

    if (setting_gods_enabled() != data.config_values[key].new_value) {
        setting_toggle_gods_enabled();
    }
    return 1;
}

static int config_change_string_basic(config_string_key key)
{
    config_set_string(key, data.config_string_values[key].new_value);
    strncpy(data.config_string_values[key].original_value,
        data.config_string_values[key].new_value, CONFIG_STRING_VALUE_MAX - 1);
    return 1;
}

static int config_change_string_language(config_string_key key)
{
    config_set_string(CONFIG_STRING_UI_LANGUAGE_DIR, data.config_string_values[key].new_value);
    if (!game_reload_language()) {
        config_set_string(CONFIG_STRING_UI_LANGUAGE_DIR, data.config_string_values[key].original_value);
        game_reload_language();
        window_plain_message_dialog_show(TR_INVALID_LANGUAGE_TITLE, TR_INVALID_LANGUAGE_MESSAGE, 1);
        return 0;
    }
    strncpy(data.config_string_values[key].original_value,
        data.config_string_values[key].new_value, CONFIG_STRING_VALUE_MAX - 1);
    string_copy(translation_for(TR_CONFIG_LANGUAGE_DEFAULT), data.language_options_data[0], CONFIG_STRING_VALUE_MAX);
    return 1;
}

static int apply_changed_configs(void)
{
    if (!data.has_changes) {
        return 1;
    }
    for (int i = 0; i < CONFIG_MAX_ALL; ++i) {
        if (config_changed(i)) {
            if (!data.config_values[i].change_action(i)) {
                return 0;
            }
        }
    }
    for (int i = 0; i < CONFIG_STRING_MAX_ENTRIES; ++i) {
        if (config_string_changed(i)) {
            if (!data.config_string_values[i].change_action(i)) {
                return 0;
            }
        }
    }
    restart_cursors();
    return 1;
}

static void button_close(int save, int param2)
{
    if (!save) {
        cancel_values();
        window_go_back();
        return;
    }
    if (apply_changed_configs() && save == 1) {
        config_save();
        window_go_back();
        return;
    }
    window_request_refresh();
}

static void button_page(int page, int param2)
{
    set_page(page);
    window_invalidate();
}

static void get_tooltip(tooltip_context *c)
{
    if (data.page_focus_button) {
        int page = data.page_focus_button - 1;
        if (page == data.page) {
            return;
        }
        int text_width = text_get_width(translation_for(page_names[page]), FONT_NORMAL_BLACK);
        if (page_buttons[page].width - 15 < text_width) {
            c->translation_key = page_names[page];
            c->type = TOOLTIP_BUTTON;
        }
    }
}

void window_config_show(window_config_page page, int show_background_image)
{
    window_type window = {
        WINDOW_CONFIG,
        draw_background,
        draw_foreground,
        handle_input,
        get_tooltip
    };
    init(page, show_background_image);
    window_show(&window);
}

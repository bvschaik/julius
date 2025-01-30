#include "asset_previewer.h"

#include "assets/assets.h"
#include "assets/group.h"
#include "core/calc.h"
#include "core/dir.h"
#include "core/direction.h"
#include "core/encoding.h"
#include "core/lang.h"
#include "core/log.h"
#include "core/file.h"
#include "core/random.h"
#include "core/string.h"
#include "core/time.h"
#include "game/animation.h"
#include "game/settings.h"
#include "game/system.h"
#include "graphics/button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/list_box.h"
#include "graphics/panel.h"
#include "graphics/renderer.h"
#include "graphics/screen.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/scroll.h"
#include "scenario/property.h"
#include "sound/music.h"
#include "window/popup_dialog.h"
#include "window/select_list.h"

#include <string.h>

enum {
    BUTTON_CHANGE_ASSET_GROUP = 0,
    BUTTON_CHANGE_TERRAIN = 1,
    BUTTON_CHANGE_ZOOM = 2,
    BUTTON_TOGGLE_ANIMATIONS = 3,
    BUTTON_REFRESH = 4,
    BUTTON_QUIT = 5,
    NUM_BUTTONS = 6
};

typedef enum {
    TERRAIN_NONE = 0,
    TERRAIN_CENTRAL = 1,
    TERRAIN_NORTHERN = 2,
    TERRAIN_DESERT = 3,
    TERRAIN_MAX = 4
} terrain;

#define TILE_X_SIZE 60
#define TILE_Y_SIZE 30
#define HALF_TYLE_X_SIZE (TILE_X_SIZE / 2)
#define HALF_TYLE_Y_SIZE (TILE_Y_SIZE / 2)
#define NUM_GRASS_TILES 58

#define ASSET_BUTTON_SIZE 18

#define REFRESHED_INFO_TIME_MS 5000

static void draw_asset_entry(const list_box_item *item);
static void select_asset(unsigned int index, int unused);
static void handle_tooltip(const list_box_item *item, tooltip_context *c);
static void button_top(const generic_button *button);
static void button_toggle_animation_frames(const generic_button *button);

static generic_button buttons[NUM_BUTTONS] = {
    { 0, 25, 180, 20, button_top, 0, BUTTON_CHANGE_ASSET_GROUP },
    { 200, 25, 140, 20, button_top, 0, BUTTON_CHANGE_TERRAIN },
    { 360, 5, 160, 20, button_top, 0, BUTTON_CHANGE_ZOOM },
    { 360, 25, 160, 20, button_top, 0, BUTTON_TOGGLE_ANIMATIONS },
    { 530, -40, 80, 40, button_top, 0, BUTTON_REFRESH },
    { 530, 5, 80, 40, button_top, 0, BUTTON_QUIT },
};

static generic_button toggle_animation_button = {
    0, 0, 0, 20, button_toggle_animation_frames
};

static const int ZOOM_VALUES[] = { 50, 100, 200, 400 };
#define TOTAL_ZOOM_VALUES (sizeof(ZOOM_VALUES) / sizeof(int))

typedef struct {
    int index;
    int is_animation_frame;
} asset_entry;

static list_box_type list_box = {
    .width_blocks = 15,
    .item_height = ASSET_BUTTON_SIZE,
    .draw_inner_panel = 1,
    .extend_to_hidden_scrollbar = 1,
    .decorate_scrollbar = 1,
    .draw_item = draw_asset_entry,
    .on_select = select_asset,
    .handle_tooltip = handle_tooltip
};

static struct {
    const dir_listing *xml_files;
    const uint8_t **xml_file_names;
    image_groups *active_group;
    int active_group_index;
    const image_atlas_data *main_atlas;
    terrain terrain;
    scenario_climate climate;
    const uint8_t *terrain_texts[TERRAIN_MAX];
    const uint8_t *zoom_texts[TOTAL_ZOOM_VALUES];
    uint8_t *encoded_asset_id;
    int encoded_asset_id_size;
    unsigned int focus_button_id;
    unsigned int animation_button_focused;
    int x_offset_top;
    asset_entry *entries;
    char *selected_asset_id;
    int hide_animation_frames;
    int scale;
    struct {
        int enabled;
        int frame;
        int reversed;
    } animation;
    time_millis last_refresh;
    int showing_refresh_info;
} data;

static int update_entries(void)
{
    int total_entries = 0;
    free(data.entries);
    data.entries = 0;
    if (!data.active_group) {
        return 0;
    }
    int total_images = data.active_group->last_image_index - data.active_group->first_image_index + 1;
    data.entries = malloc(sizeof(asset_entry) * total_images);
    if (!data.entries) {
        log_error("Not enough memory", 0, 0);
        return 0;
    }
    memset(data.entries, 0, sizeof(asset_entry) * total_images);
    int current_asset = 0;
    int animation_sprites = 0;
    for (int i = 0; i < total_images && current_asset < total_images; i++, current_asset++) {
        data.entries[i].index = current_asset;
        data.entries[i].is_animation_frame = animation_sprites-- > 0;
        int image_id = current_asset + IMAGE_MAIN_ENTRIES + 1;
        const image *img = image_get(image_id);
        if (img->animation) {
            if (data.hide_animation_frames) {
                current_asset += img->animation->num_sprites;
            } else {
                animation_sprites = img->animation->num_sprites;
            }
        }
        total_entries++;
    }
    return total_entries;
}

static void select_asset(unsigned int index, int unused)
{
    const asset_image *img = asset_image_get_from_id(data.active_group->first_image_index + data.entries[index].index);
    free(data.selected_asset_id);
    data.selected_asset_id = 0;
    if (img->id) {
        size_t id_length = (strlen(img->id) + 1) * sizeof(char);
        data.selected_asset_id = malloc(id_length);
        if (data.selected_asset_id) {
            snprintf(data.selected_asset_id, id_length, "%s", img->id);
        }
    }
    if (data.animation.enabled) {
        game_animation_init();
        data.animation.frame = 1;
        data.animation.reversed = 0;
    }
    window_invalidate();
}

static void load_assets(int changed)
{
    assets_load_single_group(data.xml_files->files[data.active_group_index].name,
        data.main_atlas->buffers, data.main_atlas->image_widths);
    data.active_group = group_get_current();
    int total_entries = update_entries();
    list_box_update_total_items(&list_box, total_entries);
    if (changed) {
        list_box_select_index(&list_box, 0);
    }
}

static int load_climate(int force)
{
    scenario_climate climate = data.terrain == TERRAIN_NONE ? data.climate : (data.terrain - 1);
    if (data.climate != climate || force) {
        if (!image_load_climate(climate, 0, 1, 1)) {
            log_error("Unable to load main graphics", 0, 0);
            return 0;
        }
        data.main_atlas = graphics_renderer()->get_image_atlas(ATLAS_MAIN);
        load_assets(force);
        data.climate = climate;
    }

    return 1;
}

static int update_asset_groups_list(void)
{
    static uint8_t currently_open_file[FILE_NAME_MAX];

    if (data.xml_files && data.xml_files->num_files > 0) {
        string_copy(data.xml_file_names[data.active_group_index], currently_open_file, FILE_NAME_MAX);
        for (int i = 0; i < data.xml_files->num_files; i++) {
            free((uint8_t *) data.xml_file_names[i]);
        }
        free((uint8_t **) data.xml_file_names);
        data.xml_file_names = 0;
    }

    data.xml_files = dir_find_files_with_extension(ASSETS_DIRECTORY "/" ASSETS_IMAGE_PATH, "xml");

    if (data.xml_files->num_files == 0) {
        data.active_group_index = 0;
        return 0;
    }

    data.xml_file_names = malloc(sizeof(uint8_t *) * data.xml_files->num_files);
    static char original_file[FILE_NAME_MAX];

    for (int i = 0; i < data.xml_files->num_files; i++) {
        snprintf(original_file, FILE_NAME_MAX, "%s", data.xml_files->files[i].name);
        file_remove_extension(original_file);
        int size = (int) strlen(original_file) + 1;
        uint8_t *file = malloc(sizeof(uint8_t) * size);
        encoding_from_utf8(original_file, file, size);
        if (string_equals(file, currently_open_file)) {
            data.active_group_index = i;
        }
        data.xml_file_names[i] = file;
    }

    if (data.active_group_index >= data.xml_files->num_files) {
        data.active_group_index = data.xml_files->num_files - 1;
    }

    return string_equals(currently_open_file, data.xml_file_names[data.active_group_index]) != 1;
}

static void create_selection_lists(void)
{
    data.terrain_texts[TERRAIN_NONE] = lang_get_string(CUSTOM_TRANSLATION, TR_WINDOW_ASSET_PREVIEWER_TERRAIN_NONE);
    data.terrain_texts[TERRAIN_CENTRAL] =
        lang_get_string(CUSTOM_TRANSLATION, TR_WINDOW_ASSET_PREVIEWER_TERRAIN_CENTRAL);
    data.terrain_texts[TERRAIN_NORTHERN] =
        lang_get_string(CUSTOM_TRANSLATION, TR_WINDOW_ASSET_PREVIEWER_TERRAIN_NORTHERN);
    data.terrain_texts[TERRAIN_DESERT] = lang_get_string(CUSTOM_TRANSLATION, TR_WINDOW_ASSET_PREVIEWER_TERRAIN_DESERT);

    for (size_t i = 0; i < TOTAL_ZOOM_VALUES; i++) {
        if (data.zoom_texts[i]) {
            continue;
        }
        uint8_t *text = malloc(sizeof(uint8_t) * 5);
        if (!text) {
            continue;
        }
        int index = string_from_int(text, ZOOM_VALUES[i], 0);
        text[index] = '%';
        text[index + 1] = 0;
        data.zoom_texts[i] = text;
    }
}

static void set_initial_options(void)
{
    data.animation.enabled = 1;
    data.animation.frame = 1;
    data.hide_animation_frames = 1;
    data.scale = 100;
}

static int init(void)
{
    image_load_fonts(encoding_get());
    set_initial_options();
    update_asset_groups_list();
    create_selection_lists();
    random_generate_pool();
    sound_music_play_editor();
    list_box_init(&list_box, 0);
    return load_climate(1);
}

static void draw_terrain_background(void)
{
    int y_offset = -HALF_TYLE_Y_SIZE;
    int y_tiles = screen_height() / HALF_TYLE_Y_SIZE + 2;
    int x_tiles = screen_width() / TILE_X_SIZE + 2;
    random_init();
    for (int y = 0; y < y_tiles; y++) {
        int x_offset = (y & 1) ? -HALF_TYLE_X_SIZE : 0;
        for (int x = 0; x < x_tiles; x++) {
            random_generate_next();
            int image_offset = random_byte() % NUM_GRASS_TILES;
            int group = (random_byte_alt() & 1) ? GROUP_TERRAIN_GRASS_1 : GROUP_TERRAIN_GRASS_2;
            image_draw_isometric_footprint(image_group(group) + image_offset, x_offset, y_offset,
                COLOR_MASK_NONE, SCALE_NONE);
            x_offset += TILE_X_SIZE;
        }
        y_offset += HALF_TYLE_Y_SIZE;
    }
}

static inline int get_current_asset_index(void)
{
    return data.entries[list_box_get_selected_index(&list_box)].index + data.active_group->first_image_index;
}

static void draw_asset(void)
{
    int image_id = get_current_asset_index() + IMAGE_MAIN_ENTRIES;
    const image *img = image_get(image_id);
    int x_start = 18 * BLOCK_SIZE;
    int y_start = 12 * BLOCK_SIZE;
    int width = screen_width() - x_start;
    int height = screen_height() - y_start;
    float scale = data.scale / 100.0f;

    int asset_width = img->is_isometric ? img->width : img->original.width;
    int asset_height = img->is_isometric ? img->height : img->original.height;

    int x_offset = calc_adjust_with_percentage(x_start + (int) (width - asset_width / scale) / 2, data.scale);
    int y_offset = calc_adjust_with_percentage(y_start + (int) (height - asset_height / scale) / 2, data.scale);
    if (img->is_isometric) {
        y_offset += img->height / 2;
        image_draw_isometric_top_from_draw_tile(image_id, x_offset, y_offset, COLOR_MASK_NONE, scale);
        image_draw_isometric_footprint_from_draw_tile(image_id, x_offset, y_offset, COLOR_MASK_NONE, scale);
    } else {
        image_draw(image_id, x_offset, y_offset, COLOR_MASK_NONE, scale);
    }
    if (data.animation.enabled && img->animation) {
        image_draw(image_id + img->animation->start_offset + data.animation.frame,
            x_offset + img->animation->sprite_offset_x,
            y_offset + img->animation->sprite_offset_y - (img->top ? img->top->original.height - HALF_TYLE_Y_SIZE : 0),
            COLOR_MASK_NONE, scale);
    }
}

static void draw_refreshed_info(void)
{
    const uint8_t *text = lang_get_string(CUSTOM_TRANSLATION, TR_WINDOW_ASSET_PREVIEWER_ASSETS_REFRESHED);
    int label_width = text_get_width(text, FONT_NORMAL_WHITE) + 100;
    label_width -= label_width % BLOCK_SIZE;
    int x_offset = (screen_width() - label_width) / 2;
    int y_offset = BLOCK_SIZE * 8;
    label_draw(x_offset, y_offset, label_width / BLOCK_SIZE, 1);
    image_draw(image_group(GROUP_CONTEXT_ICONS) + 15, x_offset + 2, y_offset + 2,
    COLOR_MASK_NONE, SCALE_NONE);
    image_draw(image_group(GROUP_CONTEXT_ICONS) + 15, x_offset + label_width - 36, y_offset + 2,
        COLOR_MASK_NONE, SCALE_NONE);

    text_draw_centered(text, x_offset, y_offset + 4, label_width, FONT_NORMAL_WHITE, 0);
}

static void draw_background(void)
{
    graphics_clear_screen();

    if (data.terrain) {
        draw_terrain_background();
    }

    if (list_box_get_total_items(&list_box) > 0) {
        draw_asset();
    }

    data.x_offset_top = (screen_width() - 39 * BLOCK_SIZE) / 2;

    outer_panel_draw(data.x_offset_top, 8, 40, 7);
    lang_text_draw_centered(CUSTOM_TRANSLATION, TR_WINDOW_ASSET_PREVIEWER_TITLE,
        data.x_offset_top + 16, 24, 530, FONT_LARGE_BLACK);

    lang_text_draw(CUSTOM_TRANSLATION, TR_WINDOW_ASSET_PREVIEWER_GROUP, data.x_offset_top + 16, 69, FONT_NORMAL_BLACK);
    if (data.xml_files->num_files > 0) {
        text_draw_centered(data.xml_file_names[data.active_group_index], data.x_offset_top + 16, 89, 180,
            FONT_NORMAL_BLACK, COLOR_MASK_NONE);
    } else {
        const uint8_t *no_groups_text = lang_get_string(CUSTOM_TRANSLATION, TR_WINDOW_ASSET_PREVIEWER_NO_GROUPS);
        text_draw_centered(no_groups_text, data.x_offset_top + 16, 89, 180, FONT_NORMAL_PLAIN, COLOR_FONT_GRAY);
    }
    lang_text_draw(CUSTOM_TRANSLATION, TR_WINDOW_ASSET_PREVIEWER_TERRAIN,
        data.x_offset_top + 216, 69, FONT_NORMAL_BLACK);
    text_draw_centered(data.terrain_texts[data.terrain], data.x_offset_top + 216, 89, 140,
        FONT_NORMAL_BLACK, COLOR_MASK_NONE);
    int width = lang_text_draw(CUSTOM_TRANSLATION, TR_ZOOM,
        data.x_offset_top + 376, 69, FONT_NORMAL_BLACK);
    int inverted_scale = calc_percentage(100, data.scale);
    text_draw_number_centered_postfix(inverted_scale, "%",
        data.x_offset_top + 376 + width, 70, 160 - width, FONT_NORMAL_BLACK);
    if (data.animation.enabled) {
        text_draw(string_from_ascii("x"), data.x_offset_top + 382, 88, FONT_NORMAL_BLACK, COLOR_MASK_NONE);
    }
    lang_text_draw(CUSTOM_TRANSLATION, TR_WINDOW_ASSET_PREVIEWER_PLAY_ANIMATIONS,
        data.x_offset_top + 404, 89, FONT_NORMAL_BLACK);
    lang_text_draw_centered(CUSTOM_TRANSLATION, TR_WINDOW_ASSET_PREVIEWER_REFRESH,
        data.x_offset_top + 546, 34, 80, FONT_NORMAL_BLACK);
    lang_text_draw_centered(CUSTOM_TRANSLATION, TR_WINDOW_ASSET_PREVIEWER_QUIT,
        data.x_offset_top + 546, 79, 80, FONT_NORMAL_BLACK);

    if (data.showing_refresh_info) {
        draw_refreshed_info();
    }

    int outer_height_blocks = (screen_height() - 11 * BLOCK_SIZE) / BLOCK_SIZE;

    outer_panel_draw(8, 10 * BLOCK_SIZE, list_box.width_blocks + 3, outer_height_blocks);
    lang_text_draw(CUSTOM_TRANSLATION, TR_WINDOW_ASSET_PREVIEWER_ASSET,
        32, 11 * BLOCK_SIZE, FONT_NORMAL_BLACK);
    list_box.height_blocks = outer_height_blocks - 4;
    list_box.x = 24;
    list_box.y = 12 * BLOCK_SIZE;
    list_box_request_refresh(&list_box);
    toggle_animation_button.x = list_box.x + 2;
    toggle_animation_button.y = list_box.y + list_box.height_blocks * BLOCK_SIZE;
    toggle_animation_button.width = list_box.width_blocks * BLOCK_SIZE;
    if (!data.hide_animation_frames) {
        text_draw(string_from_ascii("x"), toggle_animation_button.x + 6, toggle_animation_button.y + 3,
            FONT_NORMAL_BLACK, COLOR_MASK_NONE);
    }
    lang_text_draw(CUSTOM_TRANSLATION, TR_WINDOW_ASSET_PREVIEWER_SHOW_ANIMATION_FRAMES,
        toggle_animation_button.x + 28, toggle_animation_button.y + 4, FONT_NORMAL_BLACK);
}

static void encode_asset_id(const char *id)
{
    int size_needed = (int) strlen(id) + 1;
    if (size_needed > data.encoded_asset_id_size) {
        free(data.encoded_asset_id);
        data.encoded_asset_id = malloc(size_needed * sizeof(uint8_t));
        if (!data.encoded_asset_id) {
            data.encoded_asset_id_size = 0;
            return;
        }
        data.encoded_asset_id_size = size_needed;
    }
    encoding_from_utf8(id, data.encoded_asset_id, size_needed);
}

static void draw_asset_entry(const list_box_item *item)
{
    int current_image = data.entries[item->index].index;
    const asset_image *img = asset_image_get_from_id(data.active_group->first_image_index + current_image);
    font_t font = item->is_selected ? FONT_NORMAL_WHITE : FONT_NORMAL_GREEN;
    int width = text_draw_number(current_image + 1, '@', "", item->x, item->y, font, COLOR_MASK_NONE);
    const uint8_t *asset_name;
    if (img->id) {
        encode_asset_id(img->id);
        if (data.encoded_asset_id) {
            asset_name = data.encoded_asset_id;
        } else {
            asset_name = lang_get_string(CUSTOM_TRANSLATION, TR_WINDOW_ASSET_PREVIEWER_UNNAMED_ASSET);
        }
    } else {
        asset_name = lang_get_string(CUSTOM_TRANSLATION, data.entries[item->index].is_animation_frame ?
            TR_WINDOW_ASSET_PREVIEWER_ANIMATION_FRAME : TR_WINDOW_ASSET_PREVIEWER_UNNAMED_ASSET);
    }
    width += text_draw(string_from_ascii("-"), item->x + width, item->y, font, COLOR_MASK_NONE);
    text_draw_ellipsized(asset_name, item->x + width, item->y, item->width - width, font, COLOR_MASK_NONE);
    if (item->is_focused) {
        button_border_draw(item->x + 3, item->y - 5, item->width, item->height + 2, 1);
    }
}

static void draw_asset_list(void)
{
    list_box_draw(&list_box);
    if (!data.active_group || !list_box_get_total_items(&list_box)) {
        lang_text_draw_centered(CUSTOM_TRANSLATION, TR_WINDOW_ASSET_PREVIEWER_NO_ASSETS, list_box.x, list_box.y +
            (list_box.height_blocks * BLOCK_SIZE - 20) / 2, list_box.width_blocks * BLOCK_SIZE, FONT_NORMAL_GREEN);
    }
    button_border_draw(toggle_animation_button.x, toggle_animation_button.y,
        20, toggle_animation_button.height, data.animation_button_focused);
}

static void advance_animation_frame(const image *img)
{
    if (img->animation->can_reverse) {
        if (data.animation.reversed) {
            data.animation.frame--;
            if (data.animation.frame < 1) {
                data.animation.frame = 1;
                data.animation.reversed = 0;
            }
        } else {
            data.animation.frame++;
            if (data.animation.frame > img->animation->num_sprites) {
                data.animation.frame = img->animation->num_sprites;
                data.animation.reversed = 1;
            }
        }
    } else {
        data.animation.frame++;
        if (data.animation.frame > img->animation->num_sprites) {
            data.animation.frame = 1;
        }
    }
}

static void draw_foreground(void)
{
    for (unsigned int i = 0; i < NUM_BUTTONS; i++) {
        const generic_button *btn = &buttons[i];
        int x_offset = btn->x + data.x_offset_top + 16;
        int width = btn->width;
        int is_focused = data.focus_button_id == (i + 1);
        switch (btn->parameter1) {
            case BUTTON_CHANGE_ASSET_GROUP:
                if (data.xml_files->num_files == 0) {
                    is_focused = 0;
                }
                break;
            case BUTTON_TOGGLE_ANIMATIONS:
                width = 20;
                break;
            case BUTTON_CHANGE_ZOOM:
                width = lang_text_get_width(CUSTOM_TRANSLATION, TR_ZOOM, FONT_NORMAL_BLACK);
                x_offset += width;
                width = btn->width - width;
                break;
            default:
                break;
        }
        button_border_draw(x_offset, btn->y + 60, width, btn->height, is_focused);
    }

    draw_asset_list();

    if (data.showing_refresh_info && time_get_millis() - data.last_refresh > REFRESHED_INFO_TIME_MS) {
        data.showing_refresh_info = 0;
        window_invalidate();
    }
    if (data.last_refresh > 0 && !data.showing_refresh_info &&
        time_get_millis() - data.last_refresh <= REFRESHED_INFO_TIME_MS) {
        data.showing_refresh_info = 1;
        window_invalidate();
    }

    if (list_box_get_total_items(&list_box) == 0) {
        return;
    }

    int image_id = get_current_asset_index() + IMAGE_MAIN_ENTRIES;
    const image *img = image_get(image_id);
    if (data.animation.enabled && img->animation) {
        if (game_animation_should_advance(img->animation->speed_id)) {
            advance_animation_frame(img);
            window_invalidate();
        }
    }
}

static void recalculate_selected_index(void)
{
    int selected_index = list_box_get_selected_index(&list_box);
    int total_entries = list_box_get_total_items(&list_box);
    if (selected_index != LIST_BOX_NO_SELECTION) {
        if (selected_index < total_entries) {
            const asset_image *img =
                asset_image_get_from_id(get_current_asset_index());
            if (img->id && strcmp(data.selected_asset_id, img->id) == 0) {
                return;
            }
        }
        for (int i = 0; i < total_entries; i++) {
            const asset_image *img =
                asset_image_get_from_id(data.active_group->first_image_index + data.entries[i].index);
            if (img->id && data.selected_asset_id && strcmp(data.selected_asset_id, img->id) == 0) {
                list_box_select_index(&list_box, i);
                return;
            }
        }
    }
    if (selected_index >= total_entries) {
        list_box_select_index(&list_box, total_entries - 1);
    } else {
        list_box_select_index(&list_box, selected_index);
    }
}

static void refresh_window(void)
{
    int asset_index = list_box_get_total_items(&list_box) > 0 ?
        data.entries[list_box_get_scroll_position(&list_box)].index : 0;
    int group_changed = update_asset_groups_list();
    load_assets(group_changed);
    if (group_changed) {
        asset_index = 0;
    }
    recalculate_selected_index();
    window_invalidate();
    data.last_refresh = time_get_millis();
    data.showing_refresh_info = 0;
    int total_entries = list_box_get_total_items(&list_box);
    for (int i = 0; i < total_entries; i++) {
        if (data.entries[i].index == asset_index) {
            list_box_show_index(&list_box, i);
            return;
        }
    }
    list_box_show_index(&list_box, total_entries);
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (h->f5_pressed) {
        refresh_window();
    }
    if (list_box_handle_input(&list_box, m, 0)) {
        return;
    }
    if (!generic_buttons_handle_mouse(m, data.x_offset_top + 16, 60, buttons, NUM_BUTTONS, &data.focus_button_id)) {
        generic_buttons_handle_mouse(m, 0, 0, &toggle_animation_button, 1, &data.animation_button_focused);
    }
}

static void handle_tooltip(const list_box_item *item, tooltip_context *c)
{
    font_t font = item->is_selected ? FONT_NORMAL_WHITE : FONT_NORMAL_GREEN;

    const asset_image *img =
        asset_image_get_from_id(data.active_group->first_image_index + data.entries[item->index].index);
    if (!img || !img->id) {
        return;
    }
    encode_asset_id(img->id);
    if (!data.encoded_asset_id) {
        return;
    }
    int text_width = text_get_width(data.encoded_asset_id, font);
    int base_width = text_get_number_width(data.entries[item->index].index + 1, '@', "", font);
    base_width += text_get_width(string_from_ascii(" - "), font);

    if (text_width > item->width - base_width) {
        c->precomposed_text = data.encoded_asset_id;
        c->type = TOOLTIP_BUTTON;
    }
}

static void get_tooltip(tooltip_context *c)
{
    list_box_handle_tooltip(&list_box, c);
}

static void change_asset_group(int group)
{
    if (data.active_group_index != group) {
        data.active_group_index = group;
        load_assets(1);
        window_invalidate();
    }
}

static void set_terrain(int type)
{
    if (data.terrain != type) {
        data.terrain = type;
        load_climate(0);
        window_invalidate();
    }
}

static void set_zoom(int value)
{
    int selected_zoom = calc_percentage(100, ZOOM_VALUES[value]);
    if (selected_zoom != data.scale) {
        data.scale = selected_zoom;
        graphics_renderer()->update_scale(data.scale);
        window_invalidate();
    }
}

static void confirm_exit(int accepted, int checked)
{
    if (accepted) {
        system_exit();
    }
}

static void button_top(const generic_button *button)
{
    int option = button->parameter1;
    switch (option) {
        case BUTTON_CHANGE_ASSET_GROUP:
            if (data.xml_files->num_files > 0) {
                window_select_list_show_text(data.x_offset_top + 16, 60, button,
                    data.xml_file_names, data.xml_files->num_files, change_asset_group);
            }
            return;
        case BUTTON_CHANGE_TERRAIN:
            window_select_list_show_text(data.x_offset_top + 16, 60, button,
                data.terrain_texts, TERRAIN_MAX, set_terrain);
            return;
        case BUTTON_CHANGE_ZOOM:
            window_select_list_show_text(data.x_offset_top + 16, 60, button,
                data.zoom_texts, TOTAL_ZOOM_VALUES, set_zoom);
            return;
        case BUTTON_TOGGLE_ANIMATIONS:
            data.animation.enabled ^= 1;
            if (data.animation.enabled) {
                data.animation.frame = 1;
                data.animation.reversed = 0;
                game_animation_init();
            }
            window_invalidate();
            return;
        case BUTTON_REFRESH:
            refresh_window();
            return;
        case BUTTON_QUIT:
            window_popup_dialog_show(POPUP_DIALOG_QUIT, confirm_exit, 1);
            return;
    }
}

static void button_toggle_animation_frames(const generic_button *button)
{
    data.hide_animation_frames ^= 1;
    int asset_index = 0;
    int is_animation_frame = 0;
    if (list_box_get_total_items(&list_box) > 0) {
        asset_index = data.entries[list_box_get_scroll_position(&list_box)].index;
        is_animation_frame = data.entries[list_box_get_scroll_position(&list_box)].is_animation_frame;
        if (data.entries[list_box_get_selected_index(&list_box)].is_animation_frame && data.hide_animation_frames) {
            for (int i = list_box_get_selected_index(&list_box) - 1; i >= 0; i--) {
                if (!data.entries[i].is_animation_frame) {
                    list_box_select_index(&list_box, i);
                    break;
                }
            }
        }
    }
    int total_entries = update_entries();
    list_box_update_total_items(&list_box, total_entries);
    recalculate_selected_index();
    window_invalidate();

    for (unsigned int i = 0; i < list_box_get_total_items(&list_box); i++) {
        if (data.entries[i].index == asset_index ||
            (is_animation_frame && data.hide_animation_frames && data.entries[i].index > asset_index)) {
            list_box_show_index(&list_box, i);
            return;
        }
    }

    list_box_show_index(&list_box, 0);
}

int window_asset_previewer_show(void)
{
    window_type window = {
        WINDOW_ASSET_PREVIEWER,
        draw_background,
        draw_foreground,
        handle_input,
        get_tooltip
    };
    if (!init()) {
        return 0;
    }
    window_show(&window);
    return 1;
}

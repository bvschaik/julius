#include "rich_text.h"

#include "assets/assets.h"
#include "core/calc.h"
#include "core/file.h"
#include "core/image.h"
#include "core/image_group.h"
#include "core/locale.h"
#include "core/string.h"
#include "game/campaign.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/panel.h"
#include "graphics/scrollbar.h"
#include "graphics/window.h"

#include <stdlib.h>
#include <string.h>

#define MAX_LINKS 50
#define TEMP_LINE_SIZE 200

static void on_scroll(void);

static scrollbar_type scrollbar = {
    .has_y_margin = 1,
    .on_scroll_callback = on_scroll
};

static struct {
    int message_id;
    int x_min;
    int y_min;
    int x_max;
    int y_max;
} links[MAX_LINKS];

static uint8_t tmp_line[TEMP_LINE_SIZE];

static struct {
    const font_definition *normal_font;
    const font_definition *link_font;
    const font_definition *heading_font;
    int line_height;
    int paragraph_indent;

    int x_text;
    int y_text;
    int text_width_blocks;
    int text_height_blocks;
    int text_height_lines;
    int num_lines;
    int max_scroll_position;
    int num_links;
} data;

int rich_text_init(
    const uint8_t *text, int x_text, int y_text, int width_blocks, int height_blocks, int adjust_width_on_no_scroll)
{
    data.x_text = x_text;
    data.y_text = y_text;
    if (!data.num_lines) {
        data.text_height_blocks = height_blocks;
        data.text_height_lines = (height_blocks - 1) * BLOCK_SIZE / data.line_height;
        data.text_width_blocks = width_blocks;

        data.num_lines = rich_text_draw(text,
            data.x_text + 8, data.y_text + 6,
            BLOCK_SIZE * data.text_width_blocks - BLOCK_SIZE, data.text_height_lines, 1);
        scrollbar.x = data.x_text + BLOCK_SIZE * data.text_width_blocks - 1;
        scrollbar.y = data.y_text;
        scrollbar.height = BLOCK_SIZE * data.text_height_blocks;
        scrollbar.elements_in_view = data.text_height_lines;
        scrollbar_init(&scrollbar, scrollbar.scroll_position, data.num_lines);
        if (data.num_lines <= data.text_height_lines && adjust_width_on_no_scroll) {
            data.text_width_blocks += 2;
        }
        scrollbar.scrollable_width = BLOCK_SIZE * data.text_width_blocks;
        window_invalidate();
    }
    return data.text_width_blocks;
}

void rich_text_set_fonts(font_t normal_font, font_t heading_font, font_t link_font, int line_spacing)
{
    data.normal_font = font_definition_for(normal_font);
    data.heading_font = font_definition_for(heading_font);
    data.link_font = font_definition_for(link_font);
    data.line_height = data.normal_font->line_height + line_spacing;
    data.paragraph_indent = locale_paragraph_indent();
}

void rich_text_reset(int scroll_position)
{
    scrollbar_reset(&scrollbar, scroll_position);
    data.num_lines = 0;
    rich_text_clear_links();
}

void rich_text_clear_links(void)
{
    for (int i = 0; i < MAX_LINKS; i++) {
        links[i].message_id = 0;
        links[i].x_min = 0;
        links[i].x_max = 0;
        links[i].y_min = 0;
        links[i].y_max = 0;
    }
    data.num_links = 0;
}

int rich_text_get_clicked_link(const mouse *m)
{
    if (m->left.went_up) {
        for (int i = 0; i < data.num_links; i++) {
            if (m->x >= links[i].x_min && m->x <= links[i].x_max &&
                m->y >= links[i].y_min && m->y <= links[i].y_max) {
                return links[i].message_id;
            }
        }
    }
    return -1;
}

static void add_link(int message_id, int x_start, int x_end, int y)
{
    if (data.num_links < MAX_LINKS) {
        links[data.num_links].message_id = message_id;
        links[data.num_links].x_min = x_start - 2;
        links[data.num_links].x_max = x_end + 2;
        links[data.num_links].y_min = y - 1;
        links[data.num_links].y_max = y + 13;
        data.num_links++;
    }
}

static int get_word_width(const uint8_t *str, const font_definition *def, int in_link, int *num_chars, int line_start)
{
    int width = 0;
    int guard = 0;
    int word_char_seen = 0;
    int start_link = 0;
    *num_chars = 0;
    while (*str && ++guard < 2000) {
        if (*str == '@') {
            str++;
            if (*str == 'P' || *str == 'L' || *str == 'G' || *str == 'H') {
                *num_chars += 2;
                break;
            } else if (!word_char_seen) {
                (*num_chars)++;
                while (*str >= '0' && *str <= '9') {
                    str++;
                    (*num_chars)++;
                }
                in_link = 1;
                start_link = 1;
            }
        }
        int num_bytes = 1;
        if (*str == ' ') {
            if (word_char_seen) {
                break;
            }
            if (!line_start) {
                width += 4;
            }
        } else if (*str > ' ') {
            // normal char
            int letter_id = font_letter_id(def, str, &num_bytes);
            if (letter_id >= 0) {
                width += 1 + image_letter(letter_id)->original.width;
            }
            word_char_seen = 1;
            if (num_bytes > 1) {
                if (start_link) {
                    // add space before links in multibyte charsets
                    width += 4;
                    start_link = 0;
                }
                if (!in_link) {
                    *num_chars += num_bytes;
                    break;
                }
            }
        }
        str += num_bytes;
        *num_chars += num_bytes;
    }
    return width;
}

static void draw_line(const uint8_t *str, const font_definition *font, int x, int y, color_t color, int measure_only)
{
    int start_link = 0;
    int num_link_chars = 0;
    const font_definition *def = font;
    while (*str) {
        if (*str == '@') {
            int message_id = string_to_int(++str);
            while (*str >= '0' && *str <= '9') {
                str++;
            }
            int width = get_word_width(str, data.link_font, 1, &num_link_chars, 0);
            add_link(message_id, x, x + width, y);
            start_link = 1;
        }
        if (*str >= ' ') {
            def = num_link_chars > 0 ? data.link_font : font;
            int num_bytes = 1;
            int letter_id = font_letter_id(def, str, &num_bytes);
            if (letter_id < 0) {
                x += def->space_width;
            } else {
                if (num_bytes > 1 && start_link) {
                    // add space before links in multibyte charsets
                    x += def->space_width;
                    start_link = 0;
                }
                const image *img = image_letter(letter_id);
                if (!measure_only) {
                    int height = def->image_y_offset(*str, img->height + img->y_offset, def->line_height);
                    image_draw_letter(def->font, letter_id, x, y - height, color, SCALE_NONE);
                }
                x += img->original.width + def->letter_spacing;
            }
            if (num_link_chars > 0) {
                num_link_chars -= num_bytes;
            }
            str += num_bytes;
        } else {
            str++;
        }
    }
}

static int get_raw_text_width(const uint8_t *str)
{
    int width = 0;
    int guard = 0;
    while (*str && ++guard < 2000) {
        int num_bytes = 1;
        int letter_id = font_letter_id(data.normal_font, str, &num_bytes);
        if (letter_id >= 0) {
            width += 1 + image_letter(letter_id)->original.width;
        }
        str += num_bytes;
    }
    return width;
}

static int get_external_image_id(const char *filename)
{
    char full_path[FILE_NAME_MAX];
    char *paths[] = { CAMPAIGNS_DIRECTORY "/image", "image" };
    const char *found_path = 0;
    for (int i = 0; i < 2 && !found_path; i++) {
        snprintf(full_path, FILE_NAME_MAX, "%s/%s", paths[i], filename);
        if (game_campaign_has_file(full_path)) {
            found_path = full_path;
        } else {
            found_path = dir_get_file_at_location(full_path, PATH_LOCATION_COMMUNITY);
        }
    }
    if (!found_path) {
        return 0;
    }
    return assets_get_external_image(found_path, 0);
}

int rich_text_parse_image_id(const uint8_t **position, int default_image_group, int can_be_filepath)
{
    int image_id = 0;

    if (can_be_filepath) {
        int length = string_length(*position) + 1;
        char *location = malloc(length * sizeof(char));
        if (location) {
            encoding_to_utf8(*position, location, length, encoding_system_uses_decomposed());
            image_id = get_external_image_id(location);
            free(location);
        }
        if (image_id) {
            return image_id;
        }
    }
    const uint8_t *cursor = *position;
    if (*cursor == '[') {
        cursor++; // skip '['
        const char *begin = (const char *) cursor;
        const char *end = strchr(begin, ']');
        if (!end) {
            *position = 0;
            return 0;
        }
        size_t length = end - begin;
        cursor += length + 1;
        char *location = malloc((length + 1) * sizeof(char));
        if (location) {
            snprintf(location, length + 1, "%s", begin);
            char *divider = strchr(location, ':');
            // "[<asset group name>:<asset image name>]"
            if (divider) {
                *divider = 0;
                const char *group_name = location;
                const char *image_name = divider + 1;
                image_id = assets_get_image_id(group_name, image_name);
                // "[<asset png path>]"
            } else {
                image_id = get_external_image_id(location);
            }
            free(location);
        }
    } else {
        int custom_group = default_image_group;
        image_id = string_to_int(cursor);
        char c = *cursor++;
        if (image_id || c == '0') {
            while (c >= '0' && c <= '9') {
                c = *cursor++;
            }
            if (c == ':') {
                int actual_image_id = string_to_int(cursor);
                c = *cursor++;
                if (actual_image_id || c == '0') {
                    while (c >= '0' && c <= '9') {
                        c = *cursor++;
                    }
                    if (actual_image_id) {
                        custom_group = image_id;
                        image_id = actual_image_id;
                    }
                }
            }
            image_id += image_group(custom_group) - 1;
        }
    }
    *position = cursor;
    return image_id;
}

static int draw_text(const uint8_t *text, int x_offset, int y_offset,
                     int box_width, unsigned int height_lines, color_t color, int measure_only)
{
    if (!measure_only) {
        graphics_set_clip_rectangle(x_offset, y_offset, box_width, data.line_height * height_lines);
        if (height_lines != scrollbar.elements_in_view) {
            scrollbar.elements_in_view = height_lines;
            scrollbar_update_total_elements(&scrollbar, data.num_lines);
        }
    }
    int lines_to_skip = 0;
    int image_id = 0;
    int lines_before_image = 0;
    int paragraph = 0;
    int has_more_characters = 1;
    int y = y_offset;
    int guard = 0;
    unsigned int line = 0;
    unsigned int num_lines = 0;
    int heading = 0;
    while (has_more_characters || lines_to_skip) {
        if (++guard >= 1000) {
            break;
        }
        // clear line
        for (int i = 0; i < TEMP_LINE_SIZE; i++) {
            tmp_line[i] = 0;
        }
        int line_index = 0;
        int line_break = 0;
        int x_line_offset = paragraph ? data.paragraph_indent : 0;
        int current_width = x_line_offset;
        const font_definition *def = heading ? data.heading_font : data.normal_font;
        paragraph = 0;
        int centered = heading;
        while (!line_break && (has_more_characters || lines_to_skip)) {
            if (lines_to_skip) {
                lines_to_skip--;
                break;
            }
            int word_num_chars;
            int word_width = get_word_width(text, def, 0, &word_num_chars, line_index == 0);
            if (word_width >= box_width) {
                // Word too long to fit on a line, so cut it into smaller pieces.
                int can_cut_more = 1;
                while (can_cut_more) {
                    char c = *text;
                    if (c == '@') {
                        break;
                    }
                    text++;
                    tmp_line[line_index++] = c;
                    int temp_width = get_raw_text_width(tmp_line);
                    can_cut_more = (temp_width < box_width);
                }
            }
            if (current_width + word_width >= box_width) {
                line_break = 1;
                if (current_width == 0) {
                    has_more_characters = 0;
                }
            } else {
                current_width += word_width;
                for (int i = 0; i < word_num_chars; i++) {
                    char c = *text++;
                    if (c == '@') {
                        // Heading
                        if (*text == 'H') {
                            heading = 1;
                            if (line_index) {
                                line_break = 1;
                            }
                            if (line > 0) {
                                lines_to_skip = 1;
                            } else {
                                def = data.heading_font;
                                centered = 1;
                            }
                            text++;
                            break;
                        // Paragraph
                        } else if (*text == 'P') {
                            paragraph = 1;
                            if (heading) {
                                heading = 0;
                                lines_to_skip = 1;
                            }
                            text++;
                            line_break = 1;
                            break;
                        // Line break
                        } else if (*text == 'L') {
                            text++;
                            line_break = 1;
                            if (heading) {
                                heading = 0;
                                lines_to_skip = 1;
                            }
                            break;
                            // Image
                        } else if (*text == 'G') {
                            heading = 0;
                            if (line_index) {
                                num_lines++;
                            }
                            text++; // skip 'G'
                            line_break = 1;
                            // "@G[...]"
                            image_id = rich_text_parse_image_id(&text, GROUP_MESSAGE_IMAGES, 0);
                            if (!text) {
                                has_more_characters = 0;
                                break;
                            }
                            const image *img = image_get(image_id);
                            int height = img->original.height;
                            if (img->top) {
                                height += img->top->original.height - height / 2;
                            }
                            lines_to_skip = height / data.line_height;
                            if ((height % data.line_height) > data.line_height / 2) {
                                lines_to_skip++;
                            }
                            if (line > 0 || line_index) {
                                lines_before_image = 1;
                            }
                            break;
                        }
                    }
                    if (line_index || c != ' ') { // no space at start of line
                        tmp_line[line_index++] = c;
                    }
                }
                if (!*text) {
                    has_more_characters = 0;
                }
            }
        }

        int outside_viewport = 0;
        if (!measure_only) {
            if (line < scrollbar.scroll_position || line >= scrollbar.scroll_position + height_lines) {
                outside_viewport = 1;
            }
        }
        if (!outside_viewport) {
            if (centered) {
                x_line_offset = (box_width - current_width) / 2;
            }
            draw_line(tmp_line, def, x_line_offset + x_offset, y, color, measure_only);
        }
        if (!measure_only) {
            if (image_id) {
                if (lines_before_image) {
                    lines_before_image--;
                } else {
                    const image *img = image_get(image_id);
                    int height = img->original.height;
                    if (img->top) {
                        height += img->top->original.height - height / 2;
                    }
                    lines_to_skip = height / data.line_height;
                    if ((height % data.line_height) > data.line_height / 2) {
                        lines_to_skip++;
                    }
                    int image_offset_x = x_offset + (box_width - img->original.width) / 2 - 4;
                    if (line < height_lines + scrollbar.scroll_position) {
                        if (line >= scrollbar.scroll_position) {
                            image_draw(image_id, image_offset_x, y + 8, COLOR_MASK_NONE, SCALE_NONE);
                        } else {
                            image_draw(image_id, image_offset_x,
                                y + 8 - data.line_height * (scrollbar.scroll_position - line),
                                COLOR_MASK_NONE, SCALE_NONE);
                        }
                    }
                    image_id = 0;
                }
            }
        }
        line++;
        num_lines++;
        if (!outside_viewport) {
            y += data.line_height;
        }
    }
    if (!measure_only) {
        graphics_reset_clip_rectangle();
    }
    return num_lines;
}

int rich_text_draw(const uint8_t *text, int x_offset, int y_offset, int box_width, int height_lines, int measure_only)
{
    return draw_text(text, x_offset, y_offset, box_width, height_lines, 0, measure_only);
}

int rich_text_draw_colored(
    const uint8_t *text, int x_offset, int y_offset, int box_width, int height_lines, color_t color)
{
    return draw_text(text, x_offset, y_offset, box_width, height_lines, color, 0);
}

void rich_text_draw_scrollbar(void)
{
    scrollbar_draw(&scrollbar);
}

int rich_text_handle_mouse(const mouse *m)
{
    return scrollbar_handle_mouse(&scrollbar, m, 1);
}

static void on_scroll(void)
{
    rich_text_clear_links();
    window_invalidate();
}

int rich_text_scroll_position(void)
{
    return scrollbar.scroll_position;
}

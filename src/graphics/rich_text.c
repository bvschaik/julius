#include "rich_text.h"

#include "core/calc.h"
#include "core/image.h"
#include "core/string.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/window.h"

#define MAX_LINKS 50

static image_button image_button_scroll_up = {
    0, 0, 39, 26, IB_SCROLL, 96, 8, rich_text_scroll, button_none, 0, 1, 1
};
static image_button image_button_scroll_down = {
    0, 0, 39, 26, IB_SCROLL, 96, 12, rich_text_scroll, button_none, 1, 1, 1
};

static struct {
    int message_id;
    int x_min;
    int y_min;
    int x_max;
    int y_max;
} links[MAX_LINKS];

static int num_links;
static const font_definition *normal_font_def;
static const font_definition *link_font_def;

static uint8_t tmp_line[200];

static struct {
    int x_text;
    int y_text;
    int text_width_blocks;
    int text_height_blocks;
    int text_height_lines;
    int num_lines;
    int scroll_position;
    int max_scroll_position;
    int is_dragging_scroll;
    int scroll_position_drag;
    struct {
        int scroll_position;
        int num_lines;
        int text_height_lines;
    } backup;
} data;

int rich_text_init(const uint8_t *text, int x_text, int y_text, int width_blocks, int height_blocks, int adjust_width_on_no_scroll)
{
    data.x_text = x_text;
    data.y_text = y_text;
    if (!data.num_lines) {
        data.text_height_blocks = height_blocks;
        data.text_height_lines = height_blocks - 1;
        data.text_width_blocks = width_blocks;
        
        image_button_scroll_up.enabled = 1;
        image_button_scroll_down.enabled = 1;

        data.num_lines = rich_text_draw(text,
            data.x_text + 8, data.y_text + 6,
            16 * data.text_width_blocks - 16, data.text_height_lines, 1);
        if (data.num_lines <= data.text_height_lines) {
            if (adjust_width_on_no_scroll) {
                data.text_width_blocks += 2;
            }
            data.max_scroll_position = 0;
            image_button_scroll_up.enabled = 0;
            image_button_scroll_down.enabled = 0;
        } else {
            data.max_scroll_position = data.num_lines - data.text_height_lines;
        }
        window_invalidate();
    }
    return data.text_width_blocks;
}

void rich_text_set_fonts(font_t normal_font, font_t link_font)
{
    normal_font_def = font_definition_for(normal_font);
    link_font_def = font_definition_for(link_font);
}

void rich_text_reset(int scroll_position)
{
    data.scroll_position = scroll_position;
    data.num_lines = 0;
    data.is_dragging_scroll = 0;
    rich_text_clear_links();
}

void rich_text_save(void)
{
    data.backup.num_lines = data.num_lines;
    data.backup.scroll_position = data.scroll_position;
    data.backup.text_height_lines = data.text_height_lines;
    data.scroll_position = 0;
    data.text_height_lines = 30;
}

void rich_text_restore(void)
{
    data.num_lines = data.backup.num_lines;
    data.scroll_position = data.backup.scroll_position;
    data.text_height_lines = data.backup.text_height_lines;
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
    num_links = 0;
}

int rich_text_get_clicked_link(const mouse *m)
{
    if (m->left.went_down) {
        for (int i = 0; i < num_links; i++) {
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
    if (num_links < MAX_LINKS) {
        links[num_links].message_id = message_id;
        links[num_links].x_min = x_start - 2;
        links[num_links].x_max = x_end + 2;
        links[num_links].y_min = y - 1;
        links[num_links].y_max = y + 13;
        num_links++;
    }
}

static int get_word_width(const uint8_t *str, int *num_chars)
{
    int width = 0;
    int guard = 0;
    int word_char_seen = 0;
    *num_chars = 0;
    for (uint8_t c = *str; c; c = *++str) {
        if (++guard >= 2000) {
            break;
        }
        if (c == '@') {
            c = *++str;
            if (!word_char_seen) {
                if (c == 'P' || c == 'L') {
                    *num_chars += 2;
                    width = 0;
                    break;
                } else if (c == 'G') {
                    // skip graphic
                    *num_chars += 2;
                    while (c >= '0' && c <= '9') {
                        c = *++str;
                        (*num_chars)++;
                    }
                    width = 0;
                    break;
                } else {
                    (*num_chars)++;
                    while (c >= '0' && c <= '9') {
                        c = *++str;
                        (*num_chars)++;
                    }
                }
            }
        }
        if (c == ' ') {
            if (word_char_seen) {
                break;
            }
            width += 4;
        } else if (c > ' ') {
            // normal char
            int letter_id = font_letter_id(normal_font_def, c);
            if (letter_id >= 0) {
                width += 1 + image_letter(letter_id)->width;
            }
            word_char_seen = 1;
        }
        (*num_chars)++;
    }
    return width;
}

static int draw_character(const font_definition *def, uint8_t c, int x, int y, color_t color, int measure_only)
{
    int letter_id = font_letter_id(def, c);
    if (letter_id < 0) {
        return def->space_width_draw;
    }

    const image *img = image_letter(letter_id);
    if (!measure_only) {
        int height = def->image_y_offset(c, img->height, 11);
        image_draw_letter(letter_id, x, y - height, color);
    }
    return img->width;
}

static void draw_line(const uint8_t *str, int x, int y, color_t color, int measure_only)
{
    int num_link_chars = 0;
    for (uint8_t c = *str; c; c = *++str) {
        if (c == '@') {
            int message_id = string_to_int(++str);
            while (*str >= '0' && *str <= '9') {
                str++;
            }
            int width = get_word_width(str, &num_link_chars);
            add_link(message_id, x, x + width, y);
            c = *str;
        }
        if (c >= ' ') {
            const font_definition *def = normal_font_def;
            if (num_link_chars > 0) {
                def = link_font_def;
                num_link_chars--;
            }
            x += draw_character(def, c, x, y, color, measure_only);
        }
    }
}

static int draw_text(const uint8_t *text, int x_offset, int y_offset,
                     int box_width, int height_lines, color_t color, int measure_only)
{
    int image_height_lines = 0;
    int image_id = 0;
    int lines_before_image = 0;
    int paragraph = 0;
    int has_more_characters = 1;
    int y = y_offset;
    int guard = 0;
    int line = 0;
    int num_lines = 0;
    while (has_more_characters || image_height_lines) {
        if (++guard >= 1000) {
            break;
        }
        // clear line
        for (int i = 0; i < 200; i++) {
            tmp_line[i] = 0;
        }
        int line_index = 0;
        int current_width, x_line_offset;
        current_width = x_line_offset = paragraph ? 50 : 0;
        paragraph = 0;
        while ((has_more_characters || image_height_lines) && current_width < box_width) {
            if (image_height_lines) {
                image_height_lines--;
                break;
            }
            int word_num_chars;
            current_width += get_word_width(text, &word_num_chars);
            if (current_width >= box_width) {
                if (current_width == 0) {
                    has_more_characters = 0;
                }
            } else {
                for (int i = 0; i < word_num_chars; i++) {
                    char c = *text++;
                    if (c == '@') {
                        if (*text == 'P') {
                            paragraph = 1;
                            text++;
                            current_width = box_width;
                            break;
                        } else if (*text == 'L') {
                            text++;
                            current_width = box_width;
                            break;
                        } else if (*text == 'G') {
                            if (line_index) {
                                num_lines++;
                            }
                            text++; // skip 'G'
                            current_width = box_width;
                            image_id = string_to_int(text);
                            c = *text++;
                            while (c >= '0' && c <= '9') {
                                c = *text++;
                            }
                            image_id += image_group(GROUP_MESSAGE_IMAGES) - 1;
                            image_height_lines = image_get(image_id)->height / 16 + 2;
                            if (line > 0) {
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
            if (line < data.scroll_position || line >= data.scroll_position + height_lines) {
                outside_viewport = 1;
            }
        }
        if (!outside_viewport) {
            draw_line(tmp_line, x_line_offset + x_offset, y, color, measure_only);
        }
        if (!measure_only) {
            if (image_id) {
                if (lines_before_image) {
                    lines_before_image--;
                } else {
                    const image *img = image_get(image_id);
                    image_height_lines = img->height / 16 + 2;
                    int image_offset_x = x_offset + (box_width - img->width) / 2 - 4;
                    if (line < height_lines + data.scroll_position) {
                        if (line >= data.scroll_position) {
                            image_draw(image_id, image_offset_x, y + 8);
                        } else {
                            image_draw(image_id, image_offset_x, y + 8 - 16 * (data.scroll_position - line));
                        }
                    }
                    image_id = 0;
                }
            }
        }
        line++;
        num_lines++;
        if (!outside_viewport) {
            y += 16;
        }
    }
    return num_lines;
}

int rich_text_draw(const uint8_t *text, int x_offset, int y_offset, int box_width, int height_lines, int measure_only)
{
    return draw_text(text, x_offset, y_offset, box_width, height_lines, 0, measure_only);
}

int rich_text_draw_colored(const uint8_t *text, int x_offset, int y_offset, int box_width, int height_lines, color_t color)
{
    return draw_text(text, x_offset, y_offset, box_width, height_lines, color, 0);
}

void rich_text_draw_scrollbar(void)
{
    if (data.max_scroll_position) {
        image_buttons_draw(
            data.x_text + 16 * data.text_width_blocks - 1,
            data.y_text,
            &image_button_scroll_up, 1);
        image_buttons_draw(
            data.x_text + 16 * data.text_width_blocks - 1,
            data.y_text + 16 * data.text_height_blocks - 26,
            &image_button_scroll_down, 1);
        rich_text_draw_scrollbar_dot();
    }
}

void rich_text_draw_scrollbar_dot(void)
{
    if (data.max_scroll_position) {
        int pct;
        if (data.scroll_position <= 0) {
            pct = 0;
        } else if (data.scroll_position >= data.max_scroll_position) {
            pct = 100;
        } else {
            pct = calc_percentage(data.scroll_position, data.max_scroll_position);
        }
        int offset = calc_adjust_with_percentage(16 * data.text_height_blocks - 77, pct);
        if (data.is_dragging_scroll) {
            offset = data.scroll_position_drag;
        }
        image_draw(image_group(GROUP_PANEL_BUTTON) + 39,
            data.x_text + 16 * data.text_width_blocks + 6, data.y_text + offset + 26);
    }
}

static int handle_scrollbar_dot(const mouse *m)
{
    if (data.max_scroll_position <= 0 || !m->left.is_down) {
        return 0;
    }
    int total_height = 16 * data.text_height_blocks - 52;
    if (m->x < data.x_text + 16 * data.text_width_blocks + 1 ||
        m->x > data.x_text + 16 * data.text_width_blocks + 41) {
        return 0;
    }
    if (m->y < data.y_text + 26 || m->y > data.y_text + 26 + total_height) {
        return 0;
    }
    int dot_height = m->y - data.y_text - 11;
    if (dot_height > total_height) {
        dot_height = total_height;
    }
    int pct_scrolled = calc_percentage(dot_height, total_height);
    data.scroll_position = calc_adjust_with_percentage(data.max_scroll_position, pct_scrolled);
    data.is_dragging_scroll = 1;
    data.scroll_position_drag = dot_height - 25;
    rich_text_clear_links();
    if (data.scroll_position_drag < 0) {
        data.scroll_position_drag = 0;
    }
    window_invalidate();
    return 1;
}

int rich_text_handle_mouse(const mouse *m)
{
    if (m->scrolled == SCROLL_DOWN) {
        rich_text_scroll(1, 3);
    } else if (m->scrolled == SCROLL_UP) {
        rich_text_scroll(0, 3);
    }

    if (image_buttons_handle_mouse(
        m, data.x_text + 16 * data.text_width_blocks - 1, data.y_text,
        &image_button_scroll_up, 1, 0)) {
            return 1;
    }
    if (image_buttons_handle_mouse(m,
        data.x_text + 16 * data.text_width_blocks - 1,
        data.y_text + 16 * data.text_height_blocks - 26,
        &image_button_scroll_down, 1, 0)) {
            return 1;
    }
    return handle_scrollbar_dot(m);
}

void rich_text_scroll(int is_down, int num_lines)
{
    if (is_down) {
        data.scroll_position += num_lines;
        if (data.scroll_position > data.max_scroll_position) {
            data.scroll_position = data.max_scroll_position;
        }
    } else {
        data.scroll_position -= num_lines;
        if (data.scroll_position < 0) {
            data.scroll_position = 0;
        }
    }
    rich_text_clear_links();
    data.is_dragging_scroll = 0;
    window_invalidate();
}

int rich_text_scroll_position(void)
{
    return data.scroll_position;
}

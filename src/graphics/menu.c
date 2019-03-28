#include "menu.h"

#include "graphics/graphics.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"

void menu_bar_draw(menu_bar_item *items, int num_items)
{
    short x_offset = items[0].x_start;
    for (int i = 0; i < num_items; i++) {
        items[i].x_start = x_offset;
        x_offset += lang_text_draw(items[i].text_group, 0, x_offset, items[i].y_start, FONT_NORMAL_GREEN);
        items[i].x_end = x_offset;
        x_offset += 32; // spacing
    }
}

static int get_menu_bar_item(const mouse *m, menu_bar_item *items, int num_items)
{
    for (int i = 0; i < num_items; i++) {
        if (items[i].x_start <= m->x &&
            items[i].x_end > m->x &&
            items[i].y_start <= m->y &&
            items[i].y_start + 12 > m->y) {
            return i + 1;
        }
    }
    return 0;
}

int menu_bar_handle_mouse(const mouse *m, menu_bar_item *items, int num_items, int *focus_menu_id)
{
    int menu_id = get_menu_bar_item(m, items, num_items);
    if (focus_menu_id) {
        *focus_menu_id = menu_id;
    }
    return menu_id;
}

static void calculate_menu_width(menu_bar_item *menu)
{
    int max_width = 0;
    for (int i = 0; i < menu->num_items; i++) {
        menu_item *sub = &menu->items[i];
        int width_pixels = lang_text_get_width(
            menu->text_group, sub->text_number, FONT_NORMAL_BLACK);
        if (width_pixels > max_width) {
            max_width = width_pixels;
        }
    }
    int blocks = (max_width + 15) / 16 + 1; // 1 block padding
    menu->calculated_width_blocks = blocks < 10 ? 10 : blocks;
}

void menu_draw(menu_bar_item *menu, int focus_item_id)
{
    if (menu->calculated_width_blocks == 0) {
        calculate_menu_width(menu);
    }
    unbordered_panel_draw(menu->x_start, menu->y_start + 18,
        menu->calculated_width_blocks, (20 + 20 * menu->num_items) / 16);
    for (int i = 0; i < menu->num_items; i++) {
        menu_item *sub = &menu->items[i];
        int y_offset = 30 + menu->y_start + sub->y_start;
        if (i == focus_item_id - 1) {
            graphics_fill_rect(menu->x_start, y_offset - 2,
                16 * menu->calculated_width_blocks, 16, COLOR_BLACK);
            lang_text_draw_colored(menu->text_group, sub->text_number,
                menu->x_start + 8, y_offset, FONT_NORMAL_PLAIN, COLOR_ORANGE);
        } else {
            lang_text_draw(menu->text_group, sub->text_number,
                menu->x_start + 8, y_offset, FONT_NORMAL_BLACK);
        }
    }
}

static int get_menu_item(const mouse *m, menu_bar_item *menu)
{
    for (int i = 0; i < menu->num_items; i++) {
        if (menu->x_start <= m->x &&
            menu->x_start + 16 * menu->calculated_width_blocks > m->x &&
            menu->y_start + menu->items[i].y_start + 30 <= m->y &&
            menu->y_start + menu->items[i].y_start + 45 > m->y) {
            return i + 1;
        }
    }
    return 0;
}

int menu_handle_mouse(const mouse *m, menu_bar_item *menu, int *focus_item_id)
{
    int item_id = get_menu_item(m, menu);
    if (focus_item_id) {
        *focus_item_id = item_id;
    }
    if (!item_id) {
        return 0;
    }
    if (m->left.went_down) {
        menu_item *item = &menu->items[item_id -1];
        item->left_click_handler(item->parameter);
    }
    return item_id;
}

void menu_update_text(menu_bar_item *menu, int index, int text_number)
{
    menu->items[index].text_number = text_number;
    if (menu->calculated_width_blocks > 0) {
        int item_width = lang_text_get_width(
            menu->text_group, text_number, FONT_NORMAL_BLACK);
        int blocks = (item_width + 15) / 16 + 1;
        if (blocks > menu->calculated_width_blocks) {
            menu->calculated_width_blocks = blocks;
        }
    }
}

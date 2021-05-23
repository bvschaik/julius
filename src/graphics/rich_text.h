#ifndef GRAPHICS_RICH_TEXT_H
#define GRAPHICS_RICH_TEXT_H

#include "graphics/color.h"
#include "graphics/font.h"
#include "input/mouse.h"

/**
 * Initializes the rich text
 * @param text Text to show
 * @param x_text X offset for the text
 * @param y_text Y offset for the text
 * @param width_blocks Width of the text in blocks of 16px
 * @param height_blocks Height of the text in blocks of 16px
 * @param adjust_width_on_no_scroll Whether to expand the text area into the scrollbar area
 * @return Width of the text in blocks
 */
int rich_text_init(
    const uint8_t *text, int x_text, int y_text, int width_blocks, int height_blocks, int adjust_width_on_no_scroll);

/**
 * Sets fonts to use
 * @param normal_font Normal text
 * @param link_font Link text
 * @param line_spacing Spacing between lines
 */
void rich_text_set_fonts(font_t normal_font, font_t link_font, int line_spacing);

/**
 * Resets the text to the specified scroll position and forces recalculation of lines
 */
void rich_text_reset(int scroll_position);

/**
 * Clear the links table
 */
void rich_text_clear_links(void);

/**
 * Get the clicked link, if any
 * @param m Mouse state
 * @return ID of the link (>= 0), or -1 if no link was clicked
 */
int rich_text_get_clicked_link(const mouse *m);

/**
 * Draws rich text
 * @param text Text to draw
 * @param x_offset X offset
 * @param y_offset Y offset
 * @param box_width Width of the box in pixels
 * @param height_lines Number of available lines
 * @param measure_only True to only measure text, not draw it
 * @return Total number of lines required for the text
 */
int rich_text_draw(const uint8_t *text, int x_offset, int y_offset,
                   int box_width, int height_lines, int measure_only);

/**
 * Draws rich text with specified color
 * @param text Text to draw
 * @param x_offset X offset
 * @param y_offset Y offset
 * @param box_width Width of the box in pixels
 * @param height_lines Number of available lines
 * @param color Color to draw with
 * @return Total number of lines required for the text
 */
int rich_text_draw_colored(const uint8_t *text, int x_offset, int y_offset,
                           int box_width, int height_lines, color_t color);

/**
 * Draws the scrollbar
 */
void rich_text_draw_scrollbar(void);

/**
 * Handles mouse interaction with the scrollbar and scroll wheel
 * @param m Mouse state
 * @return True if any interaction was handled
 */
int rich_text_handle_mouse(const mouse *m);

/**
 * Gets scroll position in absolute number of lines
 */
int rich_text_scroll_position(void);

#endif // GRAPHICS_RICH_TEXT_H

#ifndef CORE_IMAGE_H
#define CORE_IMAGE_H

#include "core/encoding.h"
#include "core/image_group.h"
#include "graphics/color.h"

#define IMAGE_MAIN_ENTRIES 10000

#define IMAGE_FONT_MULTIBYTE_OFFSET 10000
#define IMAGE_FONT_MULTIBYTE_TRAD_CHINESE_MAX_CHARS 2188
#define IMAGE_FONT_MULTIBYTE_SIMP_CHINESE_MAX_CHARS 2130
#define IMAGE_FONT_MULTIBYTE_KOREAN_MAX_CHARS 2350
#define IMAGE_FONT_MULTIBYTE_JAPANESE_MAX_CHARS 3321

#define IMAGE_ATLAS_BIT_OFFSET 28
#define IMAGE_ATLAS_BIT_MASK 0x0fffffff

#define FOOTPRINT_WIDTH 58
#define FOOTPRINT_HEIGHT 30
#define FOOTPRINT_HALF_HEIGHT 15

/**
 * @file
 * Image functions
 */

/**
 * Image metadata
 */
typedef struct {
    int x_offset;
    int y_offset;
    int width;
    int height;
    int is_isometric;
    int top_height;
    struct {
        int num_sprites;
        int sprite_offset_x;
        int sprite_offset_y;
        int can_reverse;
        int speed_id;
        int start_offset;
    } animation;
    struct {
        int id;
        int x_offset;
        int y_offset;
    } atlas;
} image;

/**
 * Loads the image collection for the specified climate
 * @param climate_id Climate to load
 * @param is_editor Whether to load the editor graphics or not
 * @param force_reload Whether to force loading graphics even if climate/editor are the same
 * @return boolean true on success, false on failure
 */
int image_load_climate(int climate_id, int is_editor, int force_reload);

/**
 * Loads external fonts file (Cyrillic and Traditional Chinese)
 * @return boolean true on success, false on failure
 */
int image_load_fonts(encoding_type encoding);

/**
 * Loads the image collection for the specified enemy
 * @param enemy_id Enemy to load
 * @return boolean true on success, false on failure
 */
int image_load_enemy(int enemy_id);

/**
 * Indicates whether an image is external or not
 * @param image_id Image to check
 * @return 1 if image is external, 0 otherwise
 */
int image_is_external(int image_id);

/**
 * Loads the pixel data of an external image
 * @param dst The pixel buffer where the image data will be stored
 * @param image_id Image to load
 * @param row_width The width of the pixel buffer, in pixels
 * @return 1 if successful, 0 otherwise
 */
int image_load_external_pixels(color_t *dst, int image_id, int row_width);

/**
 * Loads the external data of an image
 * @param image_id Image to load
 */
void image_load_external_data(int image_id);

/**
 * Crops the transparent pixels around an image
 * @param img The image to crop
 * @param pixels The pixel data of the image
 * @param reduce_width Whether to crop the width as well
 */
void image_crop(image *img, const color_t *pixels, int reduce_width);

/**
 * Gets the image id of the first image in the group
 * @param group Image group
 * @return Image id of first image
 */
int image_group(int group);

/**
 * Gets an image by id
 * @param id Image ID
 * @return Image
 */
const image *image_get(int id);

/**
 * Gets a letter image by offset within font group
 * @param letter_id Letter offset
 * @return Image
 */
const image *image_letter(int letter_id);

/**
 * Gets an enemy image by id
 * @param id Enemy image ID
 * @return Enemy image
 */
const image *image_get_enemy(int id);

#endif // CORE_IMAGE_H

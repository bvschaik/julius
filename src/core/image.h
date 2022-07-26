#ifndef CORE_IMAGE_H
#define CORE_IMAGE_H

#include "core/encoding.h"
#include "core/image_group.h"
#include "graphics/color.h"

#define IMAGE_MAIN_ENTRIES 10000
#define IMAGE_MAX_GROUPS 300

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
 * Image animation metadata
 */
typedef struct {
    int num_sprites;
    int sprite_offset_x;
    int sprite_offset_y;
    int can_reverse;
    int speed_id;
    int start_offset;
} image_animation;

/**
 * Image metadata
 */
typedef struct image {
    int x_offset;
    int y_offset;
    int width;
    int height;
    struct {
        int width;
        int height;
    } original;
    int is_isometric;
    struct image *top;
    image_animation *animation;
    struct {
        int id;
        int x_offset;
        int y_offset;
    } atlas;
} image;

/**
 * Image copy information
 */
typedef struct {
    struct {
        int x;
        int y;
        int width;
        int height;
        const color_t *pixels;
    } src;
    struct {
        int x;
        int y;
        int width;
        int height;
        color_t *pixels;
    } dst;
    struct {
        int x_offset;
        int y_offset;
        int width;
        int height;
    } rect;
} image_copy_info;

/**
 * Loads the image collection for the specified climate
 * @param climate_id Climate to load
 * @param is_editor Whether to load the editor graphics or not
 * @param force_reload Whether to force loading graphics even if climate/editor are the same
 * @param keep_atlas_buffers Whether to keep the atlas buffers in memory after the images are created
 * @return boolean true on success, false on failure
 */
int image_load_climate(int climate_id, int is_editor, int force_reload, int keep_atlas_buffers);

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
 * @param img Image to check
 * @return 1 if image is external, 0 otherwise
 */
int image_is_external(const image *img);

/**
 * Loads the pixel data of an external image
 * @param dst The pixel buffer where the image data will be stored
 * @param img Image to load
 * @param row_width The width of the pixel buffer, in pixels
 * @return 1 if successful, 0 otherwise
 */
int image_load_external_pixels(color_t *dst, const image *img, int row_width);

/**
 * Loads the external data of an image
 * @param img Image to load
 */
void image_load_external_data(const image *img);

/**
 * Gets the real width and height of an external image
 * @param img Image to check
 * @param width The variable to set the width
 * @param height The variable to set the height
 * @return 1 if the dimensions could be set, 0 otherwise
 */
int image_get_external_dimensions(const image *img, int *width, int *height);

/**
 * Crops the transparent pixels around an image
 * @param img The image to crop
 * @param pixels The pixel data of the image
 */
void image_crop(image *img, const color_t *pixels);

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

/**
 * Copies an image
 * @param copy The copy information
 */
void image_copy(const image_copy_info *copy);

/**
 * Copies an isometric footprint
 * @param copy The copy information
 */
void image_copy_isometric_footprint(const image_copy_info *copy);

#endif // CORE_IMAGE_H

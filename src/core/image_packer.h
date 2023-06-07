#ifndef CORE_IMAGE_PACKER_H
#define CORE_IMAGE_PACKER_H

/**
 * This code is based on the japacker image packing library, however, unlike japacker (which is public domain),
 * this version of the japacker library is released under aGPL v3.0.
 * 
 * If you want to use the japacker library, please check its original source at https://github.com/crudelios/japacker.
 */

typedef enum {
    IMAGE_PACKER_STOP = 0,
    IMAGE_PACKER_CONTINUE = 1,
    IMAGE_PACKER_NEW_IMAGE = 2
} image_packer_fail_policy;

typedef enum {
    IMAGE_PACKER_SORT_BY_PERIMETER = 0,
    IMAGE_PACKER_SORT_BY_AREA = 1,
    IMAGE_PACKER_SORT_BY_HEIGHT = 2,
    IMAGE_PACKER_SORT_BY_WIDTH = 3
} image_packer_sort_type;

typedef enum {
    IMAGE_PACKER_OK = 0,
    IMAGE_PACKER_ERROR_WRONG_PARAMETERS = -1,
    IMAGE_PACKER_ERROR_NO_MEMORY = -2
} image_packer_error_type;

typedef struct {
    struct {
        unsigned int width;
        unsigned int height;
    } input;
    struct {
        unsigned int x;
        unsigned int y;
        int packed;
        int rotated;
        unsigned int image_index;
    } output;
} image_packer_rect;

typedef struct {
    image_packer_rect *rects;
    struct {
        int allow_rotation;
        int rects_are_sorted;
        int reduce_image_size;
        image_packer_sort_type sort_by;
        image_packer_fail_policy fail_policy;
    } options;
    struct {
        unsigned int images_needed;
        unsigned int last_image_width;
        unsigned int last_image_height;
    } result;
    void *internal_data;
} image_packer;

/**
 * @brief Initiates an image_packer object, allocating memory as needed.
 *
 * Don't use any other image_packer_* function on an image_packer without calling image_packer_init() first.
 *
 * @param packer The packer to init.
 * @param num_rectangles The total number of rectangles that need to be packed.
 * @param width The width of the destination rectangle.
 * @param height The height of the destination rectangle.
 * @return IMAGE_PACKER_OK on success, or another image_packer_error_type result on error.
 */
int image_packer_init(image_packer *packer, unsigned int num_rectangles, unsigned int width, unsigned int height);

/**
 * @brief Resizes the destination image. Note that this won't automatically repack any rect already packed.
 *
 * If you wish to repack the rects, make sure you set options.always_repack to 1 then call image_packer_pack() again.
 *
 * @param packer The packer whose destination image should be changed.
 * @param image_width The new width of the destination image.
 * @param image_height The new height of the destination image.
 */
void image_packer_resize_image(image_packer *packer, unsigned int image_width, unsigned int image_height);

/**
 * @brief Packs the rectangles into the destination image.
 *
 * The actual packing performed will depend on whathever options you've set beforehand.
 *
 * You can call image_packer_pack() multiple times.
 * If options.always_repack is set to 1, it will repack every rect again,regardless of whether it was packed or not.
 * This is useful if you want to try repacking everything with different packing options, for example.
 * If options.always_repack is kept at 0, only rects that weren't packed yet will be packed.
 * This is useful if you want to manually pack to many destination images.
 *
 * @param packer The image_packer struct to pack.
 * @return One of image_packer_error_type values on error, or the number of packed rects on success.
*/
int image_packer_pack(image_packer *packer);

/**
 * @brief Frees the memory associated with an image_packer object.
 * @param packer The object to free.
 */
void image_packer_free(image_packer *packer);

#endif // CORE_IMAGE_PACKER_H

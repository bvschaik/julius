#include "log.h"

#include "assets/assets.h"
#include "assets/group.h"
#include "assets/image.h"
#include "assets/layer.h"
#include "assets/xml.h"
#include "core/array.h"
#include "core/buffer.h"
#include "core/dir.h"
#include "core/file.h"
#include "core/image_packer.h"
#include "core/io.h"
#include "core/png_read.h"
#include "core/string.h"
#include "core/xml_exporter.h"
#include "graphics/color.h"
#include "platform/file_manager.h"

#include "spng/spng.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <errno.h>
#include <sys/stat.h>
#endif

#define ASSETS_IMAGE_SIZE 2048
#define CURSOR_IMAGE_SIZE 256
#define PACKED_ASSETS_DIR "packed_assets"
#define CURSORS_DIR "Cursors"
#define CURSORS_NAME "Color_Cursors"
#define BYTES_PER_PIXEL 4

#ifdef FORMAT_XML
#define FORMAT_NEWLINE "\n"
#define FORMAT_IDENT "    "
#else
#define FORMAT_NEWLINE ""
#define FORMAT_IDENT ""
#endif

static const char *LAYER_PART[] = { "footprint", "top" };
static const char *LAYER_ROTATE[] = { "90", "180", "270" };
static const char *LAYER_INVERT[] = { "horizontal", "vertical", "both" };
static const char *LAYER_MASK[] = { "grayscale", "alpha" };

static char current_file[FILE_NAME_MAX];

static color_t *final_image_pixels;
static unsigned int final_image_width;
static unsigned int final_image_height;

typedef struct {
    unsigned int id;
    const char *path;
    image_packer_rect *rect;
    color_t *pixels;
    image img;
} packed_asset;

#define PACKED_ASSETS_BLOCK_SIZE 256

static array(packed_asset) packed_assets;

const char *pref_user_dir(void)
{
    return "";
}

int random_from_stdlib(void)
{
    return 0;
}

static int remove_file(const char *filename, long unused)
{
    snprintf(current_file, FILE_NAME_MAX, "%s/%s", PACKED_ASSETS_DIR, filename);
    platform_file_manager_remove_file(current_file);
    return LIST_CONTINUE;
}

static int find_packed_assets_dir(const char *dir, long unused)
{
    return strcmp(dir, PACKED_ASSETS_DIR) == 0 ? LIST_MATCH : LIST_NO_MATCH;
}

static int prepare_packed_assets_dir(void)
{
    if (platform_file_manager_list_directory_contents(0, TYPE_DIR, 0, find_packed_assets_dir) == LIST_MATCH) {
        log_info("The packed assets dir exists, deleting its contents", 0, 0);
        if (!platform_file_manager_remove_directory(PACKED_ASSETS_DIR)) {
            log_error("There was a problem deleting the packed assets directory.", 0, 0);
            return 0;
        }
    }
    if (!platform_file_manager_create_directory(PACKED_ASSETS_DIR "/" ASSETS_IMAGE_PATH, 0, 1) ||
        !platform_file_manager_create_directory(PACKED_ASSETS_DIR "/" CURSORS_DIR, 0, 1)) {
        log_error("Failed to create directories", 0, 0);
        return 0;
    }
    return 1;
}

static void add_attribute_int(const char *name, int value)
{
    if (value != 0) {
        xml_exporter_add_attribute_int(name, value);
    }
}

static void add_attribute_bool(const char *name, int value, const char *expression_if_true)
{
    if (value != 0) {
        xml_exporter_add_attribute_text(name, expression_if_true);
    }
}

static void add_attribute_enum(const char *name, int value, const char **display_value, int max_values)
{
    if (value > 0 && value <= max_values) {
        xml_exporter_add_attribute_text(name, display_value[value - 1]);
    }
}

static void add_attribute_string(const char *name, const char *value)
{
    if (value && *value != 0) {
        xml_exporter_add_attribute_text(name, value);
    }
}

static void create_image_xml_attributes(const asset_image *image)
{
    add_attribute_string("id", image->id);
    if (image->has_defined_size) {
        add_attribute_int("width", image->img.width);
        add_attribute_int("height", image->img.height);
    }
    add_attribute_bool("isometric", image->img.is_isometric, "true");
}

static void create_layer_xml_line(const layer *l)
{
    if (!l->original_image_group && !l->original_image_id && !l->width && !l->height) {
        return;
    }
    xml_exporter_new_element("layer");

    add_attribute_string("group", l->original_image_group);
    add_attribute_string("image", l->original_image_id);
    add_attribute_int("src_x", l->src_x);
    add_attribute_int("src_y", l->src_y);
    add_attribute_int("x", l->x_offset);
    add_attribute_int("y", l->y_offset);
    add_attribute_int("width", l->width);
    add_attribute_int("height", l->height);
    add_attribute_enum("invert", l->invert, LAYER_INVERT, 3);
    add_attribute_enum("rotate", l->rotate, LAYER_ROTATE, 3);
    add_attribute_enum("part", l->part, LAYER_PART, 2);
    add_attribute_enum("mask", l->mask, LAYER_MASK, 2);

    xml_exporter_close_element();
}

static void create_animation_xml_attributes(const asset_image *image)
{

    if (!image->has_frame_elements) {
        add_attribute_int("frames", image->img.animation->num_sprites);
    }
    add_attribute_int("speed", image->img.animation->speed_id);
    add_attribute_int("x", image->img.animation->sprite_offset_x);
    add_attribute_int("y", image->img.animation->sprite_offset_y);
    add_attribute_bool("reversible", image->img.animation->can_reverse, "true");
}

static void create_frame_xml_line(const layer *l)
{
    xml_exporter_new_element("frame");

    add_attribute_string("group", l->original_image_group);
    add_attribute_string("image", l->original_image_id);
    add_attribute_int("src_x", l->src_x);
    add_attribute_int("src_y", l->src_y);
    add_attribute_int("x", l->x_offset);
    add_attribute_int("y", l->y_offset);
    add_attribute_int("width", l->width);
    add_attribute_int("height", l->height);
    add_attribute_enum("invert", l->invert, LAYER_INVERT, 3);
    add_attribute_enum("rotate", l->rotate, LAYER_ROTATE, 3);

    xml_exporter_close_element();
}

void new_packed_asset(packed_asset *asset, unsigned int index)
{
    asset->id = index;
}

int packed_asset_active(const packed_asset *asset)
{
    return asset->path != 0;
}

static packed_asset *get_asset_image_from_list(const layer *l)
{
    packed_asset *asset;
    array_foreach(packed_assets, asset) {
        if (asset->path && strcmp(l->asset_image_path, asset->path) == 0) {
            return asset;
        }
    }
    return 0;
}

static void add_asset_image_to_list(layer *l)
{
    packed_asset *asset = get_asset_image_from_list(l);
    if (!asset) {
        array_new_item_after_index(packed_assets, 1, asset);
        if (!asset) {
            log_error("Out of memory.", 0, 0);
            return;
        }
        asset->path = l->asset_image_path;
    }
    l->calculated_image_id = asset->id;
}

static void get_assets_for_group(int group_id)
{
    const image_groups *group = group_get_from_id(group_id);
    for (int image_id = group->first_image_index; image_id <= group->last_image_index; image_id++) {
        asset_image *image = asset_image_get_from_id(image_id);
        for (layer *l = &image->first_layer; l; l = l->next) {
            if (l->asset_image_path) {
                add_asset_image_to_list(l);
            }
        }
    }
}

void image_crop(image *img, const color_t *pixels)
{
    if (!img->width || !img->height) {
        return;
    }
    int x_first_opaque = 0;
    int x_last_opaque = 0;
    int y_first_opaque = 0;
    int y_last_opaque = 0;

    int found_opaque = 0;

    // Check all four sides of the rect
    for (int y = 0; y < img->height; y++) {
        const color_t *row = &pixels[y * img->width];
        for (int x = 0; x < img->width; x++) {
            if ((row[x] & COLOR_CHANNEL_ALPHA) != ALPHA_TRANSPARENT) {
                y_first_opaque = y;
                y_last_opaque = y;
                x_first_opaque = x;
                x_last_opaque = x;
                found_opaque = 1;
                break;
            }
        }
        if (found_opaque) {
            break;
        }
    }
    if (!found_opaque) {
        img->width = 0;
        img->height = 0;
        return;
    }
    found_opaque = 0;
    for (int y = img->height - 1; y > y_last_opaque; y--) {
        const color_t *row = &pixels[y * img->width];
        for (int x = img->width - 1; x >= 0; x--) {
            if ((row[x] & COLOR_CHANNEL_ALPHA) != ALPHA_TRANSPARENT) {
                y_last_opaque = y;
                if (x_first_opaque > x) {
                    x_first_opaque = x;
                }
                if (x_last_opaque < x) {
                    x_last_opaque = x;
                }
                found_opaque = 1;
                break;
            }
        }
        if (found_opaque) {
            break;
        }
    }
    found_opaque = 0;
    for (int x = 0; x < x_first_opaque; x++) {
        for (int y = y_first_opaque; y <= y_last_opaque; y++) {
            if ((pixels[y * img->width + x] & COLOR_CHANNEL_ALPHA) != ALPHA_TRANSPARENT) {
                x_first_opaque = x;
                found_opaque = 1;
                break;
            }
        }
        if (found_opaque) {
            break;
        }
    }
    found_opaque = 0;
    for (int x = img->width - 1; x > x_last_opaque; x--) {
        for (int y = y_first_opaque; y <= y_last_opaque; y++) {
            if ((pixels[y * img->width + x] & COLOR_CHANNEL_ALPHA) != ALPHA_TRANSPARENT) {
                x_last_opaque = x;
                found_opaque = 1;
                break;
            }
        }
        if (found_opaque) {
            break;
        }
    }

    img->x_offset = x_first_opaque;
    img->y_offset = y_first_opaque;
    img->width = x_last_opaque - x_first_opaque + 1;
    img->height = y_last_opaque - y_first_opaque + 1;
}

static void populate_asset_rects(image_packer *packer)
{
    packed_asset *asset;
    array_foreach(packed_assets, asset) {
        int width, height;
        if (!asset->path) {
            continue;
        }
        asset->rect = &packer->rects[asset->id];
        if (!png_load_from_file(asset->path, 1) || !png_get_image_size(&width, &height)) {
            continue;
        }
        if (!width || !height) {
            continue;
        }
        asset->pixels = malloc(sizeof(color_t) * width * height);
        if (!asset->pixels) {
            log_error("Out of memory.", 0, 0);
            continue;
        }
        if (!png_read(asset->pixels, 0, 0, width, height, 0, 0, width, 0)) {
            free(asset->pixels);
            asset->pixels = 0;
            continue;
        }
        asset->img.width = asset->img.original.width = width;
        asset->img.height = asset->img.original.height = height;
        image_crop(&asset->img, asset->pixels);
        asset->rect->input.width = asset->img.width;
        asset->rect->input.height = asset->img.height;
    }
}

static void copy_to_final_image(const color_t *pixels, const image_packer_rect *rect, int src_width)
{
    if (!rect->output.rotated) {
        for (unsigned int y = 0; y < rect->input.height; y++) {
            const color_t *src_pixel = &pixels[y * src_width];
            color_t *dst_pixel = &final_image_pixels[(y + rect->output.y) * final_image_width + rect->output.x];
            memcpy(dst_pixel, src_pixel, rect->input.width * sizeof(color_t));
        }
    } else {
        for (unsigned int y = 0; y < rect->input.height; y++) {
            const color_t *src_pixel = &pixels[y * src_width];
            color_t *dst_pixel = &final_image_pixels[(rect->output.y + rect->input.width - 1) *
                final_image_width + y + rect->output.x];
            for (unsigned int x = 0; x < rect->input.width; x++) {
                *dst_pixel = *src_pixel++;
                dst_pixel -= final_image_width;
            }
        }
    }
}

static void create_final_image(const image_packer *packer)
{
    packed_asset *asset;
    array_foreach(packed_assets, asset) {
        if (!asset->pixels) {
            continue;
        }
        color_t *dst = asset->pixels + asset->img.y_offset * asset->img.original.width + asset->img.x_offset;
        copy_to_final_image(dst, asset->rect, asset->img.original.width);
    }
}

static void save_final_image(const char *path, unsigned int width, unsigned int height, const color_t *pixels)
{
    spng_ctx *ctx = spng_ctx_new(SPNG_CTX_ENCODER);

    if (!ctx || spng_set_option(ctx, SPNG_IMG_COMPRESSION_LEVEL, 3)) {
        log_error("Error creating png structure for", path, 0);
        return;
    }
    FILE *fp = fopen(path, "wb");
    if (!fp || spng_set_png_file(ctx, fp)) {
        log_error("Error creating final png file at", path, 0);
        spng_ctx_free(ctx);
        fclose(fp);
        return;
    }
    struct spng_ihdr ihdr = {
        .width = width,
        .height = height,
        .bit_depth = 8,
        .color_type = SPNG_COLOR_TYPE_TRUECOLOR_ALPHA
    };
    if (spng_set_ihdr(ctx, &ihdr) ||
        spng_encode_image(ctx, 0, 0, SPNG_FMT_PNG, SPNG_ENCODE_PROGRESSIVE | SPNG_ENCODE_FINALIZE)) {
        log_error("Error creating final png file at", path, 0);
        spng_ctx_free(ctx);
        fclose(fp);
        return;
    }

    uint8_t *row_pixels = malloc(width * BYTES_PER_PIXEL);
    if (!row_pixels) {
        log_error("Out of memory for png creation", path, 0);
        fclose(fp);
        spng_ctx_free(ctx);
        return;
    }
    memset(row_pixels, 0, width * BYTES_PER_PIXEL);

    for (unsigned int y = 0; y < height; ++y) {
        uint8_t *pixel = row_pixels;
        for (unsigned int x = 0; x < width; x++) {
            color_t input = pixels[y * width + x];
            *(pixel + 0) = (uint8_t) COLOR_COMPONENT(input, COLOR_BITSHIFT_RED);
            *(pixel + 1) = (uint8_t) COLOR_COMPONENT(input, COLOR_BITSHIFT_GREEN);
            *(pixel + 2) = (uint8_t) COLOR_COMPONENT(input, COLOR_BITSHIFT_BLUE);
            *(pixel + 3) = (uint8_t) COLOR_COMPONENT(input, COLOR_BITSHIFT_ALPHA);
            pixel += BYTES_PER_PIXEL;
        }
        int result = spng_encode_scanline(ctx, row_pixels, width * BYTES_PER_PIXEL);
        if (result != SPNG_OK && result != SPNG_EOI) {
            log_error("Error constructing png file", path, 0);
            break;
        }
    }
    free(row_pixels);
    fclose(fp);
    spng_ctx_free(ctx);
}

static void pack_layer(const image_packer *packer, layer *l)
{
    if (!l->asset_image_path) {
        return;
    }
    const image_packer_rect *rect = &packer->rects[l->calculated_image_id];
    const packed_asset *asset = array_item(packed_assets, l->calculated_image_id);
    l->src_x = rect->output.x;
    l->src_y = rect->output.y;
    l->x_offset += asset->img.x_offset;
    l->y_offset += asset->img.y_offset;
    if (!rect->output.rotated) {
        l->width = rect->input.width;
        l->height = rect->input.height;
    }
    if (rect->output.rotated) {
        l->width = rect->input.height;
        l->height = rect->input.width;
        switch (l->rotate) {
            case ROTATE_90_DEGREES:
                l->rotate = ROTATE_180_DEGREES;
                break;
            case ROTATE_180_DEGREES:
                l->rotate = ROTATE_270_DEGREES;
                break;
            case ROTATE_270_DEGREES:
                l->rotate = ROTATE_NONE;
                break;
            case ROTATE_NONE:
            default:
                l->rotate = ROTATE_90_DEGREES;
                break;
        }
    }
}

static void calculate_image_size(asset_image *image)
{
    const layer *l = &image->first_layer;
    if (!l->calculated_image_id) {
        return;
    }
    const packed_asset *asset = array_item(packed_assets, l->calculated_image_id);
    if (asset->img.original.width != asset->img.width || asset->img.original.height != asset->img.height) {
        image->img.width = asset->img.original.width;
        image->img.height = asset->img.original.height;
        image->has_defined_size = 1;
    }
}

static void pack_group(int group_id)
{
    const image_groups *group = group_get_from_id(group_id);

    if (!group || !*group->name) {
        log_error("Could not retreive a valid group from id", 0, group_id);
        return;
    }

    array_init(packed_assets, PACKED_ASSETS_BLOCK_SIZE, new_packed_asset, packed_asset_active);

    get_assets_for_group(group_id);

    image_packer packer;
    image_packer_init(&packer, packed_assets.size, ASSETS_IMAGE_SIZE, ASSETS_IMAGE_SIZE);

    packer.options.allow_rotation = 1;
    packer.options.reduce_image_size = 1;

    log_info("Packing", group->name, 0);

    populate_asset_rects(&packer);

    if (image_packer_pack(&packer) != packed_assets.size) {
        log_error("Error during pack.", 0, 0);
        image_packer_free(&packer);
        return;
    }

    final_image_width = packer.result.last_image_width;
    final_image_height = packer.result.last_image_height;
    final_image_pixels = malloc(sizeof(color_t) * final_image_width * final_image_height);
    if (!final_image_pixels) {
        log_error("Out of memory when creating the final image.", 0, 0);
        image_packer_free(&packer);
        return;
    }
    memset(final_image_pixels, 0, sizeof(color_t) * final_image_width * final_image_height);

    create_final_image(&packer);

    printf("Info: %d Images packed. Texture size: %dx%d.\n", packed_assets.size,
        packer.result.last_image_width, packer.result.last_image_height);

    log_info("Creating xml file...", 0, 0);

    snprintf(current_file, FILE_NAME_MAX, "%s/%s/%s", PACKED_ASSETS_DIR, ASSETS_IMAGE_PATH, group->path);

    buffer buf;
    int buf_size = 5 * 1024 * 1024;
    uint8_t *buf_data = malloc(buf_size);
    buffer_init(&buf, buf_data, buf_size);
    xml_exporter_init(&buf, "assetlist");
    xml_exporter_add_text("<!-- XML auto generated by asset_packer. DO NOT use as a reference.");
    xml_exporter_newline();
    xml_exporter_add_text("     Use the assets directory from the source code instead. -->");
    xml_exporter_newline();
    xml_exporter_newline();
    xml_exporter_new_element("assetlist");
    xml_exporter_add_attribute_text("name", group->name);

    for (int image_id = group->first_image_index; image_id <= group->last_image_index; image_id++) {
        asset_image *image = asset_image_get_from_id(image_id);
        if (!image->has_defined_size) {
            calculate_image_size(image);
        }
        xml_exporter_new_element("image");

        create_image_xml_attributes(image);
        for (layer *l = &image->first_layer; l; l = l->next) {
            pack_layer(&packer, l);
            create_layer_xml_line(l);
        }
        if (image->img.animation) {
            xml_exporter_new_element("animation");
            create_animation_xml_attributes(image);
            if (image->has_frame_elements) {
                for (int i = 0; i < image->img.animation->num_sprites; i++) {
                    image_id++;
                    asset_image *frame = asset_image_get_from_id(image_id);
                    layer *l = frame->last_layer;
                    pack_layer(&packer, l);
                    create_frame_xml_line(l);
                }
            }
            xml_exporter_close_element();
        }
        xml_exporter_close_element();
    }

    xml_exporter_close_element();

    packed_asset *asset;
    array_foreach(packed_assets, asset) {
        free(asset->pixels);
    }

    FILE *xml_dest = fopen(current_file, "wb");

    if (!xml_dest) {
        log_error("Failed to create file", group->path, 0);
        return;
    }

    fwrite(buf.data, 1, buf.index, xml_dest);

    fclose(xml_dest);
    image_packer_free(&packer);
    free(buf_data);

    snprintf(current_file, FILE_NAME_MAX, "%s/%s/%s.png", PACKED_ASSETS_DIR, ASSETS_IMAGE_PATH, group->name);

    log_info("Creating png file...", 0, 0);

    save_final_image(current_file, final_image_width, final_image_height, final_image_pixels);

    free(final_image_pixels);
}

static void pack_cursors(void)
{
    static const char *cursor_names[] = { "Arrow", "Shovel", "Sword" };
    static const char *cursor_sizes[] = { "150", "200" };

#define NUM_CURSOR_NAMES (sizeof(cursor_names) / sizeof(cursor_names[0]))
#define NUM_CURSOR_SIZES (sizeof(cursor_sizes) / sizeof(cursor_sizes[0]) + 1)

    static layer cursors[NUM_CURSOR_NAMES * NUM_CURSOR_SIZES];

    image_packer packer;
    image_packer_init(&packer, NUM_CURSOR_NAMES * NUM_CURSOR_SIZES, CURSOR_IMAGE_SIZE, CURSOR_IMAGE_SIZE);

    packer.options.allow_rotation = 1;
    packer.options.reduce_image_size = 1;
    packer.options.sort_by = IMAGE_PACKER_SORT_BY_AREA;

    for (int i = 0; i < NUM_CURSOR_NAMES; i++) {
        for (int j = 0; j < NUM_CURSOR_SIZES; j++) {
            int index = i * NUM_CURSOR_SIZES + j;
            layer *cursor = &cursors[index];
            cursor->calculated_image_id = index;
            cursor->asset_image_path = malloc(FILE_NAME_MAX);
            if (!cursor->asset_image_path) {
                log_error("Out of memory.", 0, 0);
                image_packer_free(&packer);
                return;
            }
            if (j > 0) {
                snprintf(cursor->asset_image_path, FILE_NAME_MAX, "%s/%s/%s_%s.png", CURSORS_DIR, CURSORS_NAME,
                    cursor_names[i], cursor_sizes[j - 1]);
            } else {
                snprintf(cursor->asset_image_path, FILE_NAME_MAX, "%s/%s/%s.png", CURSORS_DIR, CURSORS_NAME,
                    cursor_names[i]);
            }
            if (!png_load_from_file(cursor->asset_image_path, 1) ||
                !png_get_image_size(&cursor->width, &cursor->height)) {
                image_packer_free(&packer);
                return;
            }
            color_t *data = malloc(cursor->width * cursor->height * sizeof(color_t));
            if (!data) {
                log_error("Out of memory.", 0, 0);
                image_packer_free(&packer);
                return;
            }
            png_read(data, 0, 0, cursor->width, cursor->height, 0, 0, cursor->width, 0);
            packer.rects[index].input.width = cursor->width;
            packer.rects[index].input.height = cursor->height;
            cursor->data = data;
        }
    }

    image_packer_pack(&packer);

    final_image_width = packer.result.last_image_width;
    final_image_height = packer.result.last_image_height;
    final_image_pixels = malloc(sizeof(color_t) * final_image_width * final_image_height);
    if (!final_image_pixels) {
        log_error("Out of memory when creating the final cursor image.", 0, 0);
        image_packer_free(&packer);
        return;
    }
    memset(final_image_pixels, 0, sizeof(color_t) * final_image_width * final_image_height);

    log_info("Cursor positions and sizes in packed image:", 0, 0);

    printf("   Name             x       y      width      height\n");

    for (int i = 0; i < NUM_CURSOR_NAMES * NUM_CURSOR_SIZES; i++) {
        layer *cursor = &cursors[i];
        pack_layer(&packer, cursor);
        copy_to_final_image(cursor->data, &packer.rects[i], cursor->width);
        printf("%-16s  %3d     %3d        %3d         %3d\n",
            cursor->asset_image_path + strlen(CURSORS_DIR) + strlen(CURSORS_NAME) + 2,
            packer.rects[i].output.x, packer.rects[i].output.y, cursor->width, cursor->height);
    }

    snprintf(current_file, FILE_NAME_MAX, "%s/%s/%s.png", PACKED_ASSETS_DIR, CURSORS_DIR, CURSORS_NAME);

    save_final_image(current_file, final_image_width, final_image_height, final_image_pixels);

    free(final_image_pixels);

    image_packer_free(&packer);
}

int main(int argc, char **argv)
{
    int using_custom_path = 0;
    if (argc == 2) {
        log_info("Attempting to use the path", argv[1], 0);
        if (!platform_file_manager_set_base_path(argv[1])) {
            log_info("Unable to change the base path. Attempting to run from local directory...", 0, 0);
        } else {
            using_custom_path = 1;
        }
    }
    const dir_listing *xml_files = dir_find_files_with_extension(ASSETS_DIRECTORY "/" ASSETS_IMAGE_PATH, "xml");
    if (xml_files->num_files == 0) {
        if (using_custom_path) {
            log_error("No assets found on", argv[1], 0);
        }
        log_error("Please add a valid assets folder to this directory.\n"
            "Alternatively, you can run as:\n\n"
            "asset_packer.exe [WORK_DIRECTORY]\n\n"
            "where WORK_DIRECTORY is the directory where the assets folder is in.", 0, 0);
        return 1;
    }

    if (!prepare_packed_assets_dir()) {
        return 2;
    }

#ifdef PACK_XMLS
    if (!group_create_all(xml_files->num_files) || !asset_image_init_array()) {
        log_error("Not enough memory to initialize extra assets.", 0, 0);
        return 3;
    }

    xml_init();

    for (int i = 0; i < xml_files->num_files; ++i) {
        xml_process_assetlist_file(xml_files->files[i].name);
    }

    xml_finish();

    log_info("Preparing to pack...", 0, 0);

    for (int i = 0; i < group_get_total(); i++) {
        pack_group(i);
    }

#endif

#ifdef PACK_CURSORS

    log_info("Packing cursors...", 0, 0);

    pack_cursors();

#endif

    log_info("Copying other assets...", 0, 0);

    platform_file_manager_copy_directory(ASSETS_DIRECTORY, PACKED_ASSETS_DIR, 1);

    log_info("All done!", 0, 0);

    png_unload();
    return 0;
}

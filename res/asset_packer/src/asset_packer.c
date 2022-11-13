#include "log.h"

#include "assets/assets.h"
#include "assets/group.h"
#include "assets/image.h"
#include "assets/layer.h"
#include "assets/xml.h"
#include "core/array.h"
#include "core/dir.h"
#include "core/file.h"
#include "core/image_packer.h"
#include "core/png_read.h"
#include "graphics/color.h"
#include "platform/file_manager.h"

#include "png.h"

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
#define CURSORS_DIR "Color_Cursors"
#define BYTES_PER_PIXEL 4
#define FILE_NAME_MAX 300

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
    int id;
    const char *path;
    image_packer_rect *rect;
    color_t *pixels;
} packed_asset;

#define PACKED_ASSETS_BLOCK_SIZE 256

static array(packed_asset) packed_assets;

static int remove_file(const char *filename)
{
    snprintf(current_file, FILE_NAME_MAX, "%s/%s", PACKED_ASSETS_DIR, filename);
    platform_file_manager_remove_file(current_file);
    return LIST_CONTINUE;
}

static int find_packed_assets_dir(const char *dir)
{
    return strcmp(dir, PACKED_ASSETS_DIR) == 0 ? LIST_MATCH : LIST_NO_MATCH;
}

static int prepare_packed_assets_dir(void)
{
    if (platform_file_manager_list_directory_contents(0, TYPE_DIR, 0, find_packed_assets_dir) == LIST_MATCH) {
        log_info("The packed assets dir exists, deleting its contents", 0, 0);
        platform_file_manager_list_directory_contents(PACKED_ASSETS_DIR, TYPE_FILE, 0, remove_file);
    } else if (!platform_file_manager_create_directory(PACKED_ASSETS_DIR)) {
        log_error("Failed to create directory", PACKED_ASSETS_DIR, 0);
        return 0;
    }
    return 1;
}

static void add_attribute_int(FILE *dest, const char *name, int value)
{
    if (value != 0) {
        fprintf(dest, " %s=\"%d\"", name, value);
    }
}

static void add_attribute_bool(FILE *dest, const char *name, int value, const char *expression_if_true)
{
    if (value != 0) {
        fprintf(dest, " %s=\"%s\"", name, expression_if_true);
    }
}

static void add_attribute_enum(FILE *dest, const char *name, int value, const char **display_value, int max_values)
{
    if (value > 0 && value <= max_values) {
        fprintf(dest, " %s=\"%s\"", name, display_value[value - 1]);
    }
}

static void add_attribute_string(FILE *dest, const char *name, const char *value)
{
    if (value && *value != 0) {
        fprintf(dest, " %s=\"%s\"", name, value);
    }
}

static void create_image_xml_line(FILE *xml_file, const asset_image *image)
{
    fprintf(xml_file, "%s<image", FORMAT_IDENT);

    add_attribute_string(xml_file, "id", image->id);
    if (image->has_defined_size) {
        add_attribute_int(xml_file, "width", image->img.width);
        add_attribute_int(xml_file, "height", image->img.height);
    }
    add_attribute_bool(xml_file, "isometric", image->img.is_isometric, "true");
    fprintf(xml_file, ">%s", FORMAT_NEWLINE);
}

static void create_layer_xml_line(FILE *xml_file, const layer *l)
{
    fprintf(xml_file, "%s%s<layer", FORMAT_IDENT, FORMAT_IDENT);

    add_attribute_string(xml_file, "group", l->original_image_group);
    add_attribute_string(xml_file, "image", l->original_image_id);
    add_attribute_int(xml_file, "src_x", l->src_x);
    add_attribute_int(xml_file, "src_y", l->src_y);
    add_attribute_int(xml_file, "x", l->x_offset);
    add_attribute_int(xml_file, "y", l->y_offset);
    add_attribute_int(xml_file, "width", l->width);
    add_attribute_int(xml_file, "height", l->height);
    add_attribute_enum(xml_file, "invert", l->invert, LAYER_INVERT, 3);
    add_attribute_enum(xml_file, "rotate", l->rotate, LAYER_ROTATE, 3);
    add_attribute_enum(xml_file, "part", l->part, LAYER_PART, 2);
    add_attribute_enum(xml_file, "mask", l->mask, LAYER_MASK, 2);

    fprintf(xml_file, "/>%s", FORMAT_NEWLINE);
}

static void create_animation_xml_line(FILE *xml_file, const asset_image *image)
{
    fprintf(xml_file, "%s%s<animation", FORMAT_IDENT, FORMAT_IDENT);

    if (!image->has_frame_elements) {
        add_attribute_int(xml_file, "frames", image->img.animation->num_sprites);
    }
    add_attribute_int(xml_file, "speed", image->img.animation->speed_id);
    add_attribute_int(xml_file, "x", image->img.animation->sprite_offset_x);
    add_attribute_int(xml_file, "y", image->img.animation->sprite_offset_y);
    add_attribute_bool(xml_file, "reversible", image->img.animation->can_reverse, "true");

    fprintf(xml_file, "%s>%s", image->has_frame_elements ? "" : "/", FORMAT_NEWLINE);
}

static void create_frame_xml_line(FILE *xml_file, const layer *l)
{
    fprintf(xml_file, "%s%s%s<frame", FORMAT_IDENT, FORMAT_IDENT, FORMAT_IDENT);

    add_attribute_string(xml_file, "group", l->original_image_group);
    add_attribute_string(xml_file, "image", l->original_image_id);
    add_attribute_int(xml_file, "src_x", l->src_x);
    add_attribute_int(xml_file, "src_y", l->src_y);
    add_attribute_int(xml_file, "width", l->width);
    add_attribute_int(xml_file, "height", l->height);
    add_attribute_enum(xml_file, "invert", l->invert, LAYER_INVERT, 3);
    add_attribute_enum(xml_file, "rotate", l->rotate, LAYER_ROTATE, 3);

    fprintf(xml_file, "/>%s", FORMAT_NEWLINE);
}

void new_packed_asset(packed_asset *asset, int index)
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
        if (strcmp(l->asset_image_path, asset->path) == 0) {
            return asset;
        }
    }
    return 0;
}

static void add_asset_image_to_list(layer *l)
{
    packed_asset *asset = get_asset_image_from_list(l);
    if (!asset) {
        array_new_item(packed_assets, 0, asset);
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

static void populate_asset_rects(image_packer *packer)
{
    packed_asset *asset;
    array_foreach(packed_assets, asset)
    {
        int width, height;
        asset->rect = &packer->rects[asset->id];
        if (!png_get_image_size(asset->path, &width, &height)) {
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
        if (!png_read(asset->path, asset->pixels, 0, 0, width, height, 0, 0, width, 0)) {
            free(asset->pixels);
            asset->pixels = 0;
            continue;
        }
        asset->rect->input.width = width;
        asset->rect->input.height = height;
    }
}

static void copy_to_final_image(const color_t *pixels, const image_packer_rect *rect)
{
    if (!rect->output.rotated) {
        for (unsigned int y = 0; y < rect->input.height; y++) {
            const color_t *src_pixel = &pixels[y * rect->input.width];
            color_t *dst_pixel = &final_image_pixels[(y + rect->output.y) * final_image_width + rect->output.x];
            memcpy(dst_pixel, src_pixel, rect->input.width * sizeof(color_t));
        }
    } else {
        for (unsigned int y = 0; y < rect->input.height; y++) {
            const color_t *src_pixel = &pixels[y * rect->input.width];
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
        copy_to_final_image(asset->pixels, asset->rect);
    }
}

static void save_final_image(const char *path, unsigned int width, unsigned int height, const color_t *pixels)
{
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

    if (!png_ptr) {
        log_error("Error creating png structure for", path, 0);
        return;
    }
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        log_error("Error creating png structure for", path, 0);
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return;
    }
    png_set_compression_level(png_ptr, 3);

    FILE *fp = fopen(path, "wb");
    if (!fp) {
        log_error("Error creating final png file at", path, 0);
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return;
    }
    png_init_io(png_ptr, fp);

    if (setjmp(png_jmpbuf(png_ptr))) {
        log_error("Error constructing png file", path, 0);
        fclose(fp);
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return;
    }
    png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGBA,
        PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png_ptr, info_ptr);

    uint8_t *row_pixels = malloc(width * BYTES_PER_PIXEL);
    if (!row_pixels) {
        log_error("Out of memory for png creation", path, 0);
        fclose(fp);
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return;
    }
    memset(row_pixels, 0, width * BYTES_PER_PIXEL);

    if (setjmp(png_jmpbuf(png_ptr))) {
        log_error("Error constructing png file", path, 0);
        free(row_pixels);
        fclose(fp);
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return;
    }
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
        png_write_row(png_ptr, row_pixels);
    }
    png_write_end(png_ptr, info_ptr);

    free(row_pixels);
    fclose(fp);
    png_destroy_write_struct(&png_ptr, &info_ptr);
}

static void pack_layer(const image_packer *packer, layer *l)
{
    if (!l->asset_image_path) {
        return;
    }
    image_packer_rect *rect = &packer->rects[l->calculated_image_id];
    l->src_x = rect->output.x;
    l->src_y = rect->output.y;
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

    static char current_dir[FILE_NAME_MAX];

    snprintf(current_dir, FILE_NAME_MAX, "%s/%s/", PACKED_ASSETS_DIR, group->name);
    snprintf(current_file, FILE_NAME_MAX, "%s/%s", PACKED_ASSETS_DIR, group->path);

    FILE *xml_dest = fopen(current_file, "wb");

    if (!xml_dest) {
        log_error("Failed to create file", group->path, 0);
        return;
    }

    fprintf(xml_dest, "<?xml version=\"1.0\"?>\n");
    fprintf(xml_dest, "<!DOCTYPE assetlist>\n\n");

    fprintf(xml_dest, "<!-- XML auto packed by asset_packer. DO NOT use as a reference.\n");
    fprintf(xml_dest, "     Use the assets directory from the source code instead. -->\n\n");

    fprintf(xml_dest, "<assetlist name=\"%s\">%s", group->name, FORMAT_NEWLINE);

    for (int image_id = group->first_image_index; image_id <= group->last_image_index; image_id++) {
        asset_image *image = asset_image_get_from_id(image_id);
        create_image_xml_line(xml_dest, image);
        for (layer *l = &image->first_layer; l; l = l->next) {
            pack_layer(&packer, l);
            create_layer_xml_line(xml_dest, l);
        }
        if (image->img.animation) {
            create_animation_xml_line(xml_dest, image);
            if (image->has_frame_elements) {
                for (int i = 0; i < image->img.animation->num_sprites; i++) {
                    image_id++;
                    asset_image *frame = asset_image_get_from_id(image_id);
                    layer *l = frame->last_layer;
                    pack_layer(&packer, l);
                    create_frame_xml_line(xml_dest, l);
                }
                fprintf(xml_dest, "%s%s</animation>%s", FORMAT_IDENT, FORMAT_IDENT, FORMAT_NEWLINE);
            }
        }
        fprintf(xml_dest, "%s</image>%s", FORMAT_IDENT, FORMAT_NEWLINE);
    }

    fprintf(xml_dest, "</assetlist>\n");

    packed_asset *asset;
    array_foreach(packed_assets, asset) {
        free(asset->pixels);
    }

    fclose(xml_dest);
    image_packer_free(&packer);

    snprintf(current_file, FILE_NAME_MAX, "%s/%s.png", PACKED_ASSETS_DIR, group->name);

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
                snprintf(cursor->asset_image_path, FILE_NAME_MAX, "%s/%s_%s.png", CURSORS_DIR,
                    cursor_names[i], cursor_sizes[j - 1]);
            } else {
                snprintf(cursor->asset_image_path, FILE_NAME_MAX, "%s/%s.png", CURSORS_DIR, cursor_names[i]);
            }
            if (!png_get_image_size(cursor->asset_image_path, &cursor->width, &cursor->height)) {
                image_packer_free(&packer);
                return;
            }
            color_t *data = malloc(cursor->width * cursor->height * sizeof(color_t));
            if (!data) {
                log_error("Out of memory.", 0, 0);
                image_packer_free(&packer);
                return;
            }
            png_read(cursor->asset_image_path, data, 0, 0,
                cursor->width, cursor->height, 0, 0, cursor->width, 0);
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
        copy_to_final_image(cursor->data, &packer.rects[i]);
        printf("%-16s  %3d     %3d        %3d         %3d\n",
            cursor->asset_image_path + strlen(CURSORS_DIR) + 1,
            packer.rects[i].output.x, packer.rects[i].output.y, cursor->width, cursor->height);
    }

    snprintf(current_file, FILE_NAME_MAX, "%s/%s.png", PACKED_ASSETS_DIR, CURSORS_DIR);

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
    const dir_listing *xml_files = dir_find_files_with_extension(ASSETS_DIRECTORY, "xml");
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
        xml_process_assetlist_file(xml_files->files[i]);
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

    log_info("All done!", 0, 0);

    png_unload();
    return 0;
}

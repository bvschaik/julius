#include "xml.h"

#include "assets/group.h"
#include "assets/image.h"
#include "core/calc.h"
#include "core/dir.h"
#include "core/file.h"
#include "core/log.h"
#include "core/png_read.h"
#include "core/string.h"
#include "core/xml_parser.h"
#include "graphics/renderer.h"

#include <string.h>

#define XML_BUFFER_SIZE 1024
#define XML_TOTAL_ELEMENTS 5

static int xml_start_assetlist_element(const xml_parser_element *element, const char **attributes, int total_attributes);
static int xml_start_image_element(const xml_parser_element *element, const char **attributes, int total_attributes);
static int xml_start_layer_element(const xml_parser_element *element, const char **attributes, int total_attributes);
static int xml_start_animation_element(const xml_parser_element *element, const char **attributes, int total_attributes);
static int xml_start_frame_element(const xml_parser_element *element, const char **attributes, int total_attributes);
static void xml_end_assetlist_element(void);
static void xml_end_image_element(void);
static void xml_end_animation_element(void);

static struct {
    char file_name[FILE_NAME_MAX];
    size_t file_name_position;
    int finished;
    int in_animation;
    image_groups *current_group;
    asset_image *current_image;
} data;

static const xml_parser_element xml_elements[XML_TOTAL_ELEMENTS] = {
    { "assetlist", { "name"}, xml_start_assetlist_element, xml_end_assetlist_element },
    { "image", { "id", "src", "width", "height", "group", "image", "isometric" }, xml_start_image_element, xml_end_image_element, "assetlist" },
    { "layer", { "src", "group", "image", "src_x", "src_y", "x", "y", "width", "height", "invert", "rotate", "part", "mask" }, xml_start_layer_element, 0, "image" },
    { "animation", { "frames", "speed", "reversible", "x", "y" }, xml_start_animation_element, xml_end_animation_element, "image" },
    { "frame", { "src", "src_x", "src_y", "width", "height", "group", "image", "invert", "rotate" }, xml_start_frame_element, 0, "animation"}
};

static void set_asset_image_base_path(const char *name)
{
    size_t position = 0;
    char *dst = data.file_name;
    memset(dst, 0, FILE_NAME_MAX);
    strncpy(dst, name, FILE_NAME_MAX - position - 1);
    position += strlen(name);
    dst[position++] = '/';
    data.file_name_position = position;
}

static int xml_start_assetlist_element(const xml_parser_element *element, const char **attributes, int total_attributes)
{
    data.current_group = group_get_new();
    if (total_attributes != 2) {
        return 0;
    }
    char *name = 0;
    if (strcmp(attributes[0], element->attributes[0]) == 0) {
        size_t name_length = strlen(attributes[1]);
        name = malloc(sizeof(char) * (name_length + 1));
        if (name) {
            strcpy(name, attributes[1]);
            data.current_group->name = name;
        }
    }
    if (!name || *name == '\0') {
        free(name);
        return 0;
    }
    data.current_image = 0;
    set_asset_image_base_path(data.current_group->name);
    return 1;
}

static int xml_start_image_element(const xml_parser_element *element, const char **attributes, int total_attributes)
{
    if (total_attributes > 12 || total_attributes % 2) {
        return 0;
    }
    asset_image *img = asset_image_create();
    if (!img) {
        return 0;
    }
    if (!data.current_image) {
        data.current_group->first_image_index = img->index;
    }
    data.current_group->last_image_index = img->index;
    data.current_image = img;
    const char *path = 0;
    const char *group = 0;
    const char *id = 0;
    for (int i = 0; i < total_attributes; i += 2) {
        if (strcmp(attributes[i], element->attributes[0]) == 0) {
            size_t img_id_length = strlen(attributes[i + 1]);
            char *img_id = malloc(sizeof(char) * (img_id_length + 1));
            if (img_id) {
                strcpy(img_id, attributes[i + 1]);
                img->id = img_id;
            }
        } else if (strcmp(attributes[i], element->attributes[1]) == 0) {
            path = attributes[i + 1];
        } else if (strcmp(attributes[i], element->attributes[2]) == 0) {
            img->img.width = string_to_int(string_from_ascii(attributes[i + 1]));
#ifdef BUILDING_ASSET_PACKER
            img->has_defined_size = 1;
#endif
        } else if (strcmp(attributes[i], element->attributes[3]) == 0) {
            img->img.height = string_to_int(string_from_ascii(attributes[i + 1]));
#ifdef BUILDING_ASSET_PACKER
            img->has_defined_size = 1;
#endif
        } else if (strcmp(attributes[i], element->attributes[4]) == 0) {
            group = attributes[i + 1];
        } else if (strcmp(attributes[i], element->attributes[5]) == 0) {
            id = attributes[i + 1];
        } else if (strcmp(attributes[i], element->attributes[6]) == 0) {
            const char *value = attributes[i + 1];
            if (strcmp(value, "true") == 0 || strcmp(value, "1") == 0 || strcmp(value, "isometric") == 0 ||
                strcmp(value, "yes") == 0 || strcmp(value, "y") == 0) {
                img->img.is_isometric = 1;
                asset_image_count_isometric();
            }
        }
    }
    img->last_layer = &img->first_layer;
    if (path || group) {
        asset_image_add_layer(img, path, group, id, 0, 0, 0, 0, 0, 0, INVERT_NONE, ROTATE_NONE, PART_BOTH, 0);
    }
    return 1;
}

static int xml_start_layer_element(const xml_parser_element *element, const char **attributes, int total_attributes)
{
    const char *path = 0;
    const char *group = 0;
    const char *id = 0;
    int src_x = 0;
    int src_y = 0;
    int offset_x = 0;
    int offset_y = 0;
    int width = 0;
    int height = 0;
    asset_image *img = data.current_image;
    if (total_attributes < 2 || total_attributes > 24 || total_attributes % 2) {
        return 0;
    }
    layer_invert_type invert = INVERT_NONE;
    layer_rotate_type rotate = ROTATE_NONE;
    layer_isometric_part part = PART_BOTH;
    layer_mask mask = LAYER_MASK_NONE;
    for (int i = 0; i < total_attributes; i += 2) {
        if (strcmp(attributes[i], element->attributes[0]) == 0) {
            path = attributes[i + 1];
        } else if (strcmp(attributes[i], element->attributes[1]) == 0) {
            group = attributes[i + 1];
        } else if (strcmp(attributes[i], element->attributes[2]) == 0) {
            id = attributes[i + 1];
        } else if (strcmp(attributes[i], element->attributes[3]) == 0) {
            src_x = string_to_int(string_from_ascii(attributes[i + 1]));
        } else if (strcmp(attributes[i], element->attributes[4]) == 0) {
            src_y = string_to_int(string_from_ascii(attributes[i + 1]));
        } else if (strcmp(attributes[i], element->attributes[5]) == 0) {
            offset_x = string_to_int(string_from_ascii(attributes[i + 1]));
        } else if (strcmp(attributes[i], element->attributes[6]) == 0) {
            offset_y = string_to_int(string_from_ascii(attributes[i + 1]));
        } else if (strcmp(attributes[i], element->attributes[7]) == 0) {
            width = string_to_int(string_from_ascii(attributes[i + 1]));
        } else if (strcmp(attributes[i], element->attributes[8]) == 0) {
            height = string_to_int(string_from_ascii(attributes[i + 1]));
        } else if (strcmp(attributes[i], element->attributes[9]) == 0) {
            if (strcmp(attributes[i + 1], "horizontal") == 0) {
                invert = INVERT_HORIZONTAL;
            } else if (strcmp(attributes[i + 1], "vertical") == 0) {
                invert = INVERT_VERTICAL;
            } else if (strcmp(attributes[i + 1], "both") == 0) {
                invert = INVERT_BOTH;
            }
        } else if (strcmp(attributes[i], element->attributes[10]) == 0) {
            if (strcmp(attributes[i + 1], "90") == 0) {
                rotate = ROTATE_90_DEGREES;
            } else if (strcmp(attributes[i + 1], "180") == 0) {
                rotate = ROTATE_180_DEGREES;
            } else if (strcmp(attributes[i + 1], "270") == 0) {
                rotate = ROTATE_270_DEGREES;
            }
        } else if (strcmp(attributes[i], element->attributes[11]) == 0) {
            if (strcmp(attributes[i + 1], "footprint") == 0) {
                part = PART_FOOTPRINT;
            } else if (strcmp(attributes[i + 1], "top") == 0) {
                part = PART_TOP;
            }
        } else if (strcmp(attributes[i], element->attributes[12]) == 0) {
            if (strcmp(attributes[i + 1], "grayscale") == 0) {
                mask = LAYER_MASK_GRAYSCALE;
            } else if (strcmp(attributes[i + 1], "alpha") == 0) {
                mask = LAYER_MASK_ALPHA;
            }
        }
    }
    if (!asset_image_add_layer(img, path, group, id, src_x, src_y,
        offset_x, offset_y, width, height, invert, rotate, part, mask)) {
        log_info("Invalid layer for image", img->id, 0);
    }
    return 1;
}

static int xml_start_animation_element(const xml_parser_element *element, const char **attributes, int total_attributes)
{
    asset_image *img = data.current_image;
    if (img->img.animation) {
        return 1;
    }
    if (total_attributes % 2) {
        return 0;
    }
    img->img.animation = malloc(sizeof(image_animation));
    if (!img->img.animation) {
        return 0;
    }
    memset(img->img.animation, 0, sizeof(image_animation));

    for (int i = 0; i < total_attributes; i += 2) {
        if (strcmp(attributes[i], element->attributes[0]) == 0) {
            img->img.animation->num_sprites = string_to_int(string_from_ascii(attributes[i + 1]));
        } else if (strcmp(attributes[i], element->attributes[1]) == 0) {
            img->img.animation->speed_id = calc_bound(string_to_int(string_from_ascii(attributes[i + 1])), 0, 50);
        } else if (strcmp(attributes[i], element->attributes[2]) == 0) {
            const char *value = attributes[i + 1];
            if (strcmp(value, "true") == 0 || strcmp(value, "1") == 0 || strcmp(value, "reversible") == 0 ||
                strcmp(value, "yes") == 0 || strcmp(value, "y") == 0) {
                img->img.animation->can_reverse = 1;
            }
        } else if (strcmp(attributes[i], element->attributes[3]) == 0) {
            img->img.animation->sprite_offset_x = string_to_int(string_from_ascii(attributes[i + 1]));
        } else if (strcmp(attributes[i], element->attributes[4]) == 0) {
            img->img.animation->sprite_offset_y = string_to_int(string_from_ascii(attributes[i + 1]));
        }
    }
    if (!img->img.animation->num_sprites) {
        data.in_animation = 1;
    }
    return 1;
}

static int xml_start_frame_element(const xml_parser_element *element, const char **attributes, int total_attributes)
{
    if (!data.in_animation || total_attributes < 2 || total_attributes % 2) {
        return 1;
    }
    asset_image *img = asset_image_create();
    if (!img) {
        return 0;
    }
    const char *path = 0;
    const char *group = 0;
    const char *id = 0;
    int src_x = 0;
    int src_y = 0;
    int width = 0;
    int height = 0;
    layer_invert_type invert = INVERT_NONE;
    layer_rotate_type rotate = ROTATE_NONE;
    for (int i = 0; i < total_attributes; i += 2) {
        if (strcmp(attributes[i], element->attributes[0]) == 0) {
            path = attributes[i + 1];
        } else if (strcmp(attributes[i], element->attributes[1]) == 0) {
            src_x = string_to_int(string_from_ascii(attributes[i + 1]));
        } else if (strcmp(attributes[i], element->attributes[2]) == 0) {
            src_y = string_to_int(string_from_ascii(attributes[i + 1]));
        } else if (strcmp(attributes[i], element->attributes[3]) == 0) {
            width = string_to_int(string_from_ascii(attributes[i + 1]));
        } else if (strcmp(attributes[i], element->attributes[4]) == 0) {
            height = string_to_int(string_from_ascii(attributes[i + 1]));
        } else if (strcmp(attributes[i], element->attributes[5]) == 0) {
            group = attributes[i + 1];
        } else if (strcmp(attributes[i], element->attributes[6]) == 0) {
            id = attributes[i + 1];
        } else if (strcmp(attributes[i], element->attributes[7]) == 0) {
            if (strcmp(attributes[i + 1], "horizontal") == 0) {
                invert = INVERT_HORIZONTAL;
            } else if (strcmp(attributes[i + 1], "vertical") == 0) {
                invert = INVERT_VERTICAL;
            } else if (strcmp(attributes[i + 1], "both") == 0) {
                invert = INVERT_BOTH;
            }
        } else if (strcmp(attributes[i], element->attributes[8]) == 0) {
            if (strcmp(attributes[i + 1], "90") == 0) {
                rotate = ROTATE_90_DEGREES;
            } else if (strcmp(attributes[i + 1], "180") == 0) {
                rotate = ROTATE_180_DEGREES;
            } else if (strcmp(attributes[i + 1], "270") == 0) {
                rotate = ROTATE_270_DEGREES;
            }
        }
    }
    img->last_layer = &img->first_layer;
    if (!asset_image_add_layer(img, path, group, id, src_x, src_y,
        0, 0, width, height, invert, rotate, PART_BOTH, 0)) {
        img->active = 0;
        return 1;
    }
#ifndef BUILDING_ASSET_PACKER
    if (!img->img.width || !img->img.height) {
        asset_image_unload(img);
        return 1;
    }
    asset_image_check_and_handle_reference(img);
#else
    data.current_image->has_frame_elements = 1;
#endif
    data.current_group->last_image_index = img->index;
    data.current_image->img.animation->num_sprites++;

    return 1;
}

static void xml_end_assetlist_element(void)
{
    data.finished = 1;
    data.current_group = 0;
}

static void xml_end_image_element(void)
{
#ifndef BUILDING_ASSET_PACKER
    image *img = &data.current_image->img;
    if (img->is_isometric) {
        if (((img->width + 2) % (FOOTPRINT_WIDTH + 2)) != 0) {
            log_info("Isometric image has invalid width", data.current_image->id, img->width);
        }
    }
    if (!img->width || !img->height) {
        asset_image_unload(data.current_image);
        return;
    }

    asset_image_check_and_handle_reference(data.current_image);
#endif
}

static void xml_end_animation_element(void)
{
    data.in_animation = 0;
}

int xml_process_assetlist_file(const char *xml_file_name)
{
    log_info("Loading assetlist file", xml_file_name, 0);

    FILE *xml_file = file_open_asset(xml_file_name, "r");

    if (!xml_file) {
        log_error("Error opening assetlist file", xml_file_name, 0);
        return 0;
    }

    xml_parser_init(xml_elements, XML_TOTAL_ELEMENTS);

    char buffer[XML_BUFFER_SIZE];
    int done = 0;
    int error = 0;

    do {
        size_t bytes_read = fread(buffer, 1, XML_BUFFER_SIZE, xml_file);
        done = bytes_read < sizeof(buffer);
        if (!xml_parser_parse(buffer, bytes_read, done)) {
            log_error("Error parsing file", xml_file_name, 0);
            error = 1;
            break;
        }
    } while (!done);

    if (data.current_group && (error || !data.finished)) {
        group_unload_current();
    }
#ifdef BUILDING_ASSET_PACKER
    else {
        size_t xml_file_name_length = strlen(xml_file_name);
        char *path = malloc(sizeof(char *) * (xml_file_name_length + 1));
        if (!path) {
            data.error = 1;
            group_unload_current();
        } else {
            strcpy(path, xml_file_name);
            group_get_current()->path = path;
        }
    }
#endif

    data.finished = 0;

    xml_parser_free();
    file_close(xml_file);

    return !error;
}

void xml_get_full_image_path(char *full_path, const char *image_file_name)
{
    strncpy(full_path, data.file_name, data.file_name_position);
    size_t file_name_size = strlen(image_file_name);
    strncpy(full_path + data.file_name_position, image_file_name, FILE_NAME_MAX - data.file_name_position);
    strncpy(full_path + data.file_name_position + file_name_size, ".png",
        FILE_NAME_MAX - data.file_name_position - file_name_size);
}

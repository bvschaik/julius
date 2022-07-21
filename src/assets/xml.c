#include "xml.h"

#include "assets/group.h"
#include "assets/image.h"
#include "core/calc.h"
#include "core/dir.h"
#include "core/file.h"
#include "core/log.h"
#include "core/png_read.h"
#include "core/string.h"
#include "graphics/renderer.h"

#include "expat.h"

#include <string.h>

#define XML_HASH_SEED 0x12345678
#define XML_BUFFER_SIZE 1024
#define XML_MAX_DEPTH 4
#define XML_MAX_ELEMENTS_PER_DEPTH 2
#define XML_MAX_ATTRIBUTES 13
#define XML_TAG_MAX_LENGTH 12

static const char XML_FILE_ELEMENTS[XML_MAX_DEPTH][XML_MAX_ELEMENTS_PER_DEPTH][XML_TAG_MAX_LENGTH] = { { "assetlist" }, { "image" }, { "layer", "animation" }, { "frame" } };
static const char XML_FILE_ATTRIBUTES[XML_MAX_DEPTH][XML_MAX_ELEMENTS_PER_DEPTH][XML_MAX_ATTRIBUTES][XML_TAG_MAX_LENGTH] = {
    { { "name" } }, // assetlist
    { { "id", "src", "width", "height", "group", "image", "isometric" }}, // image
    { { "src", "group", "image", "src_x", "src_y", "x", "y", "width", "height", "invert", "rotate", "part", "grayscale" }, // layer
    { "frames", "speed", "reversible", "x", "y" } }, // animation
    { { "src", "src_x", "src_y", "width", "height", "group", "image", "invert", "rotate" } } // frame
};

static void xml_start_assetlist_element(const char **attributes);
static void xml_start_image_element(const char **attributes);
static void xml_start_layer_element(const char **attributes);
static void xml_start_animation_element(const char **attributes);
static void xml_start_frame_element(const char **attributes);
static void xml_end_assetlist_element(void);
static void xml_end_image_element(void);
static void xml_end_layer_element(void);
static void xml_end_animation_element(void);
static void xml_end_frame_element(void);

static void (*xml_start_element_callback[XML_MAX_DEPTH][XML_MAX_ELEMENTS_PER_DEPTH])(const char **attributes) = {
    { xml_start_assetlist_element },
    { xml_start_image_element },
    { xml_start_layer_element, xml_start_animation_element },
    { xml_start_frame_element }
};

static void (*xml_end_element_callback[XML_MAX_DEPTH][XML_MAX_ELEMENTS_PER_DEPTH])(void) = {
    { xml_end_assetlist_element },
    { xml_end_image_element },
    { xml_end_layer_element, xml_end_animation_element },
    { xml_end_frame_element }
};

static struct {
    char file_name[FILE_NAME_MAX];
    size_t file_name_position;
    int depth;
    int error;
    int finished;
    int in_animation;
    image_groups *current_group;
    asset_image *current_image;
} data;

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

static int count_xml_attributes(const char **attributes)
{
    int total = 0;
    while (attributes[total]) {
        ++total;
    }
    return total;
}

static void xml_start_assetlist_element(const char **attributes)
{
    data.current_group = group_get_new();
    if (count_xml_attributes(attributes) != 2) {
        data.error = 1;
        return;
    }
    char *name = 0;
    if (strcmp(attributes[0], XML_FILE_ATTRIBUTES[0][0][0]) == 0) {
        size_t name_length = strlen(attributes[1]);
        name = malloc(sizeof(char) * (name_length + 1));
        if (name) {
            strcpy(name, attributes[1]);
            data.current_group->name = name;
        }
    }
    if (!name || *name == '\0') {
        free(name);
        data.error = 1;
        return;
    }
    data.current_image = 0;
    set_asset_image_base_path(data.current_group->name);
}

static void xml_start_image_element(const char **attributes)
{
    int total_attributes = count_xml_attributes(attributes);
    if (total_attributes > 12 || total_attributes % 2) {
        data.error = 1;
        return;
    }
    asset_image *img = asset_image_create();
    if (!img) {
        data.error = 1;
        return;
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
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[1][0][0]) == 0) {
            size_t img_id_length = strlen(attributes[i + 1]);
            char *img_id = malloc(sizeof(char) * (img_id_length + 1));
            if (img_id) {
                strcpy(img_id, attributes[i + 1]);
                img->id = img_id;
            }
        } else if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[1][0][1]) == 0) {
            path = attributes[i + 1];
        } else if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[1][0][2]) == 0) {
            img->img.width = string_to_int(string_from_ascii(attributes[i + 1]));
#ifdef BUILDING_ASSET_PACKER
            img->has_defined_size = 1;
#endif
        } else if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[1][0][3]) == 0) {
            img->img.height = string_to_int(string_from_ascii(attributes[i + 1]));
#ifdef BUILDING_ASSET_PACKER
            img->has_defined_size = 1;
#endif
        } else if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[1][0][4]) == 0) {
            group = attributes[i + 1];
        } else if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[1][0][5]) == 0) {
            id = attributes[i + 1];
        } else if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[1][0][6]) == 0) {
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
}

static void xml_start_layer_element(const char **attributes)
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
    int grayscale = 0;
    asset_image *img = data.current_image;
    int total_attributes = count_xml_attributes(attributes);
    if (total_attributes < 2 || total_attributes > 24 || total_attributes % 2) {
        data.error = 1;
        return;
    }
    layer_invert_type invert = INVERT_NONE;
    layer_rotate_type rotate = ROTATE_NONE;
    layer_isometric_part part = PART_BOTH;
    for (int i = 0; i < total_attributes; i += 2) {
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][0][0]) == 0) {
            path = attributes[i + 1];
        } else if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][0][1]) == 0) {
            group = attributes[i + 1];
        } else if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][0][2]) == 0) {
            id = attributes[i + 1];
        } else if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][0][3]) == 0) {
            src_x = string_to_int(string_from_ascii(attributes[i + 1]));
        } else if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][0][4]) == 0) {
            src_y = string_to_int(string_from_ascii(attributes[i + 1]));
        } else if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][0][5]) == 0) {
            offset_x = string_to_int(string_from_ascii(attributes[i + 1]));
        } else if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][0][6]) == 0) {
            offset_y = string_to_int(string_from_ascii(attributes[i + 1]));
        } else if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][0][7]) == 0) {
            width = string_to_int(string_from_ascii(attributes[i + 1]));
        } else if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][0][8]) == 0) {
            height = string_to_int(string_from_ascii(attributes[i + 1]));
        } else if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][0][9]) == 0) {
            if (strcmp(attributes[i + 1], "horizontal") == 0) {
                invert = INVERT_HORIZONTAL;
            } else if (strcmp(attributes[i + 1], "vertical") == 0) {
                invert = INVERT_VERTICAL;
            } else if (strcmp(attributes[i + 1], "both") == 0) {
                invert = INVERT_BOTH;
            }
        } else if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][0][10]) == 0) {
            if (strcmp(attributes[i + 1], "90") == 0) {
                rotate = ROTATE_90_DEGREES;
            } else if (strcmp(attributes[i + 1], "180") == 0) {
                rotate = ROTATE_180_DEGREES;
            } else if (strcmp(attributes[i + 1], "270") == 0) {
                rotate = ROTATE_270_DEGREES;
            }
        } else if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][0][11]) == 0) {
            if (strcmp(attributes[i + 1], "footprint") == 0) {
                part = PART_FOOTPRINT;
            } else if (strcmp(attributes[i + 1], "top") == 0) {
                part = PART_TOP;
            }
        } else if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][0][12]) == 0) {
            const char *value = attributes[i + 1];
            if (strcmp(value, "true") == 0 || strcmp(value, "1") == 0 || strcmp(value, "grayscale") == 0 ||
                strcmp(value, "yes") == 0 || strcmp(value, "y") == 0) {
                grayscale = 1;
            }
        }
    }
    if (!asset_image_add_layer(img, path, group, id, src_x, src_y,
        offset_x, offset_y, width, height, invert, rotate, part, grayscale)) {
        log_info("Invalid layer for image", img->id, 0);
        return;
    }
}

static void xml_start_animation_element(const char **attributes)
{
    asset_image *img = data.current_image;
    if (img->img.animation) {
        return;
    }
    int total_attributes = count_xml_attributes(attributes);
    if (total_attributes % 2) {
        data.error = 1;
        return;
    }
    img->img.animation = malloc(sizeof(image_animation));
    if (!img->img.animation) {
        data.error = 1;
        return;
    }
    memset(img->img.animation, 0, sizeof(image_animation));

    for (int i = 0; i < total_attributes; i += 2) {
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][1][0]) == 0) {
            img->img.animation->num_sprites = string_to_int(string_from_ascii(attributes[i + 1]));
        } else if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][1][1]) == 0) {
            img->img.animation->speed_id = calc_bound(string_to_int(string_from_ascii(attributes[i + 1])), 0, 50);
        } else if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][1][2]) == 0) {
            const char *value = attributes[i + 1];
            if (strcmp(value, "true") == 0 || strcmp(value, "1") == 0 || strcmp(value, "reversible") == 0 ||
                strcmp(value, "yes") == 0 || strcmp(value, "y") == 0) {
                img->img.animation->can_reverse = 1;
            }
        } else if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][1][3]) == 0) {
            img->img.animation->sprite_offset_x = string_to_int(string_from_ascii(attributes[i + 1]));
        } else if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][1][4]) == 0) {
            img->img.animation->sprite_offset_y = string_to_int(string_from_ascii(attributes[i + 1]));
        }
    }
    if (!img->img.animation->num_sprites) {
        data.in_animation = 1;
    }
}

static void xml_start_frame_element(const char **attributes)
{
    int total_attributes = count_xml_attributes(attributes);
    if (!data.in_animation || total_attributes < 2 || total_attributes % 2) {
        return;
    }
    asset_image *img = asset_image_create();
    if (!img) {
        data.error = 1;
        return;
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
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[3][0][0]) == 0) {
            path = attributes[i + 1];
        } else if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[3][0][1]) == 0) {
            src_x = string_to_int(string_from_ascii(attributes[i + 1]));
        } else if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[3][0][2]) == 0) {
            src_y = string_to_int(string_from_ascii(attributes[i + 1]));
        } else if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[3][0][3]) == 0) {
            width = string_to_int(string_from_ascii(attributes[i + 1]));
        } else if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[3][0][4]) == 0) {
            height = string_to_int(string_from_ascii(attributes[i + 1]));
        } else if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[3][0][5]) == 0) {
            group = attributes[i + 1];
        } else if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[3][0][6]) == 0) {
            id = attributes[i + 1];
        } else if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[3][0][7]) == 0) {
            if (strcmp(attributes[i + 1], "horizontal") == 0) {
                invert = INVERT_HORIZONTAL;
            } else if (strcmp(attributes[i + 1], "vertical") == 0) {
                invert = INVERT_VERTICAL;
            } else if (strcmp(attributes[i + 1], "both") == 0) {
                invert = INVERT_BOTH;
            }
        } else if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[3][0][8]) == 0) {
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
        return;
    }
#ifndef BUILDING_ASSET_PACKER
    if (!img->img.width || !img->img.height) {
        asset_image_unload(img);
        return;
    }
    asset_image_check_and_handle_reference(img);
#else
    data.current_image->has_frame_elements = 1;
#endif
    data.current_group->last_image_index = img->index;
    data.current_image->img.animation->num_sprites++;
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

static void xml_end_layer_element(void)
{}

static void xml_end_animation_element(void)
{
    data.in_animation = 0;
}

static void xml_end_frame_element(void)
{}

static int get_element_index(const char *name)
{
    for (int i = 0; i < XML_MAX_ELEMENTS_PER_DEPTH; ++i) {
        if (XML_FILE_ELEMENTS[data.depth][i][0] == 0) {
            continue;
        }
        if (strcmp(XML_FILE_ELEMENTS[data.depth][i], name) == 0) {
            return i;
        }
    }
    return -1;
}

static void XMLCALL xml_start_element(void *unused, const char *name, const char **attributes)
{
    if (data.error) {
        return;
    }
    if (data.finished || data.depth == XML_MAX_DEPTH) {
        data.error = 1;
        log_error("Invalid XML parameter", name, 0);
        return;
    }
    int index = get_element_index(name);
    if (index == -1) {
        data.error = 1;
        log_error("Invalid XML parameter", name, 0);
        return;
    }
    (*xml_start_element_callback[data.depth][index])(attributes);
    data.depth++;
}

static void XMLCALL xml_end_element(void *unused, const char *name)
{
    if (data.error) {
        return;
    }
    data.depth--;
    int index = get_element_index(name);
    if (index == -1) {
        data.error = 1;
        log_error("Invalid XML parameter", name, 0);
        return;
    }
    (*xml_end_element_callback[data.depth][index])();
}

static void clear_xml_info(void)
{
    data.error = 0;
    data.depth = 0;
    data.finished = 0;
}

int xml_process_assetlist_file(const char *xml_file_name)
{
    log_info("Loading assetlist file", xml_file_name, 0);

    FILE *xml_file = file_open_asset(xml_file_name, "r");

    if (!xml_file) {
        log_error("Error opening assetlist file", xml_file_name, 0);
        return 0;
    }

    XML_Parser parser = XML_ParserCreate(NULL);
    XML_SetHashSalt(parser, XML_HASH_SEED);
    XML_SetElementHandler(parser, xml_start_element, xml_end_element);

    char buffer[XML_BUFFER_SIZE];
    int done = 0;

    do {
        size_t bytes_read = fread(buffer, 1, XML_BUFFER_SIZE, xml_file);
        done = bytes_read < sizeof(buffer);
        if (XML_Parse(parser, buffer, (int) bytes_read, done) == XML_STATUS_ERROR || data.error) {
            log_error("Error parsing file", xml_file_name, 0);
            break;
        }
    } while (!done);

    if (data.current_group && (data.error || !data.finished)) {
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

    clear_xml_info();

    XML_ParserFree(parser);
    file_close(xml_file);

    return !data.error;
}

void xml_get_full_image_path(char *full_path, const char *image_file_name)
{
    strncpy(full_path, data.file_name, data.file_name_position);
    size_t file_name_size = strlen(image_file_name);
    strncpy(full_path + data.file_name_position, image_file_name, FILE_NAME_MAX - data.file_name_position);
    strncpy(full_path + data.file_name_position + file_name_size, ".png",
        FILE_NAME_MAX - data.file_name_position - file_name_size);
}

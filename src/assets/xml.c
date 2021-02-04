#include "xml.h"

#include "assets/group.h"
#include "assets/image.h"
#include "core/calc.h"
#include "core/dir.h"
#include "core/file.h"
#include "core/log.h"
#include "core/png_read.h"
#include "core/string.h"

#include "expat.h"

#include <string.h>

#define XML_BUFFER_SIZE 1024
#define XML_MAX_DEPTH 3
#define XML_MAX_ELEMENTS_PER_DEPTH 2
#define XML_MAX_ATTRIBUTES 8
#define XML_TAG_MAX_LENGTH 12
#define XML_MAX_IMAGE_INDEXES 256

static const char XML_FILE_ELEMENTS[XML_MAX_DEPTH][XML_MAX_ELEMENTS_PER_DEPTH][XML_TAG_MAX_LENGTH] = { { "assetlist" }, { "image" }, { "layer", "animation" } };
static const char XML_FILE_ATTRIBUTES[XML_MAX_DEPTH][XML_MAX_ELEMENTS_PER_DEPTH][XML_MAX_ATTRIBUTES][XML_TAG_MAX_LENGTH] = {
    { { "author", "name" } }, // assetlist
    { { "id", "src", "width", "height", "group", "image", "index" } }, // image
    { { "src", "group", "image", "x", "y", "invert", "rotate", "part" }, // layer
    { "frames", "speed", "reversible", "x", "y" } } // animation
};

static void xml_start_assetlist_element(const char **attributes);
static void xml_start_image_element(const char **attributes);
static void xml_start_layer_element(const char **attributes);
static void xml_start_animation_element(const char **attributes);
static void xml_end_assetlist_element(void);
static void xml_end_image_element(void);
static void xml_end_layer_element(void);
static void xml_end_animation_element(void);

static void (*xml_start_element_callback[XML_MAX_DEPTH][XML_MAX_ELEMENTS_PER_DEPTH])(const char **attributes) = {
    { xml_start_assetlist_element }, { xml_start_image_element }, { xml_start_layer_element, xml_start_animation_element }
};

static void (*xml_end_element_callback[XML_MAX_DEPTH][XML_MAX_ELEMENTS_PER_DEPTH])(void) = {
    { xml_end_assetlist_element }, { xml_end_image_element }, { xml_end_layer_element, xml_end_animation_element }
};

static struct {
    char file_name[FILE_NAME_MAX];
    size_t file_name_position;
    int image_index;
    int depth;
    int error;
    int finished;
    asset_image *current_image;
} data;

static void set_asset_image_base_path(const char *author, const char *name)
{
    size_t position = 0;
    char *dst = data.file_name;
    memset(dst, 0, FILE_NAME_MAX);
    strncpy(dst, author, FILE_NAME_MAX - 1);
    position += strlen(author);
    dst[position++] = '/';
    strncpy(dst + position, name, FILE_NAME_MAX - position - 1);
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
    image_groups *group = group_get_new();
    if (count_xml_attributes(attributes) != 4) {
        data.error = 1;
        return;
    }
    group->first_image = 0;
    for (int i = 0; i < 4; i += 2) {
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[0][0][0]) == 0) {
            strncpy(group->author, attributes[i + 1], XML_STRING_MAX_LENGTH - 1);
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[0][0][1]) == 0) {
            strncpy(group->name, attributes[i + 1], XML_STRING_MAX_LENGTH - 1);
        }
    }
    if (*group->author == '\0' || *group->name == '\0') {
        data.error = 1;
        return;
    }
    data.image_index = 0;
    data.current_image = 0;
    group->id = group_get_hash(group->author, group->name);
    set_asset_image_base_path(group->author, group->name);
}

static void xml_start_image_element(const char **attributes)
{
    if (data.image_index >= XML_MAX_IMAGE_INDEXES) {
        log_info("Image index number in the xml file exceeds the maximum. Further images for this xml will not be loaded:", data.file_name, 0);
        return;
    }
    int total_attributes = count_xml_attributes(attributes);
    if (total_attributes < 2 || total_attributes > 12 || total_attributes % 2) {
        data.error = 1;
        return;
    }
    asset_image *img = malloc(sizeof(asset_image));
    if (!img) {
        data.error = 1;
        return;
    }
    memset(img, 0, sizeof(asset_image));
    if (!data.current_image) {
        group_get_current()->first_image = img;
    } else {
        data.current_image->next = img;
    }
    data.current_image = img;
    const char *path = 0;
    const char *group = 0;
    const char *id = 0;
    for (int i = 0; i < total_attributes; i += 2) {
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[1][0][0]) == 0) {
            strncpy(img->id, attributes[i + 1], XML_STRING_MAX_LENGTH - 1);
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[1][0][1]) == 0) {
            path = attributes[i + 1];
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[1][0][2]) == 0) {
            img->img.width = string_to_int(string_from_ascii(attributes[i + 1]));
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[1][0][3]) == 0) {
            img->img.height = string_to_int(string_from_ascii(attributes[i + 1]));
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[1][0][4]) == 0) {
            group = attributes[i + 1];
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[1][0][5]) == 0) {
            id = attributes[i + 1];
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[1][0][6]) == 0) {
            int index = string_to_int(string_from_ascii(attributes[i + 1]));
            if (data.image_index < index) {
                data.image_index = index;
            }
        }
    }
    img->last_layer = &img->first_layer;
    img->index = data.image_index;
    if (path || group) {
        asset_image_add_layer(img, path, group, id, 0, 0, INVERT_NONE, ROTATE_NONE, PART_BOTH);
    }
}

static void xml_start_layer_element(const char **attributes)
{
    if (data.image_index >= XML_MAX_IMAGE_INDEXES) {
        return;
    }
    const char *path = 0;
    const char *group = 0;
    const char *id = 0;
    int offset_x = 0;
    int offset_y = 0;
    asset_image *img = data.current_image;
    int total_attributes = count_xml_attributes(attributes);
    if (total_attributes < 2 || total_attributes > 14 || total_attributes % 2) {
        data.error = 1;
        return;
    }
    layer_invert_type invert = INVERT_NONE;
    layer_rotate_type rotate = ROTATE_NONE;
    layer_isometric_part part = PART_BOTH;
    for (int i = 0; i < total_attributes; i += 2) {
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][0][0]) == 0) {
            path = attributes[i + 1];
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][0][1]) == 0) {
            group = attributes[i + 1];
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][0][2]) == 0) {
            id = attributes[i + 1];
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][0][3]) == 0) {
            offset_x = string_to_int(string_from_ascii(attributes[i + 1]));
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][0][4]) == 0) {
            offset_y = string_to_int(string_from_ascii(attributes[i + 1]));
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][0][5]) == 0) {
            if (strcmp(attributes[i + 1], "horizontal") == 0) {
                invert = INVERT_HORIZONTAL;
            } else if (strcmp(attributes[i + 1], "vertical") == 0) {
                invert = INVERT_VERTICAL;
            } else if (strcmp(attributes[i + 1], "both") == 0) {
                invert = INVERT_BOTH;
            }
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][0][6]) == 0) {
            if (strcmp(attributes[i + 1], "90") == 0) {
                rotate = ROTATE_90_DEGREES;
            } else if (strcmp(attributes[i + 1], "180") == 0) {
                rotate = ROTATE_180_DEGREES;
            } else if (strcmp(attributes[i + 1], "270") == 0) {
                rotate = ROTATE_270_DEGREES;
            }
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][0][7]) == 0) {
            if (strcmp(attributes[i + 1], "footprint") == 0) {
                part = PART_FOOTPRINT;
            } else if (strcmp(attributes[i + 1], "top") == 0) {
                part = PART_TOP;
            }
        }
    }
    if (!asset_image_add_layer(img, path, group, id, offset_x, offset_y, invert, rotate, part)) {
        log_info("Invalid layer for image", img->id, 0);
        return;
    }
}

static void xml_start_animation_element(const char **attributes)
{
    if (data.image_index >= XML_MAX_IMAGE_INDEXES) {
        return;
    }
    asset_image *img = data.current_image;
    if (img->img.num_animation_sprites) {
        return;
    }
    int total_attributes = count_xml_attributes(attributes);
    if (total_attributes % 2) {
        data.error = 1;
        return;
    }

    for (int i = 0; i < total_attributes; i += 2) {
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][1][0]) == 0) {
            img->img.num_animation_sprites = string_to_int(string_from_ascii(attributes[i + 1]));
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][1][1]) == 0) {
            img->img.animation_speed_id = calc_bound(string_to_int(string_from_ascii(attributes[i + 1])), 0, 50);
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][1][2]) == 0) {
            const char *value = attributes[i + 1];
            if (strcmp(value, "true") == 0 || strcmp(value, "1") == 0 || strcmp(value, "reversible") == 0 ||
                strcmp(value, "yes") == 0 || strcmp(value, "y") == 0) {
                img->img.animation_can_reverse = 1;
            }
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][1][3]) == 0) {
            img->img.sprite_offset_x = string_to_int(string_from_ascii(attributes[i + 1]));
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][1][4]) == 0) {
            img->img.sprite_offset_y = string_to_int(string_from_ascii(attributes[i + 1]));
        }
    }
}

static void xml_end_assetlist_element(void)
{
    data.finished = 1;
}

static void xml_end_image_element(void)
{
    if (data.image_index >= XML_MAX_IMAGE_INDEXES) {
        return;
    }
    image_groups *group = group_get_current();
    image *img = &data.current_image->img;
    img->draw.data_length = img->width * img->height * sizeof(color_t);
    img->draw.uncompressed_length = img->draw.data_length;
    if (!img->draw.data_length) {
        asset_image *prev = 0;
        asset_image *latest_image = group->first_image;
        while (latest_image) {
            if (latest_image == data.current_image) {
                break;
            }
            prev = latest_image;
            latest_image = latest_image->next;
        }
        if (prev) {
            prev->next = 0;
        } else {
            group->first_image = 0;
        }
        free(data.current_image);

        return;
    }
    img->draw.type = IMAGE_TYPE_EXTRA_ASSET;
    data.current_image->active = 1;
    if (img->draw.data_length < IMAGE_PRELOAD_MAX_SIZE) {
        asset_image_load(data.current_image);
    }
    data.image_index++;
}

static void xml_end_layer_element(void)
{
}

static void xml_end_animation_element(void)
{
}

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
    data.image_index = 0;
}

void xml_process_assetlist_file(const char *xml_file_name)
{
    log_info("Loading assetlist file", xml_file_name, 0);

    FILE *xml_file = file_open_asset(xml_file_name, "r");

    if (!xml_file) {
        log_error("Error opening assetlist file", xml_file_name, 0);
        return;
    }

    XML_Parser parser = XML_ParserCreate(NULL);
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

    if (data.error || !data.finished) {
        group_unload_current();
    }

    clear_xml_info();

    XML_ParserFree(parser);
    file_close(xml_file);
}

void xml_get_full_image_path(char *full_path, const char *image_file_name)
{
    strncpy(full_path, data.file_name, data.file_name_position);
    size_t file_name_size = strlen(image_file_name);
    strncpy(full_path + data.file_name_position, image_file_name, FILE_NAME_MAX - data.file_name_position);
    strncpy(full_path + data.file_name_position + file_name_size, ".png",
        FILE_NAME_MAX - data.file_name_position - file_name_size);
}

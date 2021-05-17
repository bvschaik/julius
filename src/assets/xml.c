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
#define XML_MAX_DEPTH 4
#define XML_MAX_ELEMENTS_PER_DEPTH 2
#define XML_MAX_ATTRIBUTES 8
#define XML_TAG_MAX_LENGTH 12
#define XML_MAX_IMAGE_INDEXES 256

static const char XML_FILE_ELEMENTS[XML_MAX_DEPTH][XML_MAX_ELEMENTS_PER_DEPTH][XML_TAG_MAX_LENGTH] = { { "assetlist" }, { "image" }, { "layer", "animation" }, { "frame" } };
static const char XML_FILE_ATTRIBUTES[XML_MAX_DEPTH][XML_MAX_ELEMENTS_PER_DEPTH][XML_MAX_ATTRIBUTES][XML_TAG_MAX_LENGTH] = {
    { { "author", "name" } }, // assetlist
    { { "id", "src", "width", "height", "group", "image", "index" } }, // image
    { { "src", "group", "image", "x", "y", "invert", "rotate", "part" }, // layer
    { "frames", "speed", "reversible", "x", "y" } }, // animation
    { { "src", "width", "height", "group", "image" } } // frame
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
    int image_index;
    int depth;
    int error;
    int finished;
    int frame_image_index;
    int in_animation;
    image_groups *current_group;
    asset_image *current_image;
    asset_image *current_animation;
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
    data.current_group = group_get_new();
    if (count_xml_attributes(attributes) != 4) {
        data.error = 1;
        return;
    }
    data.current_group->first_image = 0;
    for (int i = 0; i < 4; i += 2) {
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[0][0][0]) == 0) {
            strncpy(data.current_group->author, attributes[i + 1], XML_STRING_MAX_LENGTH - 1);
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[0][0][1]) == 0) {
            strncpy(data.current_group->name, attributes[i + 1], XML_STRING_MAX_LENGTH - 1);
        }
    }
    if (*data.current_group->author == '\0' || *data.current_group->name == '\0') {
        data.error = 1;
        return;
    }
    data.image_index = 0;
    data.current_image = 0;
    data.current_group->id = group_get_hash(data.current_group->author, data.current_group->name);
    set_asset_image_base_path(data.current_group->author, data.current_group->name);
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
        data.current_group->first_image = img;
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
            } else if (data.image_index > index) {
                log_error("The manually set index is higher than the current index. The manual index will be ignored. The current index is ", 0, data.image_index);
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
    if (!img->img.num_animation_sprites) {
        data.in_animation = 1;
    }
}

static void xml_start_frame_element(const char **attributes)
{
    int total_attributes = count_xml_attributes(attributes);
    if (!data.in_animation || total_attributes < 2 || total_attributes % 2) {
        return;
    }
    asset_image *img = malloc(sizeof(asset_image));
    if (!img) {
        data.error = 1;
        return;
    }
    memset(img, 0, sizeof(asset_image));

    const char *path = 0;
    const char *group = 0;
    const char *id = 0;
    for (int i = 0; i < total_attributes; i += 2) {
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[3][0][0]) == 0) {
            path = attributes[i + 1];
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[3][0][1]) == 0) {
            img->img.width = string_to_int(string_from_ascii(attributes[i + 1]));
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[3][0][2]) == 0) {
            img->img.height = string_to_int(string_from_ascii(attributes[i + 1]));
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[3][0][3]) == 0) {
            group = attributes[i + 1];
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[3][0][4]) == 0) {
            id = attributes[i + 1];
        }
    }
    img->last_layer = &img->first_layer;
    img->index = data.frame_image_index;
    if (!path && !(group && id)) {
        free(img);
        return;
    }
    asset_image_add_layer(img, path, group, id, 0, 0, INVERT_NONE, ROTATE_NONE, PART_BOTH);
    img->img.draw.data_length = img->img.width * img->img.height * sizeof(color_t);
    img->img.draw.uncompressed_length = img->img.draw.data_length;
    if (!img->img.draw.data_length) {
        free(img);
        return;
    }
    img->img.draw.type = IMAGE_TYPE_EXTRA_ASSET;
    img->active = 1;
    asset_image_load(img);

    data.frame_image_index++;
    if (!data.current_image->img.animation_start_offset) {
        data.current_image->img.animation_start_offset = (ANIMATION_FRAMES_GROUP + img->index) -
            (data.current_group->id + data.current_image->index) - 1;
    }
    image_groups *animations = group_get_from_hash(ANIMATION_FRAMES_GROUP);
    if (!animations->first_image) {
        animations->first_image = img;
    } else {
        data.current_animation->next = img;
    }
    data.current_animation = img;
    data.current_image->img.num_animation_sprites++;
}

static void xml_end_assetlist_element(void)
{
    data.finished = 1;
    data.current_group = 0;
}

static void xml_end_image_element(void)
{
    if (data.image_index >= XML_MAX_IMAGE_INDEXES) {
        return;
    }
    image *img = &data.current_image->img;
    img->draw.data_length = img->width * img->height * sizeof(color_t);
    img->draw.uncompressed_length = img->draw.data_length;
    if (!img->draw.data_length) {
        asset_image *prev = 0;
        asset_image *latest_image = data.current_group->first_image;
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
            data.current_group->first_image = 0;
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
    XML_SetHashSalt(parser, HASH_SEED);
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

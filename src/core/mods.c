#include "mods.h"

#include "core/calc.h"
#include "core/dir.h"
#include "core/file.h"
#include "core/log.h"
#include "core/png_read.h"
#include "core/string.h"
#include "graphics/color.h"
#include "graphics/graphics.h"
#include "graphics/image.h"

#include "expat.h"
#include "PMurHash/PMurHash.h"

#include <string.h>

#define XML_BUFFER_SIZE 1024
#define XML_MAX_DEPTH 3
#define XML_MAX_ELEMENTS_PER_DEPTH 2
#define XML_MAX_ATTRIBUTES 7
#define XML_TAG_MAX_LENGTH 12
#define XML_STRING_MAX_LENGTH 32
#define XML_MAX_IMAGE_INDEXES 256

#define MAX_LAYERS 5
#define MAX_GROUPS 100

#define IMAGE_PRELOAD_MAX_SIZE 65535

// Do not change the seed. Doing so breaks savegame compatibility with mod images
#define MOD_HASH_SEED 0x12345678

static const char *MODS_FOLDER = "mods";
static const char XML_FILE_ELEMENTS[XML_MAX_DEPTH][XML_MAX_ELEMENTS_PER_DEPTH][XML_TAG_MAX_LENGTH] = { { "mod" }, { "image" }, { "layer", "animation" } };
static const char XML_FILE_ATTRIBUTES[XML_MAX_DEPTH][XML_MAX_ELEMENTS_PER_DEPTH][XML_MAX_ATTRIBUTES][XML_TAG_MAX_LENGTH] = {
    { { "author", "name" } }, // mod
    { { "id", "src", "width", "height", "group", "image", "index" } }, // image
    { { "src", "group", "image", "x", "y" }, // layer
    { "frames", "speed", "reversible", "x", "y" } } // animation
};
static color_t DUMMY_IMAGE_DATA = COLOR_BLACK;

static void xml_start_mod_element(const char **attributes);
static void xml_start_image_element(const char **attributes);
static void xml_start_layer_element(const char **attributes);
static void xml_start_animation_element(const char **attributes);
static void xml_end_mod_element(void);
static void xml_end_image_element(void);
static void xml_end_layer_element(void);
static void xml_end_animation_element(void);

static void (*xml_start_element_callback[XML_MAX_DEPTH][XML_MAX_ELEMENTS_PER_DEPTH])(const char **attributes) = {
    { xml_start_mod_element }, { xml_start_image_element }, { xml_start_layer_element, xml_start_animation_element }
};

static void (*xml_end_element_callback[XML_MAX_DEPTH][XML_MAX_ELEMENTS_PER_DEPTH])(void) = {
    { xml_end_mod_element }, { xml_end_image_element }, { xml_end_layer_element, xml_end_animation_element }
};

typedef struct {
    char *modded_image_path;
    int original_image_id;
    int x_offset;
    int y_offset;
    int width;
    int height;
    int is_modded_image_reference;
    color_t *data;
} layer;

typedef struct {
    int active;
    int loaded;
    char id[XML_STRING_MAX_LENGTH];
    layer layers[MAX_LAYERS];
    int num_layers;
    image img;
    color_t *data;
    int index;
} modded_image;

typedef struct {
    char author[XML_STRING_MAX_LENGTH];
    char name[XML_STRING_MAX_LENGTH];
    int id;
    int images;
    int first_image_id;
} image_groups;

static struct {
    struct {
        char file_name[FILE_NAME_MAX];
        size_t file_name_position;
        char image_base_path[FILE_NAME_MAX];
        size_t image_base_path_position;
        int image_index;
        int depth;
        int error;
        int finished;
        modded_image *current_image;
    } xml;
    image_groups groups[MAX_GROUPS];
    modded_image images[MAX_MODDED_IMAGES];
    int total_groups;
    int total_images;
    int loaded;
    int roadblock_image;
} data;

static void get_full_image_path(char *full_path, const char *file_name)
{
    strncpy(full_path, data.xml.image_base_path, data.xml.image_base_path_position);
    size_t file_name_size = strlen(file_name);
    strncpy(full_path + data.xml.image_base_path_position, file_name, FILE_NAME_MAX - data.xml.image_base_path_position);
    strncpy(full_path + data.xml.image_base_path_position + file_name_size, ".png",
        FILE_NAME_MAX - data.xml.image_base_path_position - file_name_size);
}

static void load_dummy_layer(layer *l)
{
    l->data = &DUMMY_IMAGE_DATA;
    l->width = 1;
    l->height = 1;
}

static void load_layer(layer *l)
{
    int size = l->width * l->height * sizeof(color_t);
    l->data = malloc(size);
    if (!l->data) {
        log_error("Problem loading layer", l->modded_image_path, 0);
        load_dummy_layer(l);
        return;
    }
    memset(l->data, 0, size);
    if (l->modded_image_path) {
        if (!png_read(l->modded_image_path, (uint8_t *) l->data)) {
            free(l->data);
            log_error("Problem loading layer from file", l->modded_image_path, 0);
            load_dummy_layer(l);
        }
        return;
    }
    const image *layer_image = image_get(l->original_image_id);
    if (layer_image->draw.type == IMAGE_TYPE_MOD) {
        // Ugly const removal. The only other way would be to memcpy the image data.
        // That's a waste of ram, and we're not going to change l->data anyway
        l->data = (color_t *) image_data(l->original_image_id);
        l->is_modded_image_reference = 1;
        if (!l->data) {
            log_error("Problem loading layer from image id", 0, l->original_image_id);
            load_dummy_layer(l);
        }
        return;
    }
    canvas_type type = graphics_get_canvas_type();
    graphics_set_custom_canvas(l->data, l->width, l->height);
    if (layer_image->draw.type == IMAGE_TYPE_ISOMETRIC) {
        int tiles = (l->width + 2) / 60;
        int y_offset = l->height - 30 * tiles;
        y_offset += 15 * tiles - 15;
        image_draw_isometric_footprint_from_draw_tile(l->original_image_id, 0, y_offset, 0);
        image_draw_isometric_top_from_draw_tile(l->original_image_id, 0, y_offset, 0);
    } else {
        image_draw(l->original_image_id, 0, 0);
    }
    graphics_set_active_canvas(type);
}

static void unload_layer(layer *l)
{
    free(l->modded_image_path);
    l->modded_image_path = 0;
    if (!l->is_modded_image_reference) {
        free(l->data);
    }
    l->data = 0;
}

static color_t layer_get_color_for_image_position(layer *l, int x, int y)
{
    x -= l->x_offset;
    y -= l->y_offset;
    if (x < 0 || x >= l->width || y < 0 || y >= l->height) {
        return ALPHA_TRANSPARENT;
    }
    return l->data[y * l->width + x];
}

static int load_modded_image(modded_image *img)
{
    if (img->loaded) {
        return 1;
    }

    for (int i = 0; i < img->num_layers; ++i) {
        load_layer(&img->layers[i]);
    }

    // Special cases for images which are simple aliases to another mod image
   if (img->num_layers == 1) {
        layer *l = &img->layers[0];
        if (l->is_modded_image_reference &&
            img->img.width == l->width && img->img.height == l->height &&
            l->x_offset == 0 && l->y_offset == 0) {
            img->data = l->data;
            unload_layer(l);
            img->loaded = 1;
            return 1;
        }
    }

    img->data = malloc(img->img.draw.data_length);
    if (!img->data) {
        log_error("Not enough memory to load image", img->id, 0);
        for (int i = 0; i < img->num_layers; ++i) {
            unload_layer(&img->layers[i]);
        }
        img->active = 0;
        return 0;
    }
    memset(img->data, 0, img->img.draw.data_length);
    for (int y = 0; y < img->img.height; ++y) {
        color_t *pixel = &img->data[y * img->img.width];
        for (int x = 0; x < img->img.width; ++x) {
            for (int l = img->num_layers - 1; l >= 0; --l) {
                color_t image_pixel_alpha = *pixel & COLOR_CHANNEL_ALPHA;
                if (image_pixel_alpha == ALPHA_OPAQUE) {
                    break;
                }
                color_t layer_pixel = layer_get_color_for_image_position(&img->layers[l], x, y);
                color_t layer_pixel_alpha = layer_pixel & COLOR_CHANNEL_ALPHA;
                if (layer_pixel_alpha == ALPHA_TRANSPARENT) {
                    continue;
                }
                if (image_pixel_alpha == ALPHA_TRANSPARENT) {
                    *pixel = layer_pixel;
                } else if (layer_pixel_alpha == ALPHA_OPAQUE) {
                    color_t alpha = image_pixel_alpha >> COLOR_BITSHIFT_ALPHA;
                    *pixel = COLOR_BLEND_ALPHA_TO_OPAQUE(*pixel, layer_pixel, alpha);
                } else {
                    color_t alpha_src = image_pixel_alpha >> COLOR_BITSHIFT_ALPHA;
                    color_t alpha_dst = layer_pixel_alpha >> COLOR_BITSHIFT_ALPHA;
                    color_t alpha_mix = COLOR_MIX_ALPHA(alpha_src, alpha_dst);
                    *pixel = COLOR_BLEND_ALPHAS(*pixel, layer_pixel, alpha_src, alpha_dst, alpha_mix);
                }
            }
            ++pixel;
        }
    }
    for (int i = 0; i < img->num_layers; ++i) {
        unload_layer(&img->layers[i]);
    }
    img->loaded = 1;
    return 1;
}

static void set_modded_image_base_path(const char *author, const char *name)
{
    size_t position = data.xml.file_name_position;
    char *dst = data.xml.image_base_path;
    memset(dst + position, 0, FILE_NAME_MAX - position);
    strncpy(dst + position, author, FILE_NAME_MAX - position - 1);
    position += strlen(author);
    dst[position++] = '/';
    strncpy(dst + position, name, FILE_NAME_MAX - position - 1);
    position += strlen(name);
    dst[position++] = '/';
    data.xml.image_base_path_position = position;
}

static int add_layer_from_image_path(modded_image *img, const char *path, int offset_x, int offset_y)
{
    layer *current_layer = &img->layers[img->num_layers];
    current_layer->modded_image_path = malloc(FILE_NAME_MAX * sizeof(char));
    get_full_image_path(current_layer->modded_image_path, path);
    if (!png_get_image_size(current_layer->modded_image_path, &current_layer->width, &current_layer->height)) {
        log_info("Unable to load image", path, 0);
        unload_layer(current_layer);
        return 0;
    }
    if (!img->img.width) {
        img->img.width = current_layer->width;
    }
    if (!img->img.height) {
        img->img.height = current_layer->height;
    }
    current_layer->x_offset = offset_x;
    current_layer->y_offset = offset_y;
    img->num_layers++;
    return 1;
}

static int add_layer_from_image_id(modded_image *img, const char *group_id, const char *image_id, int offset_x, int offset_y)
{
    layer *current_layer = &img->layers[img->num_layers];
    current_layer->width = 0;
    current_layer->height = 0;
    const image *original_image = 0;
    if (strcmp(group_id, "this") == 0) {
        int image = data.groups[data.total_groups - 1].first_image_id;
        for (int i = 0; i < data.groups[data.total_groups - 1].images; ++i) {
            if (strcmp(data.images[image].id, image_id) == 0) {
                current_layer->original_image_id = data.groups[data.total_groups - 1].id + data.images[image].index;
                original_image = &data.images[image].img;
                break;
            }
            image++;
        }
        if (!current_layer->original_image_id) {
            return 0;
        }
    } else {
        int group = string_to_int(string_from_ascii(group_id));
        int id = image_id ? string_to_int(string_from_ascii(image_id)) : 0;
        current_layer->original_image_id = image_group(group) + id;
        original_image = image_get(current_layer->original_image_id);
    }
    if (!original_image) {
        return 0;
    }
    if (!img->img.width) {
        img->img.width = original_image->width;
    }
    if (!img->img.height) {
        img->img.height = original_image->height;
    }
    current_layer->width = original_image->width;
    current_layer->height = original_image->height;
    current_layer->x_offset = offset_x;
    current_layer->y_offset = offset_y;
    img->num_layers++;
    return 1;
}

static int count_xml_attributes(const char **attributes)
{
    int total = 0;
    while (attributes[total]) {
        ++total;
    }
    return total;
}

static uint32_t get_group_hash(const char *author, const char *name)
{
    uint32_t hash = MOD_HASH_SEED;
    uint32_t carry = 0;
    uint32_t author_length = (uint32_t) strlen(author);
    uint32_t name_length = (uint32_t) strlen(name);

    PMurHash32_Process(&hash, &carry, author, author_length);
    PMurHash32_Process(&hash, &carry, name, name_length);

    hash = PMurHash32_Result(hash, carry, author_length + name_length);

    // The following code increases the risk of hash collision but allows better image indexing
    if (hash < 0x4000) {
        hash |= 0x4000;
    }
    return hash & 0xffffff00;
}

static void xml_start_mod_element(const char **attributes)
{
    data.total_groups++;
    if (count_xml_attributes(attributes) != 4) {
        data.xml.error = 1;
        return;
    }
    image_groups *group = &data.groups[data.total_groups - 1];
    group->first_image_id = data.total_images;
    group->images = 0;
    for (int i = 0; i < 4; i += 2) {
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[0][0][0]) == 0) {
            strncpy(group->author, attributes[i + 1], XML_STRING_MAX_LENGTH - 1);
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[0][0][1]) == 0) {
            strncpy(group->name, attributes[i + 1], XML_STRING_MAX_LENGTH - 1);
        }
    }
    if (*group->author == '\0' || *group->name == '\0') {
        data.xml.error = 1;
        return;
    }
    data.xml.image_index = 0;
    group->id = get_group_hash(group->author, group->name);
    set_modded_image_base_path(group->author, group->name);
}

static void xml_start_image_element(const char **attributes)
{
    if (data.xml.image_index >= XML_MAX_IMAGE_INDEXES) {
        log_info("Image index number in the xml file exceeds the maximum. Further images for this xml will not be loaded:", data.xml.file_name, 0);
        return;
    }
    data.xml.current_image = &data.images[data.total_images];
    modded_image *img = data.xml.current_image;
    const char *path = 0;
    const char *group = 0;
    const char *id = 0;
    int total_attributes = count_xml_attributes(attributes);
    if (total_attributes < 2 || total_attributes > 12 || total_attributes % 2) {
        data.xml.error = 1;
        return;
    }
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
            if (data.xml.image_index < index) {
                data.xml.image_index = index;
            }
        }
    }
    img->index = data.xml.image_index;
    if (path) {
        add_layer_from_image_path(img, path, 0, 0);
    } else if (group) {
        add_layer_from_image_id(img, group, id, 0, 0);
    }
}

static void xml_start_layer_element(const char **attributes)
{
    if (data.xml.image_index >= XML_MAX_IMAGE_INDEXES) {
        return;
    }
    const char *path = 0;
    const char *group = 0;
    const char *id = 0;
    int offset_x = 0;
    int offset_y = 0;
    modded_image *img = data.xml.current_image;
    if (img->num_layers == MAX_LAYERS) {
        return;
    }
    int total_attributes = count_xml_attributes(attributes);
    if (total_attributes < 2 || total_attributes > 8 || total_attributes % 2) {
        data.xml.error = 1;
        return;
    }
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
    }
    if (path) {
        add_layer_from_image_path(img, path, offset_x, offset_y);
    } else if (group) {
        add_layer_from_image_id(img, group, id, offset_x, offset_y);
    } else {
        log_info("Invalid layer for image", img->id, img->num_layers);
    }
}

static void xml_start_animation_element(const char **attributes)
{
    if (data.xml.image_index >= XML_MAX_IMAGE_INDEXES) {
        return;
    }
    modded_image *img = data.xml.current_image;
    if (img->img.num_animation_sprites) {
        return;
    }
    int total_attributes = count_xml_attributes(attributes);
    if (total_attributes % 2) {
        data.xml.error = 1;
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

static void xml_end_mod_element(void)
{
    data.xml.finished = 1;
}

static void xml_end_image_element(void)
{
    if (data.xml.image_index >= XML_MAX_IMAGE_INDEXES) {
        return;
    }
    image *img = &data.xml.current_image->img;
    img->draw.data_length = img->width * img->height * sizeof(color_t);
    img->draw.uncompressed_length = img->draw.data_length;
    if (!data.xml.current_image->num_layers || !img->draw.data_length) {
        return;
    }
    img->draw.type = IMAGE_TYPE_MOD;
    data.xml.current_image->active = 1;
    if (img->draw.data_length < IMAGE_PRELOAD_MAX_SIZE) {
        load_modded_image(data.xml.current_image);
    }
    data.total_images++;
    data.groups[data.total_groups - 1].images++;
    data.xml.image_index++;
}

static void xml_end_layer_element(void)
{}

static void xml_end_animation_element(void)
{}

static int get_element_index(const char *name)
{
    for (int i = 0; i < XML_MAX_ELEMENTS_PER_DEPTH; ++i) {
        if (XML_FILE_ELEMENTS[data.xml.depth][i][0] == 0) {
            continue;
        }
        if (strcmp(XML_FILE_ELEMENTS[data.xml.depth][i], name) == 0) {
            return i;
        }
    }
    return -1;
}

static void XMLCALL xml_start_element(void *unused, const char *name, const char **attributes)
{
    if (data.xml.error) {
        return;
    }
    if (data.xml.finished || data.xml.depth == XML_MAX_DEPTH) {
        data.xml.error = 1;
        log_error("Invalid XML parameter", name, 0);
        return;
    }
    int index = get_element_index(name);
    if(index == -1) {
        data.xml.error = 1;
        log_error("Invalid XML parameter", name, 0);
        return;
    }
    (*xml_start_element_callback[data.xml.depth][index])(attributes);
    data.xml.depth++;
}

static void XMLCALL xml_end_element(void *unused, const char *name)
{
    if (data.xml.error) {
        return;
    }
    data.xml.depth--;
    int index = get_element_index(name);
    if (index == -1) {
        data.xml.error = 1;
        log_error("Invalid XML parameter", name, 0);
        return;
    }
    (*xml_end_element_callback[data.xml.depth][index])();
}

static const char *append_file_to_mods_folder(const char *file_name)
{
    strncpy(&data.xml.file_name[data.xml.file_name_position], file_name, FILE_NAME_MAX - data.xml.file_name_position - 1);
    return data.xml.file_name;
}

static void clear_xml_info(void)
{
    data.xml.error = 0;
    data.xml.depth = 0;
    data.xml.finished = 0;
    data.xml.image_index = 0;
}

static void process_mod_file(const char *xml_file_name)
{
    xml_file_name = append_file_to_mods_folder(xml_file_name);
    log_info("Loading mod file", xml_file_name, 0);

    FILE *xml_file = file_open(xml_file_name, "r");

    if (!xml_file) {
        log_error("Error opening mod file", xml_file_name, 0);
        return;
    }

    XML_Parser parser = XML_ParserCreate(NULL);
    XML_SetElementHandler(parser, xml_start_element, xml_end_element);

    char buffer[XML_BUFFER_SIZE];
    int done = 0;

    do {
        size_t bytes_read = fread(buffer, 1, XML_BUFFER_SIZE, xml_file);
        done = bytes_read < sizeof(buffer);
        if (XML_Parse(parser, buffer, (int) bytes_read, done) == XML_STATUS_ERROR || data.xml.error) {
            log_error("Error parsing file", xml_file_name, 0);
            break;
        }
    } while (!done);

    if (data.xml.error || !data.xml.finished) {
        data.total_groups--;
        for (int i = 1; i <= data.groups[data.total_groups].images; ++i) {
            modded_image *img = &data.images[data.total_images - i];
            for (int i = 0; i < img->num_layers; ++i) {
                unload_layer(&img->layers[i]);
            }
            if (img->loaded) {
                free(img->data);
            }
            memset(img, 0, sizeof(modded_image));
        }
        data.total_images -= data.groups[data.total_groups].images;
    }

    clear_xml_info();

    XML_ParserFree(parser);
    file_close(xml_file);
}

static void setup_mods_folder_string(void)
{
    size_t mods_folder_length = strlen(MODS_FOLDER);
    strcpy(data.xml.file_name, MODS_FOLDER);
    data.xml.file_name[mods_folder_length] = '/';
    data.xml.file_name_position = mods_folder_length + 1;
    strncpy(data.xml.image_base_path, data.xml.file_name, FILE_NAME_MAX);
}

static int get_image_position_from_id(int image_id)
{
    unsigned int image_group = image_id & 0xffffff00;
    int image_index = image_id & 0xff;
    for (int i = 0; i < data.total_groups; ++i) {
        image_groups *group = &data.groups[i];
        if (group->id == image_group) {
            for (int j = 0; j < group->images; ++j) {
                if (data.images[group->first_image_id + j].index == image_index) {
                    return group->first_image_id + j;
                }
            }
        }
    }
    return image_id != data.roadblock_image ? data.roadblock_image : 0;
}

void mods_init(void)
{
    if (data.loaded) {
        return;
    }

    setup_mods_folder_string();

    const dir_listing *xml_files = dir_find_files_with_extension(MODS_FOLDER, "xml");

    for (int i = 0; i < xml_files->num_files; ++i) {
        process_mod_file(xml_files->files[i]);
    }

    data.loaded = 1;

    // By default, if the requested image is not found, the roadblock image will be shown.
    // This ensures compatibility with previous release versions of Augustus, which only had roadblocks
    data.roadblock_image = get_image_position_from_id(mods_get_group_id("Keriew", "Roadblocks"));
}

int mods_get_group_id(const char *mod_author, const char *mod_name)
{
    int id = get_group_hash(mod_author, mod_name);
    for (int i = 0; i < data.total_groups; ++i) {
        image_groups *group = &data.groups[i];
        if (id == group->id) {
            return id;
        }
    }
    return 0;
}

int mods_get_image_id(int mod_group_id, const char *image_name)
{
    if (!image_name || !*image_name) {
        return 0;
    }
    int max_images = 0;
    int image_id = 0;
    for (int i = 0; i < data.total_groups; ++i) {
        image_groups *group = &data.groups[i];
        if (group->id == mod_group_id) {
            max_images = group->images;
            image_id = group->first_image_id;
            break;
        }
    }
    for (int i = 0; i < max_images; ++i) {
        if (strcmp(data.images[image_id + i].id, image_name) == 0) {
            return mod_group_id + data.images[image_id + i].index;
        }
    }
    return 0;
}

const image *mods_get_image(int image_id)
{
    modded_image *img = &data.images[get_image_position_from_id(image_id)];
    if (!img->active) {
        return image_get(0);
    }
    return &img->img;
}

const color_t *mods_get_image_data(int image_id)
{
    modded_image *img = &data.images[get_image_position_from_id(image_id)];
    if (!img->active) {
        return image_data(0);
    }
    if (!img->loaded) {
        if (!load_modded_image(img)) {
            return image_data(0);
        }
    }
    return img->data;
}

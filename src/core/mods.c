#include "mods.h"

#include "core/dir.h"
#include "core/file.h"
#include "core/log.h"
#include "core/string.h"
#include "graphics/color.h"
#include "graphics/graphics.h"

#include "expat.h"

#include <string.h>

#define XML_BUFFER_SIZE 1024
#define XML_MAX_DEPTH 3
#define XML_MAX_ATTRIBUTES 5
#define XML_TAG_MAX_LENGTH 10
#define XML_STRING_MAX_LENGTH 32

#define MAX_LAYERS 5
#define MAX_GROUPS 100

#define IMAGE_PRELOAD_MAX_SIZE 65535

#define MIX_RB(src, dst, alpha_src, alpha_dst) ((((src & 0xff00ff) * alpha_src + (dst & 0xff00ff) * (256 - alpha_src) * alpha_dst) >> 8) & 0xff00ff)
#define MIX_G(src, dst, alpha_src, alpha_dst) ((((src & 0x00ff00) * alpha_src + (dst & 0x00ff00) * (256 - alpha_src) * alpha_dst) >> 8) & 0x00ff00)
#define MIX_A(alpha_src, alpha_dst) (alpha_src + alpha_dst * (256 - alpha_src))

static const char *MODS_FOLDER = "mods";
static const char XML_FILE_ELEMENTS[XML_MAX_DEPTH][XML_TAG_MAX_LENGTH] = { "mod", "image", "layer" };
static const char XML_FILE_ATTRIBUTES[XML_MAX_DEPTH][XML_MAX_ATTRIBUTES][XML_TAG_MAX_LENGTH] = {
    { "author", "name" }, // mod
    { "id", "src", "width", "height" }, // image
    { "src", "group", "image", "x", "y" } // layer
};
static const color_t DUMMY_IMAGE_DATA = COLOR_BLACK;

static void xml_start_mod_element(const char **attributes);
static void xml_start_image_element(const char **attributes);
static void xml_start_layer_element(const char **attributes);
static void xml_end_mod_element(void);
static void xml_end_image_element(void);
static void xml_end_layer_element(void);

static const void (*xml_start_element_callback[XML_MAX_DEPTH])(const char **attributes) = {
    xml_start_mod_element, xml_start_image_element, xml_start_layer_element
};

static const void (*xml_end_element_callback[XML_MAX_DEPTH])(void) = {
    xml_end_mod_element, xml_end_image_element, xml_end_layer_element
};

typedef struct {
    char *modded_image_path;
    int original_image_id;
    int x_offset;
    int y_offset;
    int width;
    int height;
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
} modded_image;

typedef struct {
    char author[XML_STRING_MAX_LENGTH];
    char name[XML_STRING_MAX_LENGTH];
    int id;
    int images;
} image_groups;

static struct {
    struct {
        char file_name[FILE_NAME_MAX];
        size_t file_name_position;
        char image_base_path[FILE_NAME_MAX];
        size_t image_base_path_position;
        int depth;
        int error;
        int finished;
        modded_image *current_image;
    } xml;
    image_groups groups[MAX_GROUPS];
    modded_image images[MAX_MODDED_IMAGES];
    int total_groups;
    int total_images;
} data;

static void load_layer(layer *l)
{

}

static color_t layer_get_color_for_image_position(layer *l, int x, int y)
{
    return COLOR_BLACK;
}

static void load_dummy_image(modded_image *img)
{
    img->active = 1;
    img->loaded = 1;
    img->img.width = 1;
    img->img.height = 1;
    img->img.draw.data_length = sizeof(color_t);
    img->img.draw.uncompressed_length = sizeof(color_t);
    img->img.draw.type = IMAGE_TYPE_MOD;
    img->data = &DUMMY_IMAGE_DATA;
}

static void load_modded_image(modded_image *img)
{
    if (img->loaded) {
        return;
    }
    img->data = malloc(img->img.draw.data_length);
    memset(img->data, 0, img->img.draw.data_length);
    if (!img->data) {
        log_error("Not enough memory to load image", img->id, 0);
        load_dummy_image(img);
        return;
    }
    canvas_type type = graphics_get_canvas_type();
    graphics_set_custom_canvas(img->data, img->img.width, img->img.height);

    for (int i = 0; i < img->num_layers; ++i) {
        load_layer(&img->layers[i]);
    }

    for (int y = 0; y < img->img.height; ++y) {
        color_t *pixel = graphics_get_pixel(0, y);
        for (int x = 0; x < img->img.width; ++x) {
            for (int l = img->num_layers - 1; l >= 0; --l) {
                int image_pixel_alpha = *pixel & ALPHA_OPAQUE;
                if (image_pixel_alpha == ALPHA_OPAQUE) {
                    break;
                }
                color_t layer_pixel = COLOR_BLUE;// layer_get_color_for_image_position(&img->layers[l], x, y);
                int layer_pixel_alpha = layer_pixel & ALPHA_OPAQUE;
                if (layer_pixel_alpha == ALPHA_TRANSPARENT) {
                    continue;
                }
                if (layer_pixel_alpha == ALPHA_OPAQUE && image_pixel_alpha == ALPHA_TRANSPARENT) {
                    *pixel = layer_pixel;
                } if (layer_pixel_alpha == ALPHA_OPAQUE) {
                    int alpha = image_pixel_alpha >> 24;
                    *pixel = MIX_RB(layer_pixel, *pixel, alpha, 1) | MIX_G(layer_pixel, *pixel, alpha, 1) | ALPHA_OPAQUE;
                } else {
                    int alpha_src = layer_pixel_alpha >> 24;
                    int alpha_dst = image_pixel_alpha >> 24;
                    *pixel = MIX_RB(layer_pixel, *pixel, alpha_src, alpha_dst) | MIX_G(layer_pixel, *pixel, alpha_src, alpha_dst);
                    *pixel |= MIX_A(alpha_src, alpha_dst) << 24;
                }
            }
            ++pixel;
        }
    }

    for (int i = 0; i < img->num_layers; ++i) {
      //  unload_layer(&img->layers[i]);
    }

    graphics_set_active_canvas(type);
    img->img.draw.type = IMAGE_TYPE_MOD;
    img->loaded = 1;
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

static const char *get_full_image_path(const char *path)
{
    strncpy(data.xml.file_name + data.xml.file_name_position, path, FILE_NAME_MAX - data.xml.file_name_position - 1);
    return data.xml.file_name;
}

static int add_layer_from_image_path(modded_image *img, const char *path, int offset_x, int offset_y)
{
    layer *current_layer = &img->layers[img->num_layers];
    size_t path_size = strlen(path);
    current_layer->modded_image_path = malloc((path_size + 5) * sizeof(char));
    strncpy(current_layer->modded_image_path, path, path_size);
    strncpy(current_layer->modded_image_path + path_size, ".png", 5);

    if (!img->img.width || !img->img.height) {
        int width = 0;
        int height = 0;
     //   if (!png_get_image_size(get_full_image_path(current_layer->modded_image_path), &width, &height)) {
     //       log_info("Unable to load image", path, 0);
     //       return 0;
     //   }
        if (!img->img.width) {
            img->img.width = width;
        }
        if (!img->img.height) {
            img->img.height = height;
        }
    }
    current_layer->x_offset = offset_x;
    current_layer->y_offset = offset_y;
    img->num_layers++;
    return 1;
}

static int add_layer_from_image_id(modded_image *img, const char *group_id, const char *image_id, int offset_x, int offset_y)
{
    layer *current_layer = &img->layers[img->num_layers];
    const image *original_image = 0;
    if (strcmp(group_id, "this") == 0) {
        current_layer->original_image_id = mods_get_image_id(data.groups[data.total_groups].id, image_id);
        if (!current_layer->original_image_id) {
            return 0;
        }
    } else {
        int group = string_to_int(string_from_ascii(group_id));
        int id = string_to_int(string_from_ascii(image_id));
        current_layer->original_image_id = image_group(group) + id;
    }
    original_image = image_get(current_layer->original_image_id);
    if (!original_image) {
        return 0;
    }
    if (!img->img.width) {
        img->img.width = original_image->width;
    }
    if (!img->img.height) {
        img->img.height = original_image->height;
    }
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

static void xml_start_mod_element(const char **attributes)
{
    if (count_xml_attributes(attributes) != 4) {
        data.xml.error = 1;
        return;
    }
    image_groups *group = &data.groups[data.total_groups];
    group->id = data.total_images + MAIN_ENTRIES;
    group->images = 0;
    for (int i = 0; i < 4; i += 2) {
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[0][0]) == 0) {
            strncpy(group->author, attributes[i + 1], XML_STRING_MAX_LENGTH - 1);
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[0][1]) == 0) {
            strncpy(group->name, attributes[i + 1], XML_STRING_MAX_LENGTH - 1);
        }
    }
    if (*group->author == '\0' || *group->name == '\0') {
        data.xml.error = 1;
        return;
    }
    set_modded_image_base_path(group->author, group->name);
}

static void xml_start_image_element(const char **attributes)
{
    data.xml.current_image = &data.images[data.total_images];
    modded_image *img = data.xml.current_image;
    int total_attributes = count_xml_attributes(attributes);
    if (total_attributes < 2 || total_attributes > 8 || total_attributes % 2) {
        data.xml.error = 1;
        return;
    }
    for (int i = 0; i < total_attributes; i += 2) {
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[1][0]) == 0) {
            strncpy(img->id, attributes[i + 1], XML_STRING_MAX_LENGTH - 1);
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[1][1]) == 0) {
            add_layer_from_image_path(img, attributes[i + 1], 0, 0);
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[1][2]) == 0) {
            img->img.width = string_to_int(string_from_ascii(attributes[i + 1]));
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[1][3]) == 0) {
            img->img.height = string_to_int(string_from_ascii(attributes[i + 1]));
        }
    }
    if (img->id == '\0') {
        data.xml.error = 1;
    }
}

static void xml_start_layer_element(const char **attributes)
{
    const char *path = 0;
    const char *group = 0;
    const char *id = 0;
    int offset_x = 0;
    int offset_y = 0;
    modded_image *img = data.xml.current_image;
    int total_attributes = count_xml_attributes(attributes);
    if (total_attributes < 2 && total_attributes > 8 || total_attributes % 2) {
        data.xml.error = 1;
        return;
    }
    for (int i = 0; i < total_attributes; i += 2) {
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][0]) == 0) {
            path = attributes[i + 1];
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][1]) == 0) {
            group = attributes[i + 1];
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][2]) == 0) {
            id = attributes[i + 1];
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][3]) == 0) {
            offset_x = string_to_int(string_from_ascii(attributes[i + 1]));
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][4]) == 0) {
            offset_y = string_to_int(string_from_ascii(attributes[i + 1]));
        }
    }
    if (path) {
        add_layer_from_image_path(img, path, offset_x, offset_y);
    } else if (group && id) {
        add_layer_from_image_id(img, group, id, offset_x, offset_y);
    } else {
        log_info("Invalid layer for image", img->id, img->num_layers);
    }
}

static void xml_end_mod_element(void)
{
    data.total_groups++;
    data.xml.finished = 1;
}

static void xml_end_image_element(void)
{
    image *img = &data.xml.current_image->img;
    img->draw.data_length = img->width * img->height * sizeof(color_t);
    img->draw.uncompressed_length = img->draw.data_length;
    if (!data.xml.current_image->num_layers || !img->draw.data_length) {
        return;
    }
    data.xml.current_image->active = 1;
    if (img->draw.data_length < IMAGE_PRELOAD_MAX_SIZE) {
        load_modded_image(data.xml.current_image);
    }
    data.total_images++;
    data.groups[data.total_groups].images++;
}

static void xml_end_layer_element(void)
{}

static void XMLCALL xml_start_element(void *unused, const char *name, const char **attributes)
{
    if (data.xml.error) {
        return;
    }
    if (data.xml.finished || data.xml.depth == XML_MAX_DEPTH ||
        strcmp(XML_FILE_ELEMENTS[data.xml.depth], name) != 0) {
        data.xml.error = 1;
        log_error("Invalid XML parameter", name, 0);
        return;
    }
    (*xml_start_element_callback[data.xml.depth])(attributes);
    data.xml.depth++;
}

static void XMLCALL xml_end_element(void *unused, const char *name)
{
    if (data.xml.error) {
        return;
    }
    data.xml.depth--;
    if (strcmp(XML_FILE_ELEMENTS[data.xml.depth], name) != 0) {
        data.xml.error = 1;
        log_error("Invalid XML parameter", name, 0);
        return;
    }
    (*xml_end_element_callback[data.xml.depth])();
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
}

static void process_mod_file(const char *xml_file_name)
{
    xml_file_name = append_file_to_mods_folder(xml_file_name);
    log_info("Loading mod file", xml_file_name, 0);

    FILE *xml_file = file_open(xml_file_name, "r");

    if (!xml_file) {
        log_error("Error opening mod file", 0, 0);
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
            log_error("Error parsing file", 0, 0);
            break;
        }
    } while (!done);

    if (data.xml.error || !data.xml.finished) {
        for (int i = 1; i <= data.groups[data.total_groups].images; ++i) {
            modded_image *img = &data.images[data.total_images - i];
            if (img->loaded) {
                free(img->data);
            }
            memset(img, 0, sizeof(modded_image));
        }
        data.total_images -= data.groups[data.total_groups].images;
    }

    XML_ParserFree(parser);
    file_close(xml_file);
}

static void setup_mods_folder_string(void)
{
    size_t mods_folder_length = strlen(MODS_FOLDER);
    strncpy(data.xml.file_name, MODS_FOLDER, FILE_NAME_MAX - 1);
    data.xml.file_name[mods_folder_length] = '/';
    data.xml.file_name_position = mods_folder_length + 1;
    strncpy(data.xml.image_base_path, data.xml.file_name, FILE_NAME_MAX - 1);
}

void mods_init(void)
{
    setup_mods_folder_string();

    const dir_listing *xml_files = dir_find_files_with_extension(MODS_FOLDER, "xml");

    for (int i = 0; i < xml_files->num_files; ++i) {
        process_mod_file(xml_files->files[i]);
    }
}

int mods_get_group_id(const char *mod_author, const char *mod_name)
{
    for (int i = 0; i < data.total_groups; ++i) {
        image_groups *group = &data.groups[i];
        if (strcmp(group->author, mod_author) == 0 &&
            strcmp(group->name, mod_name) == 0) {
            return group->id;
        }
    }
    return 0;
}

int mods_get_image_id(int mod_group_id, const char *image_name)
{
    int max_images = 0;
    for (int i = 0; i < data.total_groups; ++i) {
        image_groups *group = &data.groups[i];
        if (group->id == mod_group_id) {
            max_images = group->images;
            break;
        }
    }
    mod_group_id -= MAIN_ENTRIES;
    for (int i = 0; i < max_images; ++i) {
        if (strcmp(data.images[mod_group_id].id, image_name) == 0) {
            return mod_group_id + MAIN_ENTRIES;
        }
        mod_group_id++;
    }
    return 0;
}

const image *mods_get_image(int image_id)
{
    modded_image *img = &data.images[image_id - MAIN_ENTRIES];
    if (!img->active) {
        return 0;
    }
    return &img->img;
}

const color_t *mods_get_image_data(int image_id)
{
    modded_image *img = &data.images[image_id - MAIN_ENTRIES];
    if (!img->active) {
        return 0;
    }
    if (!img->loaded) {
        load_modded_image(img);
    }
    return img->data;
}

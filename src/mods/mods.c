#include "mods.h"

#include "core/dir.h"
#include "core/log.h"
#include "mods/group.h"
#include "mods/image.h"
#include "mods/xml.h"

#include <stdlib.h>
#include <string.h>

static const char *MODS_FOLDER = "mods";

static struct {
    int loaded;
    modded_image *roadblock_image;
} data;

void mods_init(void)
{
    if (data.loaded) {
        return;
    }

    xml_setup_base_folder_string(MODS_FOLDER);

    const dir_listing *xml_files = dir_find_files_with_extension(MODS_FOLDER, "xml");

    if (!group_create_all(xml_files->num_files)) {
        log_error("Not enough memory to initialize mods. The game will probably crash.", 0, 0);
    }

    for (int i = 0; i < xml_files->num_files; ++i) {
        xml_process_mod_file(xml_files->files[i]);
    }

    // By default, if the requested image is not found, the roadblock image will be shown.
    // This ensures compatibility with previous release versions of Augustus, which only had roadblocks
    data.roadblock_image = modded_image_get_from_id(mods_get_group_id("Areldir", "Roadblocks"));
    data.loaded = 1;
}

int mods_get_group_id(const char *mod_author, const char *mod_name)
{
    return group_get_hash(mod_author, mod_name);
}

int mods_get_image_id(int mod_group_id, const char *image_name)
{
    if (!image_name || !*image_name) {
        return 0;
    }
    image_groups *group = group_get_from_hash(mod_group_id);
    if (!group) {
        return 0;
    }
    for (modded_image *img = group->first_image; img; img = img->next) {
        if (strcmp(img->id, image_name) == 0) {
            return mod_group_id + img->index;
        }
    }
    return 0;
}

const image *mods_get_image(int image_id)
{
    modded_image *img = modded_image_get_from_id(image_id);
    if (!img) {
        img = data.roadblock_image;
    }
    if (!img || !img->active) {
        return image_get(0);
    }
    return &img->img;
}

const color_t *mods_get_image_data(int image_id)
{
    modded_image *img = modded_image_get_from_id(image_id);
    if (!img) {
        img = data.roadblock_image;
    }
    if (!img || !img->active) {
        return image_data(0);
    }
    if (!img->loaded) {
        if (!modded_image_load(img)) {
            return image_data(0);
        }
    }
    return img->data;
}

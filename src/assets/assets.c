#include "assets.h"

#include "assets/group.h"
#include "assets/image.h"
#include "assets/xml.h"
#include "core/dir.h"
#include "core/log.h"

#include <stdlib.h>
#include <string.h>

static struct {
    int loaded;
    int roadblock_image_id;
    asset_image *roadblock_image;
} data;

void assets_init(void)
{
    if (data.loaded) {
        return;
    }

    asset_image_init_array();

    const dir_listing *xml_files = dir_find_files_with_extension(ASSETS_DIRECTORY, "xml");

    if (!group_create_all(xml_files->num_files) || !asset_image_init_array()) {
        log_error("Not enough memory to initialize extra assets. The game will probably crash.", 0, 0);
    }

    for (int i = 0; i < xml_files->num_files; ++i) {
        xml_process_assetlist_file(xml_files->files[i]);
    }

    // By default, if the requested image is not found, the roadblock image will be shown.
    // This ensures compatibility with previous release versions of Augustus, which only had roadblocks
    data.roadblock_image_id = assets_get_group_id("Roadblocks");
    data.roadblock_image = asset_image_get_from_id(data.roadblock_image_id - MAIN_ENTRIES);
    data.loaded = 1;
}

int assets_get_group_id(const char *assetlist_name)
{
    image_groups *group = group_get_from_name(assetlist_name);
    if (group) {
        return group->first_image_index + MAIN_ENTRIES;
    }
    log_info("Asset group not found: ", assetlist_name, 0);
    return data.roadblock_image_id;
}

int assets_get_image_id(const char *assetlist_name, const char *image_name)
{
    if (!image_name || !*image_name) {
        return data.roadblock_image_id;
    }
    image_groups *group = group_get_from_name(assetlist_name);
    if (!group) {
        log_info("Asset group not found: ", assetlist_name, 0);
        return data.roadblock_image_id;
    }
    const asset_image *image = asset_image_get_from_id(group->first_image_index);
    while (image && image->index <= group->last_image_index) {
        if (strcmp(image->id, image_name) == 0) {
            return image->index + MAIN_ENTRIES;
        }
        image = asset_image_get_from_id(image->index + 1);
    }
    log_info("Asset image not found: ", image_name, 0);
    log_info("Asset group is: ", assetlist_name, 0);
    return data.roadblock_image_id;
}

const image *assets_get_image(int image_id)
{
    asset_image *img = asset_image_get_from_id(image_id - MAIN_ENTRIES);
    if (!img) {
        img = data.roadblock_image;
    }
    if (!img || (img->loaded && !img->data)) {
        return image_get(0);
    }
    return &img->img;
}

const color_t *assets_get_image_data(int image_id)
{
    asset_image *img = asset_image_get_from_id(image_id - MAIN_ENTRIES);
    if (!img) {
        img = data.roadblock_image;
    }
    if (!img || !asset_image_load(img)) {
        return image_data(0);
    }
    return img->data;
}

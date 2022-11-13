#include "assets.h"

#include "assets/group.h"
#include "assets/image.h"
#include "assets/xml.h"
#include "core/dir.h"
#include "core/log.h"
#include "graphics/renderer.h"

#include <stdlib.h>
#include <string.h>

static struct {
    int roadblock_image_id;
    asset_image *roadblock_image;
    int asset_lookup[ASSET_MAX_KEY];
} data;

void assets_init(int force_reload, color_t **main_images, int *main_image_widths)
{
    if (graphics_renderer()->has_image_atlas(ATLAS_EXTRA_ASSET) && !force_reload) {
        asset_image_reload_climate();
        return;
    }

    graphics_renderer()->free_image_atlas(ATLAS_EXTRA_ASSET);

    const dir_listing *xml_files = dir_find_files_with_extension(ASSETS_DIRECTORY, "xml");

    if (!group_create_all(xml_files->num_files) || !asset_image_init_array()) {
        log_error("Not enough memory to initialize extra assets. The game will probably crash.", 0, 0);
    }

    xml_init();

    for (int i = 0; i < xml_files->num_files; ++i) {
        xml_process_assetlist_file(xml_files->files[i]);
    }

    xml_finish();

    asset_image_load_all(main_images, main_image_widths);

    // By default, if the requested image is not found, the roadblock image will be shown.
    // This ensures compatibility with previous release versions of Augustus, which only had roadblocks
    data.roadblock_image_id = assets_get_group_id("Logistics");
    data.roadblock_image = asset_image_get_from_id(data.roadblock_image_id - IMAGE_MAIN_ENTRIES);
    data.asset_lookup[ASSET_HIGHWAY_BASE_START] = assets_get_image_id("Logistics", "Highway_Base_Start");
    data.asset_lookup[ASSET_HIGHWAY_BARRIER_START] = assets_get_image_id("Logistics", "Highway_Barrier_Start");
    data.asset_lookup[ASSET_AQUEDUCT_WITH_WATER] = assets_get_image_id("Logistics", "Aqueduct_Bridge_Left_Water");
    data.asset_lookup[ASSET_AQUEDUCT_WITHOUT_WATER] = assets_get_image_id("Logistics", "Aqueduct_Bridge_Left_Empty");
    data.asset_lookup[ASSET_GOLD_SHIELD] = assets_get_image_id("UI", "GoldShield");
}

int assets_load_single_group(const char *file_name, color_t **main_images, int *main_image_widths)
{
    if (!group_create_all(1) || !asset_image_init_array()) {
        log_error("Not enough memory to initialize extra assets. The game will probably crash.", 0, 0);
        return 0;
    }
    graphics_renderer()->free_image_atlas(ATLAS_EXTRA_ASSET);
    return xml_process_assetlist_file(file_name) && asset_image_load_all(main_images, main_image_widths);
}

int assets_get_group_id(const char *assetlist_name)
{
    image_groups *group = group_get_from_name(assetlist_name);
    if (group) {
        return group->first_image_index + IMAGE_MAIN_ENTRIES;
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
        if (image->id && strcmp(image->id, image_name) == 0) {
            return image->index + IMAGE_MAIN_ENTRIES;
        }
        image = asset_image_get_from_id(image->index + 1);
    }
    log_info("Asset image not found: ", image_name, 0);
    log_info("Asset group is: ", assetlist_name, 0);
    return data.roadblock_image_id;
}

int assets_lookup_image_id(asset_id id)
{
    return data.asset_lookup[id];
}

const image *assets_get_image(int image_id)
{
    asset_image *img = asset_image_get_from_id(image_id - IMAGE_MAIN_ENTRIES);
    if (!img) {
        img = data.roadblock_image;
    }
    if (!img) {
        return image_get(0);
    }
    return &img->img;
}

void assets_load_unpacked_asset(int image_id)
{
    asset_image *img = asset_image_get_from_id(image_id - IMAGE_MAIN_ENTRIES);
    if (!img) {
        return;
    }
    const color_t *data;
    if (img->is_reference) {
        asset_image *referenced_asset =
            asset_image_get_from_id(img->first_layer.calculated_image_id - IMAGE_MAIN_ENTRIES);
        data = referenced_asset->data;
    } else {
        data = img->data;
    }
    graphics_renderer()->load_unpacked_image(&img->img, data);
}

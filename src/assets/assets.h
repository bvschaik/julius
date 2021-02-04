#ifndef ASSETS_H
#define ASSETS_H

#include "core/image.h"

#define ASSETS_DIR_NAME "assets"
#define ASSETS_DIRECTORY "***" ASSETS_DIR_NAME "***"

#define PATH_ROTATE_OFFSET 56

void assets_init(void);

int assets_get_group_id(const char *assetlist_author, const char *asset_name);

int assets_get_image_id(int asset_group_id, const char *image_name);

const image *assets_get_image(int image_id);

const color_t *assets_get_image_data(int image_id);

#endif // ASSETS_H

#ifndef CORE_MODS_H
#define CORE_MODS_H

#include "core/image.h"

void mods_init(void);

int mods_get_group_id(const char *mod_author, const char *mod_name);

int mods_get_image_id(int mod_group_id, const char *image_name);

const image *mods_get_image(int image_id);

const color_t *mods_get_image_data(int image_id);

#endif // CORE_MODS_H

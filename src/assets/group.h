#ifndef ASSETS_GROUP_H
#define ASSETS_GROUP_H

#include "assets/image.h"
#include "assets/xml.h"

#include <stdint.h>

typedef struct {
    const char *name;
#ifdef BUILDING_ASSET_PACKER
    const char *path;
#endif
    int first_image_index;
    int last_image_index;
} image_groups;

int group_create_all(int total);

image_groups *group_get_new(void);
void group_unload_current(void);

image_groups *group_get_current(void);

int group_get_total(void);

image_groups *group_get_from_id(int id);
image_groups *group_get_from_name(const char *name);
image_groups *group_get_from_image_index(int index);

#endif // ASSETS_GROUP_H

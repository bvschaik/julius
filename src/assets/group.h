#ifndef ASSETS_GROUP_H
#define ASSETS_GROUP_H

#include "assets/image.h"
#include "assets/xml.h"

#include <stdint.h>

typedef struct {
    char author[XML_STRING_MAX_LENGTH];
    char name[XML_STRING_MAX_LENGTH];
    int id;
    asset_image *first_image;
} image_groups;

int group_create_all(int total);

image_groups *group_get_new(void);
void group_unload_current(void);

image_groups *group_get_current(void);

uint32_t group_get_hash(const char *author, const char *name);
image_groups *group_get_from_hash(uint32_t hash);

#endif // ASSETS_GROUP_H

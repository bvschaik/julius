#ifndef MODS_GROUP_H
#define MODS_GROUP_H

#include "mods/xml.h"
#include "mods/image.h"

#include <stdint.h>

typedef struct {
    char author[XML_STRING_MAX_LENGTH];
    char name[XML_STRING_MAX_LENGTH];
    int id;
    modded_image *first_image;
} image_groups;

int group_create_all(int total);

image_groups *group_get_new(void);
void group_unload_current(void);

image_groups *group_get_current(void);

uint32_t group_get_hash(const char *author, const char *name);
image_groups *group_get_from_hash(uint32_t hash);

#endif // MODS_GROUP_H

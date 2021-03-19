#include "group.h"

#include "PMurHash/PMurHash.h"

#include <stdlib.h>
#include <string.h>

#define GROUP_HASH_MASK 0xffffff00
#define GROUP_MIN_HASH 0x4000

typedef struct {
    uint32_t original;
    uint32_t replacement;
} hash_replacement;

static struct {
    int total_groups;
    image_groups *groups;
    hash_replacement engineer_to_architect;
} data;

void group_setup_hash_replacements(void)
{
    data.engineer_to_architect.original = group_get_hash("Areldir", "Engineer");
    data.engineer_to_architect.replacement = group_get_hash("Areldir", "Architect");
}

int group_create_all(int total)
{
    data.groups = malloc(sizeof(image_groups) * total);
    if (!data.groups) {
        return 0;
    }
    memset(data.groups, 0, sizeof(image_groups) * total);
    return 1;
}

image_groups *group_get_new(void)
{
    return &data.groups[data.total_groups++];
}

image_groups *group_get_current(void)
{
    return &data.groups[data.total_groups - 1];
}

uint32_t group_get_hash(const char *author, const char *name)
{
    uint32_t hash = HASH_SEED;
    uint32_t carry = 0;
    uint32_t author_length = (uint32_t) strlen(author);
    uint32_t name_length = (uint32_t) strlen(name);

    PMurHash32_Process(&hash, &carry, author, author_length);
    PMurHash32_Process(&hash, &carry, name, name_length);

    hash = PMurHash32_Result(hash, carry, author_length + name_length);
    
    // The following code increases the risk of hash collision but allows better image indexing
    if (hash < GROUP_MIN_HASH) {
        hash |= GROUP_MIN_HASH;
    }
    return hash & GROUP_HASH_MASK;
}

image_groups *group_get_from_hash(uint32_t hash)
{
    hash &= GROUP_HASH_MASK;
    if (hash == data.engineer_to_architect.original) {
        hash = data.engineer_to_architect.replacement;
    }
    for (int i = 0; i < data.total_groups; ++i) {
        image_groups *group = &data.groups[i];
        if (hash == group->id) {
            return group;
        }
    }
    return 0;
}

void group_unload_current(void)
{
    image_groups *group = group_get_current();
    asset_image *img = group->first_image;
    memset(group, 0, sizeof(image_groups));
    while (img) {
        asset_image *next = img->next;
        asset_image_unload(img);
        free(img);
        img = next;
    }
    data.total_groups--;
}

#include "group.h"

#include <stdlib.h>
#include <string.h>

static struct {
    int total_groups;
    image_groups *groups;
} data;

int group_create_all(int total)
{
    data.groups = malloc(sizeof(image_groups) * total);
    if (!data.groups) {
        return 0;
    }
    memset(data.groups, 0, sizeof(image_groups) * total);
    data.total_groups = 0;
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

void group_unload_current(void)
{
    image_groups *group = group_get_current();
    asset_image *image = asset_image_get_from_id(group->last_image_index);
    while (image && image->index >= group->first_image_index) {
        asset_image_unload(image);
        image = asset_image_get_from_id(image->index - 1);
    }
    memset(group, 0, sizeof(image_groups));
    data.total_groups--;
}

image_groups *group_get_from_id(int id)
{
    for (int i = 0; i < data.total_groups; i++) {
        image_groups *current = &data.groups[i];
        if (current->first_image_index >= id && current->last_image_index <= id) {
            return current;
        }
    }
    return 0;
}

image_groups *group_get_from_name(const char *name)
{
    if (!name || !*name) {
        return 0;
    }
    for (int i = 0; i < data.total_groups; i++) {
        image_groups *current = &data.groups[i];
        if (strcmp(current->name, name) == 0) {
            return current;
        }
    }
    return 0;
}

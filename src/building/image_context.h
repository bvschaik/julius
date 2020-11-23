#ifndef BUILDING_IMAGE_CONTEXT_H
#define BUILDING_IMAGE_CONTEXT_H

typedef struct {
    int is_valid;
    int group_offset;
    int item_offset;
} building_image;

void building_image_context_init(void);

void building_image_context_set_hedge_image(int grid_offset);



#endif // BUILDING_IMAGE_CONTEXT_H


#include "image_packer.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#define TOLERABLE_AREA_DIFFERENCE_PERCENTAGE 2

typedef struct empty_area {
    unsigned int x, y;
    unsigned int width, height;
    unsigned int comparator;
    struct empty_area *prev, *next;
} empty_area;

typedef struct {
    image_packer_rect **sorted_rects;
    unsigned int num_rects;
    unsigned int image_width;
    unsigned int image_height;
    struct {
        struct empty_area *first;
        struct empty_area *last;
        struct empty_area *list;
        int index;
        int size;
        void (*set_comparator)(empty_area *area);
    } empty_areas;
} internal_data;

static int compare_rect_perimeters(const void *a, const void *b)
{
    // Since we're just comparing perimeters and don't actually need the values,
    // we don't need to actually calculate them. Check documentation for qsort to see how this works
    return (*(image_packer_rect **) b)->input.height + (*(image_packer_rect **) b)->input.width -
        (*(image_packer_rect **) a)->input.height - (*(image_packer_rect **) a)->input.width;
}

static int compare_rect_areas(const void *a, const void *b)
{
    return (*(image_packer_rect **) b)->input.height * (*(image_packer_rect **) b)->input.width -
        (*(image_packer_rect **) a)->input.height * (*(image_packer_rect **) a)->input.width;
}

static int compare_rect_heights(const void *a, const void *b)
{
    return (*(image_packer_rect **) b)->input.height - (*(image_packer_rect **) a)->input.height;
}

static int compare_rect_widths(const void *a, const void *b)
{
    return (*(image_packer_rect **) b)->input.width - (*(image_packer_rect **) a)->input.width;
}

static void set_perimeter_comparator(empty_area *area)
{
    area->comparator = area->height + area->width;
}

static void set_area_comparator(empty_area *area)
{
    area->comparator = area->height * area->width;
}

static void set_width_comparator(empty_area *area)
{
    area->comparator = area->width;
}

static void set_height_comparator(empty_area *area)
{
    area->comparator = area->height;
}

static int sort_rects(image_packer *packer)
{
    internal_data *data = packer->internal_data;
    data->sorted_rects = (image_packer_rect **) malloc(data->num_rects * sizeof(image_packer_rect *));

    if (!data->sorted_rects) {
        return 0;
    }
    for (unsigned int i = 0; i < data->num_rects; i++) {
        data->sorted_rects[i] = &packer->rects[i];
    }

    if (packer->options.rects_are_sorted != 1) {
        int (*sort_by)(const void *, const void *);
        switch (packer->options.sort_by) {
            case IMAGE_PACKER_SORT_BY_AREA:
                sort_by = compare_rect_areas;
                data->empty_areas.set_comparator = set_area_comparator;
                break;
            case IMAGE_PACKER_SORT_BY_HEIGHT:
                sort_by = compare_rect_heights;
                data->empty_areas.set_comparator = set_height_comparator;
                break;
            case IMAGE_PACKER_SORT_BY_WIDTH:
                sort_by = compare_rect_widths;
                data->empty_areas.set_comparator = set_width_comparator;
                break;
            case IMAGE_PACKER_SORT_BY_PERIMETER:
            default:
                sort_by = compare_rect_perimeters;
                data->empty_areas.set_comparator = set_perimeter_comparator;
                break;
        }
        qsort(data->sorted_rects, data->num_rects, sizeof(image_packer_rect *), sort_by);
        packer->options.rects_are_sorted = 1;
    }
    return 1;
}

static void reset_empty_areas(internal_data *data, unsigned int width, unsigned int height)
{
    memset(data->empty_areas.list, 0, sizeof(empty_area) * data->empty_areas.size);

    data->empty_areas.index = 0;

    // The first empty space is always the entire area of the image
    data->empty_areas.first = &data->empty_areas.list[0];
    data->empty_areas.last = &data->empty_areas.list[0];
    data->empty_areas.first->width = width;
    data->empty_areas.first->height = height;
}

static void sort_empty_area(internal_data *data, empty_area *area, empty_area *current)
{
    if (!data->empty_areas.first) {
        data->empty_areas.first = area;
        data->empty_areas.last = area;
        return;
    }
    while (current) {
        if (current->comparator < area->comparator) {
            if (current == data->empty_areas.last) {
                data->empty_areas.last = area;
            } else {
                area->next = current->next;
                area->next->prev = area;
            }
            area->prev = current;
            current->next = area;
            return;
        }
        current = current->prev;
    }
    data->empty_areas.first->prev = area;
    area->next = data->empty_areas.first;
    data->empty_areas.first = area;
}

static void delist_empty_area(internal_data *data, empty_area *area)
{
    if (area == data->empty_areas.first) {
        data->empty_areas.first = area->next;
    }
    if (area == data->empty_areas.last) {
        data->empty_areas.last = area->prev;
    }
    if (area->prev) {
        area->prev->next = area->next;
    }
    if (area->next) {
        area->next->prev = area->prev;
    }
    area->prev = 0;
    area->next = 0;
}

static int merge_adjacent_empty_areas(internal_data *data, empty_area *area)
{
    for (empty_area *current = data->empty_areas.first; current; current = current->next) {
        int same_height = current->y == area->y && current->height == area->height;
        // Adjacent area to the left
        if (same_height && current->x + current->width == area->x) {
            area->x = current->x;
            area->width += current->width;
            delist_empty_area(data, current);
            merge_adjacent_empty_areas(data, area);
            return 1;
        }
        // Adjacent area to the right
        if (same_height && area->x + area->width == current->x) {
            area->width += current->width;
            delist_empty_area(data, current);
            merge_adjacent_empty_areas(data, area);
            return 1;
        }
        int same_width = current->x == area->x && current->width == area->width;
        // Adjacent area to the top
        if (same_width && current->y + current->height == area->y) {
            area->y = current->y;
            area->height += current->height;
            delist_empty_area(data, current);
            merge_adjacent_empty_areas(data, area);
            return 1;
        }
        // Adjacent area to the bottom
        if (same_width && area->y + area->height == current->y) {
            area->height += current->height;
            delist_empty_area(data, current);
            merge_adjacent_empty_areas(data, area);
            return 1;
        }
    }
    return 0;
}

static void split_empty_area(internal_data *data, empty_area *area, unsigned int width, unsigned int height)
{
    empty_area *new_area = &data->empty_areas.list[++data->empty_areas.index];

    int remaining_width = area->width - width;
    int remaining_height = area->height - height;

    if (remaining_width > remaining_height) {
        new_area->x = area->x;
        new_area->y = area->y + height;
        new_area->width = width;
        new_area->height = area->height - height;
        area->x += width;
        area->width -= width;
    } else {
        new_area->x = area->x + width;
        new_area->y = area->y;
        new_area->width = area->width - width;
        new_area->height = height;
        area->y += height;
        area->height -= height;
    }

    empty_area *original_prev = area->prev;
    delist_empty_area(data, area);

    int merged = merge_adjacent_empty_areas(data, area) + merge_adjacent_empty_areas(data, new_area);

    data->empty_areas.set_comparator(area);
    data->empty_areas.set_comparator(new_area);

    if (!merged) {
        sort_empty_area(data, area, original_prev);
        sort_empty_area(data, new_area, area->prev);
    } else {
        if (new_area->comparator < area->comparator) {
            sort_empty_area(data, area, data->empty_areas.last);
            sort_empty_area(data, new_area, area->prev);
        } else {
            sort_empty_area(data, new_area, data->empty_areas.last);
            sort_empty_area(data, area, new_area->prev);
        }
    }
}

static int pack_rect(internal_data *data, image_packer_rect *rect, int allow_rotation)
{
    unsigned int width, height;

    if (!rect->output.rotated) {
        width = rect->input.width;
        height = rect->input.height;
    } else {
        width = rect->input.height;
        height = rect->input.width;
    }

    for (empty_area *area = data->empty_areas.first; area; area = area->next) {
        if (height > area->height || width > area->width) {
            continue;
        }
        rect->output.x = area->x;
        rect->output.y = area->y;
        rect->output.packed = 1;

        if (height == area->height && width == area->width) {
            delist_empty_area(data, area);
            return 1;
        }
        if (height == area->height) {
            area->x += width;
            area->width -= width;
            empty_area *prev = area->prev;
            delist_empty_area(data, area);
            if (merge_adjacent_empty_areas(data, area)) {
                prev = data->empty_areas.last;
            }
            data->empty_areas.set_comparator(area);
            sort_empty_area(data, area, prev);
            return 1;
        }
        if (width == area->width) {
            area->y += height;
            area->height -= height;
            empty_area *prev = area->prev;
            delist_empty_area(data, area);
            if (merge_adjacent_empty_areas(data, area)) {
                prev = data->empty_areas.last;
            }
            data->empty_areas.set_comparator(area);
            sort_empty_area(data, area, prev);
            return 1;
        }

        split_empty_area(data, area, width, height);
        return 1;
    }

    if (allow_rotation) {
        rect->output.rotated = 1;
        return pack_rect(data, rect, 0);
    }
    rect->output.rotated = 0;
    return 0;
}

static void reduce_last_image_size(image_packer *packer, unsigned int rects_area)
{
    internal_data *data = packer->internal_data;

    unsigned int current_area = data->image_width * data->image_height;

    if (current_area * 100 / rects_area < 100 + TOLERABLE_AREA_DIFFERENCE_PERCENTAGE) {
        return;
    }

    unsigned int image_index = packer->result.images_needed - 1;

    float image_ratio = data->image_width / (float) data->image_height;
    unsigned int needed_width = (unsigned int) sqrt(rects_area * image_ratio) + 1;
    unsigned int needed_height = (unsigned int) sqrt(rects_area / image_ratio) + 1;
    unsigned int delta_width = (data->image_width - needed_width) / 2;
    unsigned int delta_height = (data->image_height - needed_height) / 2;
    unsigned int last_successful_width = data->image_width;
    unsigned int last_successful_height = data->image_height;

    while (delta_width && delta_height) {
        if (last_successful_width == packer->result.last_image_width) {
            packer->result.last_image_width -= delta_width;
            packer->result.last_image_height -= delta_height;
        } else {
            packer->result.last_image_width += delta_width;
            packer->result.last_image_height += delta_height;
        }

        reset_empty_areas(data, packer->result.last_image_width, packer->result.last_image_height);

        int failed_to_pack = 0;

        for (unsigned int i = 0; i < data->num_rects; i++) {
            image_packer_rect *rect = data->sorted_rects[i];

            if (rect->output.image_index != image_index) {
                continue;
            }
            if (!pack_rect(data, rect, packer->options.allow_rotation)) {
                failed_to_pack = 1;
                break;
            }
        }
        if (!failed_to_pack) {
            last_successful_width = packer->result.last_image_width;
            last_successful_height = packer->result.last_image_height;

            unsigned int area_percentage_difference = last_successful_width * last_successful_height * 100 / rects_area;

            if (area_percentage_difference < 100 + TOLERABLE_AREA_DIFFERENCE_PERCENTAGE) {
                return;
            }
        }
        delta_width /= 2;
        delta_height /= 2;
    }

    if (last_successful_width != packer->result.last_image_width) {
        packer->result.last_image_width = last_successful_width;
        packer->result.last_image_height = last_successful_height;

        reset_empty_areas(data, packer->result.last_image_width, packer->result.last_image_height);

        for (unsigned int i = 0; i < data->num_rects; i++) {
            if (data->sorted_rects[i]->output.image_index != image_index) {
                continue;
            }
            pack_rect(data, data->sorted_rects[i], packer->options.allow_rotation);
        }
    }
}

int image_packer_init(image_packer *packer, unsigned int num_rectangles, unsigned int width, unsigned int height)
{
    memset(packer, 0, sizeof(image_packer));

    packer->internal_data = malloc(sizeof(internal_data));

    if (!packer->internal_data) {
        return IMAGE_PACKER_ERROR_NO_MEMORY;
    }

    memset(packer->internal_data, 0, sizeof(internal_data));

    packer->rects = (image_packer_rect *) malloc(sizeof(image_packer_rect) * num_rectangles);
    if (!packer->rects) {
        return IMAGE_PACKER_ERROR_NO_MEMORY;
    }
    memset(packer->rects, 0, sizeof(image_packer_rect) * num_rectangles);
    internal_data *data = packer->internal_data;
    data->num_rects = num_rectangles;
    data->image_width = width;
    data->image_height = height;

    unsigned int size = data->num_rects + 1;
    data->empty_areas.list = (empty_area *) malloc(size * sizeof(empty_area));
    if (!data->empty_areas.list) {
        return IMAGE_PACKER_ERROR_NO_MEMORY;
    }
    data->empty_areas.size = size;

    return IMAGE_PACKER_OK;
}

void image_packer_resize_image(image_packer *packer, unsigned int image_width, unsigned int image_height)
{
    internal_data *data = packer->internal_data;
    data->image_width = image_width;
    data->image_height = image_height;
}

int image_packer_pack(image_packer *packer)
{
    internal_data *data = packer->internal_data;

    if (!data->num_rects || !data->image_width || !data->image_height ||
        !packer->rects || data->empty_areas.size != data->num_rects + 1) {
        return IMAGE_PACKER_ERROR_WRONG_PARAMETERS;
    }
    if (packer->options.rects_are_sorted != 1 || !data->sorted_rects) {
        if (!sort_rects(packer)) {
            return IMAGE_PACKER_ERROR_NO_MEMORY;
        }
    }
    if (packer->options.always_repack) {
        packer->result.images_needed = 0;
    }
    int request_new_image;
    unsigned int packed_rects = 0;
    unsigned int area_used_in_last_image;

    do {
        reset_empty_areas(data, data->image_width, data->image_height);

        request_new_image = 0;
        area_used_in_last_image = 0;

        for (unsigned int i = 0; i < data->num_rects; i++) {
            image_packer_rect *rect = data->sorted_rects[i];

            if (rect->output.packed && (packer->options.always_repack != 1 || packer->result.images_needed != 0)) {
                continue;
            }
            rect->output.packed = 0;

            if (!pack_rect(data, rect, packer->options.allow_rotation)) {
                if (packer->options.fail_policy == IMAGE_PACKER_CONTINUE) {
                    continue;
                } else if (packer->options.fail_policy == IMAGE_PACKER_NEW_IMAGE) {
                    request_new_image = 1;
                } else {
                    return i;
                }
                if (data->empty_areas.first->width == data->image_width &&
                    data->empty_areas.first->height == data->image_height) {
                    packer->result.images_needed--;
                    return packed_rects;
                }
            } else {
                rect->output.image_index = packer->result.images_needed;
                rect->output.packed = 1;
                area_used_in_last_image += rect->input.width * rect->input.height;
                packed_rects++;
            }
        }
        packer->result.images_needed++;
    } while (request_new_image);

    packer->result.last_image_width = data->image_width;
    packer->result.last_image_height = data->image_height;

    if (packer->options.reduce_image_size == 1) {
        reduce_last_image_size(packer, area_used_in_last_image);
    }

    return packed_rects;
}

void image_packer_free(image_packer *packer)
{
    internal_data *data = packer->internal_data;
    free(data->empty_areas.list);
    free(data->sorted_rects);
    free(data);
    packer->internal_data = 0;
}

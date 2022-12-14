#include "image_packer.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

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

    if (!width || !height) {
        return 1;
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

static int create_last_image(image_packer *packer, unsigned int remaining_area)
{
    internal_data *data = packer->internal_data;

    double image_ratio = data->image_width / (double) data->image_height;
    unsigned int needed_width = (unsigned int) sqrt(image_ratio * remaining_area) + 1;
    unsigned int needed_height = (unsigned int) sqrt(remaining_area / image_ratio) + 1;
    unsigned int width_increase_step = needed_width / 64 + 1;
    unsigned int height_increase_step = needed_height / 64 + 1;
    packer->result.last_image_width = needed_width;
    packer->result.last_image_height = needed_height;

    int must_increase_size = 1;
    int total_images_packed = 0;

    while (must_increase_size) {
        packer->result.last_image_width += width_increase_step;
        packer->result.last_image_height += height_increase_step;

        if (packer->result.last_image_width > data->image_width) {
            packer->result.last_image_width = data->image_width;
        }
        if (packer->result.last_image_height > data->image_height) {
            packer->result.last_image_height = data->image_height;
        }

        int images_packed_in_loop = 0;
        int area_packed_in_loop = 0;

        reset_empty_areas(data, packer->result.last_image_width, packer->result.last_image_height);

        int failed = 0;

        for (unsigned int i = 0; i < data->num_rects; i++) {
            image_packer_rect *rect = data->sorted_rects[i];

            if (rect->output.packed && rect->output.image_index != packer->result.images_needed) {
                continue;
            }
            if (!pack_rect(data, rect, packer->options.allow_rotation)) {
                failed = 1;
                if (packer->result.last_image_width < data->image_width ||
                    packer->result.last_image_height < data->image_height) {
                    break;
                }
            } else {
                rect->output.packed = 1;
                rect->output.image_index = packer->result.images_needed;
                images_packed_in_loop++;
                area_packed_in_loop += rect->input.width * rect->input.height;
            }
        }

        if (!failed) {
            packer->result.images_needed++;
            total_images_packed += images_packed_in_loop;
            must_increase_size = 0;
        } else if (packer->result.last_image_width == data->image_width &&
              packer->result.last_image_height == data->image_height) {
            packer->result.images_needed++;
            total_images_packed += images_packed_in_loop;
            remaining_area -= area_packed_in_loop;
            needed_width = (unsigned int) sqrt(image_ratio * remaining_area) + 1;
            needed_height = (unsigned int) sqrt(remaining_area / image_ratio) + 1;
            width_increase_step = needed_width / 64 + 1;
            height_increase_step = needed_height / 64 + 1;
            packer->result.last_image_width = needed_width;
            packer->result.last_image_height = needed_height;
        }
    }
    return total_images_packed;
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
    unsigned int packed_rects = 0;
    unsigned int area_used_in_last_image = 0;
    unsigned int remaining_area = 0;

    for (unsigned int i = 0; i < data->num_rects; i++) {
        remaining_area += data->sorted_rects[i]->input.width * data->sorted_rects[i]->input.height;
    }

    unsigned int available_area = packer->options.reduce_image_size == 1 ? data->image_width * data->image_height : 0;

    while (remaining_area > available_area) {
        reset_empty_areas(data, data->image_width, data->image_height);

        area_used_in_last_image = 0;

        for (unsigned int i = 0; i < data->num_rects; i++) {
            image_packer_rect *rect = data->sorted_rects[i];

            if (rect->output.packed) {
                continue;
            }
            rect->output.packed = 0;
            if (!pack_rect(data, rect, packer->options.allow_rotation)) {
                if (packer->options.fail_policy == IMAGE_PACKER_CONTINUE) {
                    remaining_area -= rect->input.width * rect->input.height;
                    continue;
                } else if (packer->options.fail_policy == IMAGE_PACKER_STOP) {
                    packer->result.last_image_width = data->image_width;
                    packer->result.last_image_height = data->image_height;
                    return i;
                }
                if (data->empty_areas.first->width == data->image_width &&
                    data->empty_areas.first->height == data->image_height) {
                    packer->result.images_needed--;
                    packer->result.last_image_width = data->image_width;
                    packer->result.last_image_height = data->image_height;
                    return packed_rects;
                }
            } else {
                rect->output.image_index = packer->result.images_needed;
                rect->output.packed = 1;
                area_used_in_last_image += rect->input.width * rect->input.height;
                packed_rects++;
            }
        }
        remaining_area -= area_used_in_last_image;
        packer->result.images_needed++;
    }

    packer->result.last_image_width = data->image_width;
    packer->result.last_image_height = data->image_height;

    if (packer->options.reduce_image_size == 1) {
        packed_rects += create_last_image(packer, remaining_area);
    }

    return packed_rects;
}

void image_packer_free(image_packer *packer)
{
    internal_data *data = packer->internal_data;
    if (data) {
        free(data->empty_areas.list);
        free(data->sorted_rects);
        free(data);
    }
    free(packer->rects);
    memset(packer, 0, sizeof(image_packer));
}

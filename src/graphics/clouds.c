#include "clouds.h"

#include "core/config.h"
#include "core/image.h"
#include "core/random.h"
#include "core/speed.h"
#include "graphics/renderer.h"

#include <math.h>
#include <string.h>

#define NUM_CLOUD_ELLIPSES 30
#define CLOUD_ALPHA_CUTOFF 32

#define CLOUD_WIDTH 64
#define CLOUD_HEIGHT 64

#define CLOUD_SCALE 12

#define CLOUD_MIN_CREATION_TIMEOUT 200
#define CLOUD_MAX_CREATION_TIMEOUT 2400

#define CLOUD_ROWS 4
#define CLOUD_COLUMNS 4
#define NUM_CLOUDS (CLOUD_ROWS * CLOUD_COLUMNS)

#define CLOUD_TEXTURE_WIDTH (CLOUD_WIDTH * CLOUD_COLUMNS)
#define CLOUD_TEXTURE_HEIGHT (CLOUD_HEIGHT * CLOUD_ROWS)

#define CLOUD_SPEED 0.6

#define PI 3.14159265358979323846

typedef enum {
    STATUS_INACTIVE,
    STATUS_CREATED,
    STATUS_MOVING
} cloud_status;

typedef struct {
    int x;
    int y;
    int width;
    int height;
    int half_width;
    int half_height;
    int radius;
    int squared_width;
    int squared_height;
    int width_times_height;
} ellipse;

typedef struct {
    image img;
    int x;
    int y;
    cloud_status status;
    struct {
        speed_type x;
        speed_type y;
    } speed;
    float scale_x;
    float scale_y;
    int side;
    int angle;
} cloud_type;

static cloud_type clouds[NUM_CLOUDS];
static int movement_timeout;

static int random_from_min_to_range(int min, int range)
{
    return min + random_between_from_stdlib(0, range);
}

static void position_ellipse(ellipse *e, int cloud_width, int cloud_height)
{
    double angle = random_fractional_from_stdlib() * PI * 2;

    e->x = CLOUD_WIDTH / 2 + random_fractional_from_stdlib() * cloud_width * cos(angle);
    e->y = CLOUD_HEIGHT / 2 + random_fractional_from_stdlib() * cloud_height * sin(angle);

    e->width = random_from_min_to_range(CLOUD_WIDTH * 0.10, CLOUD_WIDTH * 0.10);
    e->height = random_from_min_to_range(CLOUD_HEIGHT * 0.10, CLOUD_HEIGHT * 0.10);

    e->half_width = e->width / 2;
    e->half_height = e->height / 2;
    e->squared_width = e->width * e->width;
    e->squared_height = e->height * e->height;

    e->width_times_height = e->squared_width * e->squared_height;
    e->radius = e->half_width > e->half_height ? e->half_width : e->half_height;
}

static int ellipse_is_inside_bounds(const ellipse *e)
{
    int x = e->x;
    int y = e->y;
    return x - e->width >= 0 && x + e->width < CLOUD_WIDTH &&
        y - e->height >= 0 && y + e->height < CLOUD_HEIGHT;
}

static void set_alpha(color_t *cloud, int x, int y, int alpha)
{
    alpha = 255 - alpha;

    int index = y * CLOUD_WIDTH + x;

    int current_alpha = cloud[index] >> COLOR_BITSHIFT_ALPHA;

    alpha = (current_alpha + ((alpha * (255 - current_alpha)) >> 8));

    if (alpha > CLOUD_ALPHA_CUTOFF) {
        alpha = CLOUD_ALPHA_CUTOFF;
    }

    cloud[index] = ALPHA_TRANSPARENT | (alpha << COLOR_BITSHIFT_ALPHA);
}

static void generate_cloud_ellipse(color_t *cloud, int width, int height)
{
    ellipse e;
    do {
        position_ellipse(&e, width, height);
    } while (!ellipse_is_inside_bounds(&e));

    // Do the entire diameter
    for (int x = -e.width; x <= e.width; x++) {
        int alpha = x * x * e.squared_height * 255 / e.width_times_height;
        set_alpha(cloud, e.x + x, e.y, alpha);
    }

    int line_width = e.width;
    int line_delta = 0;

    // Now do all four quarters at the same time, away from the diameter
    for (int y = 1; y <= e.height; y++) {
        int line_limit = line_width - (line_delta - 1);
        int squared_y = y * y;
        while (line_limit) {
            if (line_limit * line_limit * e.squared_height + squared_y * e.squared_width <= e.width_times_height) {
                break;
            }
            line_limit--;
        }
        line_delta = line_width - line_limit;
        line_width = line_limit;

        int alpha = squared_y * e.squared_width * 255 / e.width_times_height;

        set_alpha(cloud, e.x, e.y - y, alpha);
        set_alpha(cloud, e.x, e.y + y, alpha);

        for (int x = 1; x <= line_width; x++) {
            int alpha = (x * x * e.squared_height + squared_y * e.squared_width) * 255 / e.width_times_height;

            set_alpha(cloud, e.x + x, e.y - y, alpha);
            set_alpha(cloud, e.x + x, e.y + y, alpha);
            set_alpha(cloud, e.x - x, e.y - y, alpha);
            set_alpha(cloud, e.x - x, e.y + y, alpha);
        }
    }
}

static void init_cloud_images(void)
{
    graphics_renderer()->create_custom_image(CUSTOM_IMAGE_CLOUDS, CLOUD_TEXTURE_WIDTH, CLOUD_TEXTURE_HEIGHT, 0);
    for (int i = 0; i < NUM_CLOUDS; i++) {
        cloud_type *cloud = &clouds[i];
        image *img = &cloud->img;
        img->width = img->original.width = CLOUD_WIDTH;
        img->height = img->original.height = CLOUD_HEIGHT;
        img->atlas.id = (ATLAS_CUSTOM << IMAGE_ATLAS_BIT_OFFSET) | CUSTOM_IMAGE_CLOUDS;
        img->atlas.x_offset = (i % CLOUD_COLUMNS) * CLOUD_WIDTH;
        img->atlas.y_offset = (i / CLOUD_COLUMNS) * CLOUD_HEIGHT;
        cloud->x = 0;
        cloud->y = 0;
        cloud->side = 0;
        cloud->angle = 0;
        cloud->status = STATUS_INACTIVE;
        speed_clear(&cloud->speed.x);
        speed_clear(&cloud->speed.y);
    }
}

static void generate_cloud(cloud_type *cloud)
{
    if (!graphics_renderer()->has_custom_image(CUSTOM_IMAGE_CLOUDS)) {
        init_cloud_images();
    }

    color_t buffer[CLOUD_WIDTH * CLOUD_HEIGHT];
    memset(buffer, 0, sizeof(color_t) * CLOUD_WIDTH * CLOUD_HEIGHT);

    int width = random_from_min_to_range(CLOUD_WIDTH * 0.15f, CLOUD_WIDTH * 0.2f);
    int height = random_from_min_to_range(CLOUD_HEIGHT * 0.15f, CLOUD_HEIGHT * 0.2f);

    for (int i = 0; i < NUM_CLOUD_ELLIPSES; i++) {
        generate_cloud_ellipse(buffer, width, height);
    }

    const image *img = &cloud->img;

    graphics_renderer()->update_custom_image_from(CUSTOM_IMAGE_CLOUDS, buffer,
        img->atlas.x_offset, img->atlas.y_offset, img->width, img->height);

    cloud->x = 0;
    cloud->y = 0;
    cloud->scale_x = (1.5f - random_fractional_from_stdlib()) / CLOUD_SCALE;
    cloud->scale_y = (1.5f - random_fractional_from_stdlib()) / CLOUD_SCALE;
    int scaled_width = CLOUD_WIDTH / cloud->scale_x;
    int scaled_height = CLOUD_HEIGHT / cloud->scale_y;
    cloud->side = sqrt(scaled_width * scaled_width + scaled_height * scaled_height);
    cloud->angle = random_between_from_stdlib(0, 360);
    cloud->status = STATUS_CREATED;
}

static int cloud_intersects(const cloud_type *cloud)
{
    for (int i = 0; i < NUM_CLOUDS; i++) {
        const cloud_type *other = &clouds[i];
        if (other->status != STATUS_MOVING) {
            continue;
        }
        if (other->x < cloud->x + cloud->side && other->x + other->side > cloud->x &&
            other->y < cloud->y + cloud->side && other->y + other->side > cloud->y) {
            return 1;
        }
    }
    return 0;
}

static void position_cloud(cloud_type *cloud, int x_limit, int y_limit)
{
    int offset_x = random_between_from_stdlib(0, x_limit / 2);

    cloud->x = x_limit - offset_x + cloud->side;
    cloud->y = (y_limit - offset_x) / 2 - cloud->side;

    if (!cloud_intersects(cloud)) {
        cloud->status = STATUS_MOVING;
        speed_clear(&cloud->speed.x);
        speed_clear(&cloud->speed.y);
        movement_timeout = random_between_from_stdlib(CLOUD_MIN_CREATION_TIMEOUT, CLOUD_MAX_CREATION_TIMEOUT);
    }
}

void clouds_pause(void)
{
    for (int i = 0; i < NUM_CLOUDS; i++) {
        speed_clear(&clouds[i].speed.x);
        speed_clear(&clouds[i].speed.y);
    }
}

void clouds_draw(int x_offset, int y_offset, int x_limit, int y_limit, float base_scale)
{
    if (!config_get(CONFIG_UI_DRAW_CLOUD_SHADOWS)) {
        return;
    }
    for (int i = 0; i < NUM_CLOUDS; i++) {
        cloud_type *cloud = &clouds[i];
        if (cloud->status == STATUS_INACTIVE) {
            generate_cloud(cloud);
            continue;
        } else if (cloud->status == STATUS_CREATED) {
            if (movement_timeout > 0) {
                movement_timeout--;
            } else {
                position_cloud(cloud, x_limit, y_limit);
            }
            continue;
        } else if (cloud->x < -cloud->side || cloud->y >= y_limit) {
            cloud->status = STATUS_INACTIVE;
            continue;
        }
        speed_set_target(&cloud->speed.x, -CLOUD_SPEED, SPEED_CHANGE_IMMEDIATE, 1);
        speed_set_target(&cloud->speed.y, CLOUD_SPEED / 2, SPEED_CHANGE_IMMEDIATE, 1);

        graphics_renderer()->draw_image_advanced(&cloud->img,
            (cloud->x - x_offset) / base_scale, (cloud->y - y_offset) / base_scale, COLOR_MASK_NONE,
            cloud->scale_x * base_scale, cloud->scale_y * base_scale, cloud->angle, 1);

        cloud->x += speed_get_delta(&cloud->speed.x);
        cloud->y += speed_get_delta(&cloud->speed.y);
    }
}

#include "renderer.h"

#include "core/calc.h"
#include "core/config.h"
#include "core/time.h"
#include "graphics/renderer.h"
#include "graphics/screen.h"
#include "platform/cursor.h"
#include "platform/emscripten/emscripten.h"
#include "platform/haiku/haiku.h"
#include "platform/platform.h"
#include "platform/screen.h"
#include "platform/switch/switch.h"
#include "platform/vita/vita.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#if SDL_VERSION_ATLEAST(2, 0, 1)
#define USE_YUV_TEXTURES
#define HAS_YUV_TEXTURES (platform_sdl_version_at_least(2, 0, 1))
#else
#define HAS_YUV_TEXTURES 0
#endif

#if SDL_VERSION_ATLEAST(2, 0, 10)
#define USE_RENDERCOPYF
#define HAS_RENDERCOPYF (platform_sdl_version_at_least(2, 0, 10))
#endif

#if SDL_VERSION_ATLEAST(2, 0, 12)
#define USE_TEXTURE_SCALE_MODE
#define HAS_TEXTURE_SCALE_MODE (platform_sdl_version_at_least(2, 0, 12))
#else
#define HAS_TEXTURE_SCALE_MODE 0
#endif

#define MAX_UNPACKED_IMAGES 10

#define MAX_PACKED_IMAGE_SIZE 64000

#if (defined(__ANDROID__) || defined(__EMSCRIPTEN__)) && !SDL_VERSION_ATLEAST(2, 24, 0)
// On the arm versions of android, on SDL < 2.24.0, atlas textures that are too large will make the renderer fetch
// some images from the atlas with an off-by-one pixel, making things look terrible. Defining a smaller atlas texture
// prevents the problem, at the cost of performance due to the extra texture context switching.
// This also happens on emscripten for android, hence the emscripten inclusion.
#define MAX_TEXTURE_SIZE 1024
#endif

#ifdef __vita__
// On Vita, due to the small amount of VRAM, having textures that are too large will cause the game to eventually crash
// when changing climates, due to lack of contiguous memory space. Creating smaller atlases mitigates the issue
#define MAX_TEXTURE_SIZE 2048
#endif

typedef struct buffer_texture {
    SDL_Texture *texture;
    int id;
    int width;
    int height;
    int tex_width;
    int tex_height;
    struct buffer_texture *next;
} buffer_texture;

static struct {
    SDL_Renderer *renderer;
    SDL_Texture *render_texture;
    int is_software_renderer;
    int paused;
    struct {
        SDL_Texture *texture;
        int size;
        struct {
            int x, y;
        } hotspot;
    } cursors[CURSOR_MAX];
    SDL_Texture **texture_lists[ATLAS_MAX];
    image_atlas_data atlas_data[ATLAS_MAX];
    struct {
        SDL_Texture *texture;
        color_t *buffer;
        image img;
    } custom_textures[CUSTOM_IMAGE_MAX];
    struct {
        int width;
        int height;
    } max_texture_size;
    struct {
        buffer_texture *first;
        buffer_texture *last;
        int current_id;
    } texture_buffers;
    struct {
        int id;
        time_millis last_used;
        SDL_Texture *texture;
    } unpacked_images[MAX_UNPACKED_IMAGES];
    graphics_renderer_interface renderer_interface;
    int supports_yuv_textures;
    float city_scale;
    int should_correct_texture_offset;
    int disable_linear_filter;
} data;

static int save_screen_buffer(color_t *pixels, int x, int y, int width, int height, int row_width)
{
    if (data.paused) {
        return 0;
    }
    SDL_Rect rect = { x, y, width, height };
    return SDL_RenderReadPixels(data.renderer, &rect, SDL_PIXELFORMAT_ARGB8888, pixels,
        row_width * sizeof(color_t)) == 0;
}

static void draw_line(int x_start, int x_end, int y_start, int y_end, color_t color)
{
    if (data.paused) {
        return;
    }
    SDL_SetRenderDrawColor(data.renderer,
        (color & COLOR_CHANNEL_RED) >> COLOR_BITSHIFT_RED,
        (color & COLOR_CHANNEL_GREEN) >> COLOR_BITSHIFT_GREEN,
        (color & COLOR_CHANNEL_BLUE) >> COLOR_BITSHIFT_BLUE,
        (color & COLOR_CHANNEL_ALPHA) >> COLOR_BITSHIFT_ALPHA);
    SDL_RenderDrawLine(data.renderer, x_start, y_start, x_end, y_end);
}

static void draw_rect(int x_start, int x_end, int y_start, int y_end, color_t color)
{
    if (data.paused) {
        return;
    }
    SDL_SetRenderDrawColor(data.renderer,
        (color & COLOR_CHANNEL_RED) >> COLOR_BITSHIFT_RED,
        (color & COLOR_CHANNEL_GREEN) >> COLOR_BITSHIFT_GREEN,
        (color & COLOR_CHANNEL_BLUE) >> COLOR_BITSHIFT_BLUE,
        (color & COLOR_CHANNEL_ALPHA) >> COLOR_BITSHIFT_ALPHA);
    SDL_Rect rect = { x_start, y_start, x_end, y_end };
    SDL_RenderDrawRect(data.renderer, &rect);
}

static void fill_rect(int x_start, int x_end, int y_start, int y_end, color_t color)
{
    if (data.paused) {
        return;
    }
    SDL_SetRenderDrawColor(data.renderer,
        (color & COLOR_CHANNEL_RED) >> COLOR_BITSHIFT_RED,
        (color & COLOR_CHANNEL_GREEN) >> COLOR_BITSHIFT_GREEN,
        (color & COLOR_CHANNEL_BLUE) >> COLOR_BITSHIFT_BLUE,
        (color & COLOR_CHANNEL_ALPHA) >> COLOR_BITSHIFT_ALPHA);
    SDL_Rect rect = { x_start, y_start, x_end, y_end };
    SDL_RenderFillRect(data.renderer, &rect);
}

static void set_clip_rectangle(int x, int y, int width, int height)
{
    if (data.paused) {
        return;
    }
    SDL_Rect clip = { x, y, width, height };
    SDL_RenderSetClipRect(data.renderer, &clip);
}

static void reset_clip_rectangle(void)
{
    if (data.paused) {
        return;
    }
    SDL_RenderSetClipRect(data.renderer, NULL);
}

static void set_viewport(int x, int y, int width, int height)
{
    if (data.paused) {
        return;
    }
    SDL_Rect viewport = { x, y, width, height };
    SDL_RenderSetViewport(data.renderer, &viewport);
}

static void reset_viewport(void)
{
    if (data.paused) {
        return;
    }
    SDL_RenderSetViewport(data.renderer, NULL);
    SDL_RenderSetClipRect(data.renderer, NULL);
}

static void clear_screen(void)
{
    if (data.paused) {
        return;
    }
    SDL_SetRenderDrawColor(data.renderer, 0, 0, 0, 0);
    SDL_RenderClear(data.renderer);
}

static void get_max_image_size(int *width, int *height)
{
    *width = data.max_texture_size.width;
    *height = data.max_texture_size.height;
}

static void free_unpacked_assets(void)
{
    for (int i = 0; i < MAX_UNPACKED_IMAGES; i++) {
        if (data.unpacked_images[i].texture) {
            SDL_DestroyTexture(data.unpacked_images[i].texture);
        }
    }
    memset(data.unpacked_images, 0, sizeof(data.unpacked_images));
}

static void free_texture_atlas(atlas_type type)
{
    if (!data.texture_lists[type]) {
        return;
    }
    SDL_Texture **list = data.texture_lists[type];
    data.texture_lists[type] = 0;
    for (int i = 0; i < data.atlas_data[type].num_images; i++) {
        if (list[i]) {
            SDL_DestroyTexture(list[i]);
        }
    }
    free(list);

    if (type == ATLAS_EXTRA_ASSET) {
        free_unpacked_assets();
    }
}

static void free_atlas_data_buffers(atlas_type type)
{
    image_atlas_data *atlas_data = &data.atlas_data[type];
    if (atlas_data->buffers) {
#ifndef __VITA__
        for (int i = 0; i < atlas_data->num_images; i++) {
            free(atlas_data->buffers[i]);
        }
#endif
        free(atlas_data->buffers);
        atlas_data->buffers = 0;
    }
    if (atlas_data->image_widths) {
        free(atlas_data->image_widths);
        atlas_data->image_widths = 0;
    }
    if (atlas_data->image_heights) {
        free(atlas_data->image_heights);
        atlas_data->image_heights = 0;
    }
}

static void reset_atlas_data(atlas_type type)
{
    free_atlas_data_buffers(type);
    data.atlas_data[type].num_images = 0;
    data.atlas_data[type].type = type;
}

static void free_texture_atlas_and_data(atlas_type type)
{
    free_texture_atlas(type);
    reset_atlas_data(type);
}

static const image_atlas_data *prepare_texture_atlas(atlas_type type, int num_images, int last_width, int last_height)
{
    free_texture_atlas_and_data(type);
    image_atlas_data *atlas_data = &data.atlas_data[type];
    atlas_data->num_images = num_images;
    atlas_data->image_widths = malloc(sizeof(int) * num_images);
    atlas_data->image_heights = malloc(sizeof(int) * num_images);
    atlas_data->buffers = malloc(sizeof(color_t *) * num_images);
    if (!atlas_data->image_widths || !atlas_data->image_heights || !atlas_data->buffers) {
        reset_atlas_data(type);
        return 0;
    }
#ifdef __VITA__
    SDL_Texture **list = malloc(sizeof(SDL_Texture *) * num_images);
    if (!list) {
        reset_atlas_data(type);
        return 0;
    }
    memset(list, 0, sizeof(SDL_Texture *) * num_images);
    for (int i = 0; i < num_images; i++) {
        int width = i == num_images - 1 ? last_width : data.max_texture_size.width;
        atlas_data->image_heights[i] = i == num_images - 1 ? last_height : data.max_texture_size.height;
        SDL_Log("Creating atlas texture with size %dx%d", width, atlas_data->image_heights[i]);
        list[i] = SDL_CreateTexture(data.renderer,
            SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, atlas_data->image_heights[i]);
        if (!list[i]) {
            SDL_LogError(SDL_LOG_PRIORITY_ERROR, "Unable to create texture. Reason: %s", SDL_GetError());
            free_texture_atlas(type);
            reset_atlas_data(type);
            continue;
        }
        SDL_Log("Texture created");
        SDL_LockTexture(list[i], NULL, (void **) &atlas_data->buffers[i], &atlas_data->image_widths[i]);
        atlas_data->image_widths[i] /= sizeof(color_t);
        SDL_SetTextureBlendMode(list[i], SDL_BLENDMODE_BLEND);
    }
    data.texture_lists[type] = list;
#else
    for (int i = 0; i < num_images; i++) {
        atlas_data->image_widths[i] = i == num_images - 1 ? last_width : data.max_texture_size.width;
        atlas_data->image_heights[i] = i == num_images - 1 ? last_height : data.max_texture_size.height;
        int size = atlas_data->image_widths[i] * atlas_data->image_heights[i] * sizeof(color_t);
        atlas_data->buffers[i] = malloc(size);
        if (!atlas_data->buffers[i]) {
            reset_atlas_data(type);
            return 0;
        }
        memset(atlas_data->buffers[i], 0, size);
    }
#endif
    return atlas_data;
}

static int create_texture_atlas(const image_atlas_data *atlas_data, int delete_buffers)
{
    if (!atlas_data || atlas_data != &data.atlas_data[atlas_data->type] || !atlas_data->num_images) {
        return 0;
    }
#ifdef __VITA__
    SDL_Texture **list = data.texture_lists[atlas_data->type];
    for (int i = 0; i < atlas_data->num_images; i++) {
        SDL_UnlockTexture(list[i]);
    }
#else
    data.texture_lists[atlas_data->type] = malloc(sizeof(SDL_Texture *) * atlas_data->num_images);
    SDL_Texture **list = data.texture_lists[atlas_data->type];
    if (!list) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to create texture lists for atlas %d - out of memory",
            atlas_data->type);
        return 0;
    }
    memset(list, 0, sizeof(SDL_Texture *) * atlas_data->num_images);
    for (int i = 0; i < atlas_data->num_images; i++) {
        SDL_Log("Creating atlas texture with size %dx%d", atlas_data->image_widths[i], atlas_data->image_heights[i]);
        SDL_Surface *surface = SDL_CreateRGBSurfaceFrom((void *) atlas_data->buffers[i],
            atlas_data->image_widths[i], atlas_data->image_heights[i],
            32, atlas_data->image_widths[i] * sizeof(color_t),
            COLOR_CHANNEL_RED, COLOR_CHANNEL_GREEN, COLOR_CHANNEL_BLUE, COLOR_CHANNEL_ALPHA);
        if (!surface) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to create surface for texture. Reason: %s",
                SDL_GetError());
            free_texture_atlas(atlas_data->type);
            return 0;
        }
        list[i] = SDL_CreateTextureFromSurface(data.renderer, surface);
        SDL_FreeSurface(surface);
        if (delete_buffers) {
            free(atlas_data->buffers[i]);
            atlas_data->buffers[i] = 0;
        }
        if (!list[i]) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to create texture. Reason: %s", SDL_GetError());
            free_texture_atlas(atlas_data->type);
            return 0;
        }
        SDL_SetTextureBlendMode(list[i], SDL_BLENDMODE_BLEND);
    }
#endif
    if (delete_buffers) {
        free_atlas_data_buffers(atlas_data->type);
    }
    return 1;
}

static int has_texture_atlas(atlas_type type)
{
    return data.texture_lists[type] != 0;
}

static const image_atlas_data *get_texture_atlas(atlas_type type)
{
    if (!has_texture_atlas(type)) {
        return 0;
    }
    return &data.atlas_data[type];
}

static void free_all_textures(void)
{
    for (atlas_type i = ATLAS_FIRST; i < ATLAS_MAX - 1; i++) {
        free_texture_atlas_and_data(i);
    }
    for (int i = 0; i < CUSTOM_IMAGE_MAX; i++) {
        if (data.custom_textures[i].texture) {
            SDL_DestroyTexture(data.custom_textures[i].texture);
            data.custom_textures[i].texture = 0;
#ifndef __vita__
            free(data.custom_textures[i].buffer);
#endif
            data.custom_textures[i].buffer = 0;
            memset(&data.custom_textures[i].img, 0, sizeof(image));
        }
    }

    buffer_texture *texture_info = data.texture_buffers.first;
    while (texture_info) {
        buffer_texture *current = texture_info;
        texture_info = texture_info->next;
        SDL_DestroyTexture(current->texture);
        free(current);
    }
    data.texture_buffers.first = 0;
    data.texture_buffers.last = 0;
    data.texture_buffers.current_id = 0;
}

static SDL_Texture *get_texture(int texture_id)
{
    atlas_type type = texture_id >> IMAGE_ATLAS_BIT_OFFSET;
    if (type == ATLAS_CUSTOM) {
        return data.custom_textures[texture_id & IMAGE_ATLAS_BIT_MASK].texture;
    } else if (type == ATLAS_EXTERNAL) {
        return data.custom_textures[CUSTOM_IMAGE_EXTERNAL].texture;
    } else if (type == ATLAS_UNPACKED_EXTRA_ASSET) {
        int unpacked_asset_id = texture_id & IMAGE_ATLAS_BIT_MASK;
        for (int i = 0; i < MAX_UNPACKED_IMAGES; i++) {
            if (data.unpacked_images[i].id == unpacked_asset_id && data.unpacked_images[i].texture) {
                return data.unpacked_images[i].texture;
            }
        }
        return 0;
    }
    if (!data.texture_lists[type]) {
        return 0;
    }
    return data.texture_lists[type][texture_id & IMAGE_ATLAS_BIT_MASK];
}

static void set_texture_color_and_scale_mode(SDL_Texture *texture, color_t color, float scale)
{
    SDL_SetTextureColorMod(texture,
        (color & COLOR_CHANNEL_RED) >> COLOR_BITSHIFT_RED,
        (color & COLOR_CHANNEL_GREEN) >> COLOR_BITSHIFT_GREEN,
        (color & COLOR_CHANNEL_BLUE) >> COLOR_BITSHIFT_BLUE);
    SDL_SetTextureAlphaMod(texture, (color & COLOR_CHANNEL_ALPHA) >> COLOR_BITSHIFT_ALPHA);

#ifdef USE_TEXTURE_SCALE_MODE
    if (!HAS_TEXTURE_SCALE_MODE) {
        return;
    }
    SDL_ScaleMode current_scale_mode;
    SDL_GetTextureScaleMode(texture, &current_scale_mode);

    SDL_ScaleMode city_scale_mode = SDL_ScaleModeNearest;
    SDL_ScaleMode texture_scale_mode = scale != 1.0f ? SDL_ScaleModeLinear : SDL_ScaleModeNearest;
    SDL_ScaleMode desired_scale_mode = data.city_scale == scale ? city_scale_mode : texture_scale_mode;
    if (data.disable_linear_filter) {
        desired_scale_mode = SDL_ScaleModeNearest;
    }
    if (current_scale_mode != desired_scale_mode) {
        SDL_SetTextureScaleMode(texture, desired_scale_mode);
    }
#endif
}

static void draw_texture(const image *img, int x, int y, color_t color, float scale)
{
    if (data.paused) {
        return;
    }
    if (!color) {
        color = COLOR_MASK_NONE;
    }
    SDL_Texture *texture = get_texture(img->atlas.id);

    if (!texture) {
        return;
    }

    set_texture_color_and_scale_mode(texture, color, scale);

    x += img->x_offset;
    y += img->y_offset;

    int src_correction = scale == data.city_scale && data.should_correct_texture_offset ? 1 : 0;

    SDL_Rect src_coords = { img->atlas.x_offset + src_correction, img->atlas.y_offset + src_correction,
        img->width - src_correction, img->height - src_correction };

    // When zooming out, instead of drawing the grid image, we reduce the isometric textures' size,
    // which ends up simulating a grid without any performance penalty
    int grid_correction = (img->is_isometric && config_get(CONFIG_UI_SHOW_GRID) && data.city_scale > 2.0f) ?
        2 : -src_correction;

#ifdef USE_RENDERCOPYF
    if (HAS_RENDERCOPYF) {
        SDL_FRect dst_coords = { (x + grid_correction) / scale, (y + grid_correction) / scale,
            (img->width - grid_correction) / scale, (img->height - grid_correction) / scale };
        SDL_RenderCopyF(data.renderer, texture, &src_coords, &dst_coords);
        return;
    }
#endif

    SDL_Rect dst_coords = { (int) round((x + grid_correction) / scale), (int) round((y + grid_correction) / scale),
        (int) round((img->width - grid_correction) / scale), (int) round((img->height - grid_correction) / scale) };
    SDL_RenderCopy(data.renderer, texture, &src_coords, &dst_coords);
}

static void create_custom_texture(custom_image_type type, int width, int height, int is_yuv)
{
    if (data.paused) {
        return;
    }
    if (data.custom_textures[type].texture) {
        SDL_DestroyTexture(data.custom_textures[type].texture);
        data.custom_textures[type].texture = 0;
    }
    memset(&data.custom_textures[type].img, 0, sizeof(data.custom_textures[type].img));
#ifndef __vita__
    if (data.custom_textures[type].buffer) {
        free(data.custom_textures[type].buffer);
        data.custom_textures[type].buffer = 0;
    }
#endif

    data.custom_textures[type].texture = SDL_CreateTexture(data.renderer,
        is_yuv ? SDL_PIXELFORMAT_YV12 : SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
    data.custom_textures[type].img.width = width;
    data.custom_textures[type].img.height = height;
    data.custom_textures[type].img.atlas.id = (ATLAS_CUSTOM << IMAGE_ATLAS_BIT_OFFSET) | type;
    SDL_SetTextureBlendMode(data.custom_textures[type].texture,
        type == CUSTOM_IMAGE_VIDEO ? SDL_BLENDMODE_NONE : SDL_BLENDMODE_BLEND);
}

static color_t *get_custom_texture_buffer(custom_image_type type, int *actual_texture_width)
{
    if (data.paused || !data.custom_textures[type].texture) {
        return 0;
    }

#ifdef __vita__
    int pitch;
    SDL_LockTexture(data.custom_textures[type].texture, NULL, (void **) &data.custom_textures[type].buffer, &pitch);
    if (actual_texture_width) {
        *actual_texture_width = pitch / sizeof(color_t);
    }
    SDL_UnlockTexture(data.custom_textures[type].texture);
#else
    free(data.custom_textures[type].buffer);
    int width, height;
    Uint32 format;
    SDL_QueryTexture(data.custom_textures[type].texture, &format, NULL, &width, &height);
    if (format == SDL_PIXELFORMAT_YV12) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot get buffer to YUV texture");
        return 0;
    }
    data.custom_textures[type].buffer = (color_t *) malloc((size_t) width * height * sizeof(color_t));
    if (actual_texture_width) {
        *actual_texture_width = width;
    }
#endif
    return data.custom_textures[type].buffer;
}

static void release_custom_texture_buffer(custom_image_type type)
{
#ifndef __vita__
    free(data.custom_textures[type].buffer);
    data.custom_textures[type].buffer = 0;
#endif
}

static void update_custom_texture(custom_image_type type)
{
#ifndef __vita__
    if (data.paused || !data.custom_textures[type].texture || !data.custom_textures[type].buffer) {
        return;
    }
    int width, height;
    SDL_QueryTexture(data.custom_textures[type].texture, NULL, NULL, &width, &height);
    SDL_UpdateTexture(data.custom_textures[type].texture, NULL,
        data.custom_textures[type].buffer, sizeof(color_t) * width);
#endif
}

static void update_custom_texture_yuv(custom_image_type type, const uint8_t *y_data, int y_width,
    const uint8_t *cb_data, int cb_width, const uint8_t *cr_data, int cr_width)
{
#ifdef USE_YUV_TEXTURES
    if (data.paused || !data.supports_yuv_textures || !data.custom_textures[type].texture) {
        return;
    }
    int width, height;
    Uint32 format;
    SDL_QueryTexture(data.custom_textures[type].texture, &format, NULL, &width, &height);
    if (format != SDL_PIXELFORMAT_YV12) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Texture is not YUV format");
        return;
    }
    SDL_UpdateYUVTexture(data.custom_textures[type].texture, NULL,
        y_data, y_width, cb_data, cb_width, cr_data, cr_width);
#endif
}

static buffer_texture *get_saved_texture_info(int texture_id)
{
    if (!texture_id || !data.texture_buffers.first) {
        return 0;
    }
    for (buffer_texture *texture_info = data.texture_buffers.first; texture_info; texture_info = texture_info->next) {
        if (texture_info->id == texture_id) {
            return texture_info;
        }
    }
    return 0;
}

static int save_to_texture(int texture_id, int x, int y, int width, int height)
{
    if (data.paused) {
        return 0;
    }
    SDL_Texture *former_target = SDL_GetRenderTarget(data.renderer);
    if (!former_target) {
        return 0;
    }

    buffer_texture *texture_info = get_saved_texture_info(texture_id);
    SDL_Texture *texture = 0;

    if (!texture_info || (texture_info && (texture_info->tex_width < width || texture_info->tex_height < height))) {
        if (texture_info) {
            SDL_DestroyTexture(texture_info->texture);
            texture_info->texture = 0;
            texture_info->tex_width = 0;
            texture_info->tex_height = 0;
        }
        texture = SDL_CreateTexture(data.renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_TARGET, width, height);
        if (!texture) {
            return 0;
        }
#ifdef USE_TEXTURE_SCALE_MODE
        if (HAS_TEXTURE_SCALE_MODE) {
            SDL_SetTextureScaleMode(texture, SDL_ScaleModeNearest);
        }
#endif
    } else {
        texture = texture_info->texture;
    }

    SDL_Rect former_viewport;
    SDL_RenderGetViewport(data.renderer, &former_viewport);
    SDL_Rect src_rect = { x + former_viewport.x, y + former_viewport.y, width, height };
    SDL_Rect dst_rect = { 0, 0, width, height };
    SDL_SetRenderTarget(data.renderer, texture);
    SDL_RenderCopy(data.renderer, former_target, &src_rect, &dst_rect);
    SDL_SetRenderTarget(data.renderer, former_target);
    SDL_RenderSetViewport(data.renderer, &former_viewport);

    if (!texture_info) {
        texture_info = malloc(sizeof(buffer_texture));

        if (!texture_info) {
            SDL_DestroyTexture(texture);
            return 0;
        }

        memset(texture_info, 0, sizeof(buffer_texture));

        texture_info->id = ++data.texture_buffers.current_id;
        texture_info->next = 0;

        if (!data.texture_buffers.first) {
            data.texture_buffers.first = texture_info;
        } else {
            data.texture_buffers.last->next = texture_info;
        }
        data.texture_buffers.last = texture_info;
    }
    texture_info->texture = texture;
    texture_info->width = width;
    texture_info->height = height;
    if (width > texture_info->tex_width) {
        texture_info->tex_width = width;
    }
    if (height > texture_info->tex_height) {
        texture_info->tex_height = height;
    }

    return texture_info->id;
}

static void draw_saved_texture(int texture_id, int x, int y)
{
    if (data.paused) {
        return;
    }
    buffer_texture *texture_info = get_saved_texture_info(texture_id);
    if (!texture_info) {
        return;
    }
    SDL_Rect src_coords = { 0, 0, texture_info->width, texture_info->height };
    SDL_Rect dst_coords = { x, y, texture_info->width, texture_info->height };
    SDL_RenderCopy(data.renderer, texture_info->texture, &src_coords, &dst_coords);
}

static void create_blend_texture(custom_image_type type)
{
    SDL_Texture *texture = SDL_CreateTexture(data.renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_TARGET, 58, 30);
    if (!texture) {
        return;
    }
    const image *img = image_get(image_group(GROUP_TERRAIN_FLAT_TILE));
    SDL_Texture *flat_tile = get_texture(img->atlas.id);
    SDL_Texture *former_target = SDL_GetRenderTarget(data.renderer);
    SDL_Rect former_viewport;
    SDL_Rect former_clip;
    SDL_RenderGetViewport(data.renderer, &former_viewport);
    SDL_RenderGetClipRect(data.renderer, &former_clip);

    SDL_SetRenderTarget(data.renderer, texture);
    SDL_Rect rect = { 0, 0, 58, 30 };
    SDL_RenderSetClipRect(data.renderer, &rect);
    SDL_RenderSetViewport(data.renderer, &rect);
    SDL_SetRenderDrawColor(data.renderer, 0xff, 0xff, 0xff, 0xff);
    color_t color = type == CUSTOM_IMAGE_RED_FOOTPRINT ? COLOR_MASK_RED : COLOR_MASK_GREEN;
    SDL_RenderClear(data.renderer);
    SDL_SetTextureBlendMode(flat_tile, SDL_BLENDMODE_BLEND);

    SDL_SetTextureColorMod(flat_tile,
        (color & COLOR_CHANNEL_RED) >> COLOR_BITSHIFT_RED,
        (color & COLOR_CHANNEL_GREEN) >> COLOR_BITSHIFT_GREEN,
        (color & COLOR_CHANNEL_BLUE) >> COLOR_BITSHIFT_BLUE);
    SDL_SetTextureAlphaMod(flat_tile, 0xff);
    SDL_Rect src_coords = { img->atlas.x_offset, img->atlas.y_offset, img->width, img->height };
    SDL_RenderCopy(data.renderer, flat_tile, &src_coords, 0);

    SDL_SetRenderTarget(data.renderer, former_target);
    SDL_RenderSetViewport(data.renderer, &former_viewport);
    SDL_RenderSetClipRect(data.renderer, &former_clip);

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_MOD);

    data.custom_textures[type].texture = texture;
    memset(&data.custom_textures[type].img, 0, sizeof(data.custom_textures[type].img));
    data.custom_textures[type].img.is_isometric = 1;
    data.custom_textures[type].img.width = 58;
    data.custom_textures[type].img.height = 30;
    data.custom_textures[type].img.atlas.id = (ATLAS_CUSTOM << IMAGE_ATLAS_BIT_OFFSET) | type;
}

static void draw_custom_texture(custom_image_type type, int x, int y, float scale, int disable_filtering)
{
    if (data.paused) {
        return;
    }
    if (type == CUSTOM_IMAGE_RED_FOOTPRINT || type == CUSTOM_IMAGE_GREEN_FOOTPRINT) {
        if (!data.custom_textures[type].texture) {
            create_blend_texture(type);
        }
    }
    data.disable_linear_filter = disable_filtering;
    draw_texture(&data.custom_textures[type].img, x, y, 0, scale);
    data.disable_linear_filter = 0;
}

static int has_custom_texture(custom_image_type type)
{
    return data.custom_textures[type].texture != 0;
}

static void load_unpacked_image(const image *img, const color_t *pixels)
{
    if (data.paused) {
        return;
    }
    int unpacked_image_id = img->atlas.id & IMAGE_ATLAS_BIT_MASK;
    int first_empty = -1;
    int oldest_texture_index = 0;
    for (int i = 0; i < MAX_UNPACKED_IMAGES; i++) {
        if (data.unpacked_images[i].id == unpacked_image_id && data.unpacked_images[i].texture) {
            return;
        }
        if (first_empty == -1 && !data.unpacked_images[i].texture) {
            first_empty = i;
            break;
        }
        if (data.unpacked_images[oldest_texture_index].last_used < data.unpacked_images[i].last_used) {
            oldest_texture_index = i;
        }
    }
    int index = first_empty != -1 ? first_empty : oldest_texture_index;

    int image_height = img->height;
    if (img->top) {
        image_height += img->top->height;
    }

    SDL_Surface *surface = SDL_CreateRGBSurfaceFrom((void *) pixels, img->width, image_height, 32,
        img->width * sizeof(color_t), COLOR_CHANNEL_RED, COLOR_CHANNEL_GREEN, COLOR_CHANNEL_BLUE, COLOR_CHANNEL_ALPHA);
    if (!surface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to create surface for texture. Reason: %s", SDL_GetError());
        return;
    }
    data.unpacked_images[index].last_used = time_get_millis();
    data.unpacked_images[index].id = unpacked_image_id;

    if (data.unpacked_images[index].texture) {
        SDL_DestroyTexture(data.unpacked_images[index].texture);
        data.unpacked_images[index].texture = 0;
    }
    data.unpacked_images[index].texture = SDL_CreateTextureFromSurface(data.renderer, surface);
    while (!data.unpacked_images[index].texture) {
        int oldest_texture_index = -1;
        for (int i = 0; i < MAX_UNPACKED_IMAGES; i++) {
            if (data.unpacked_images[i].texture &&
                (oldest_texture_index == -1 ||
                data.unpacked_images[oldest_texture_index].last_used < data.unpacked_images[i].last_used)) {
                oldest_texture_index = i;
            }
        }
        if (oldest_texture_index == -1) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to create surface for texture - %s", SDL_GetError());
            SDL_FreeSurface(surface);
            return;
        }
        SDL_DestroyTexture(data.unpacked_images[oldest_texture_index].texture);
        data.unpacked_images[oldest_texture_index].texture = 0;
        data.unpacked_images[index].texture = SDL_CreateTextureFromSurface(data.renderer, surface);
    }
    SDL_SetTextureBlendMode(data.unpacked_images[index].texture, SDL_BLENDMODE_BLEND);
    SDL_FreeSurface(surface);
}

static int should_pack_image(int width, int height)
{
    return width * height < MAX_PACKED_IMAGE_SIZE;
}

static void update_scale(int city_scale)
{
    // The renderer draws the textures off-by-one when "scale * 100" is a multiple of 8, or when zooming out enough,
    // this fixes that rendering bug by properly offseting the textures
    data.should_correct_texture_offset = (city_scale > 250 && (city_scale % 100) != 0) || (city_scale % 8) == 0;
    data.city_scale = city_scale / 100.0f;
}

static int supports_yuv_texture(void)
{
    return data.supports_yuv_textures;
}

static void create_renderer_interface(void)
{
    data.renderer_interface.clear_screen = clear_screen;
    data.renderer_interface.set_viewport = set_viewport;
    data.renderer_interface.reset_viewport = reset_viewport;
    data.renderer_interface.set_clip_rectangle = set_clip_rectangle;
    data.renderer_interface.reset_clip_rectangle = reset_clip_rectangle;
    data.renderer_interface.draw_line = draw_line;
    data.renderer_interface.draw_rect = draw_rect;
    data.renderer_interface.fill_rect = fill_rect;
    data.renderer_interface.draw_image = draw_texture;
    data.renderer_interface.create_custom_image = create_custom_texture;
    data.renderer_interface.has_custom_image = has_custom_texture;
    data.renderer_interface.get_custom_image_buffer = get_custom_texture_buffer;
    data.renderer_interface.release_custom_image_buffer = release_custom_texture_buffer;
    data.renderer_interface.update_custom_image = update_custom_texture;
    data.renderer_interface.update_custom_image_yuv = update_custom_texture_yuv;
    data.renderer_interface.draw_custom_image = draw_custom_texture;
    data.renderer_interface.supports_yuv_image_format = supports_yuv_texture;
    data.renderer_interface.save_image_from_screen = save_to_texture;
    data.renderer_interface.draw_image_to_screen = draw_saved_texture;
    data.renderer_interface.save_screen_buffer = save_screen_buffer;
    data.renderer_interface.get_max_image_size = get_max_image_size;
    data.renderer_interface.prepare_image_atlas = prepare_texture_atlas;
    data.renderer_interface.create_image_atlas = create_texture_atlas;
    data.renderer_interface.get_image_atlas = get_texture_atlas;
    data.renderer_interface.has_image_atlas = has_texture_atlas;
    data.renderer_interface.free_image_atlas = free_texture_atlas_and_data;
    data.renderer_interface.load_unpacked_image = load_unpacked_image;
    data.renderer_interface.should_pack_image = should_pack_image;
    data.renderer_interface.update_scale = update_scale;

    graphics_renderer_set_interface(&data.renderer_interface);
}

int platform_renderer_init(SDL_Window *window)
{
    free_all_textures();

    SDL_Log("Creating renderer");
    data.renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (!data.renderer) {
        SDL_Log("Unable to create renderer, trying software renderer: %s", SDL_GetError());
        data.renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
        if (!data.renderer) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to create renderer: %s", SDL_GetError());
            return 0;
        }
    }

    SDL_RendererInfo info;
    SDL_GetRendererInfo(data.renderer, &info);
    SDL_Log("Loaded renderer: %s", info.name);

#ifdef USE_YUV_TEXTURES
    if (!data.supports_yuv_textures && HAS_YUV_TEXTURES) {
        for (unsigned int i = 0; i < info.num_texture_formats; i++) {
            if (info.texture_formats[i] == SDL_PIXELFORMAT_YV12) {
                data.supports_yuv_textures = 1;
                break;
            }
        }
    }
#endif

    data.is_software_renderer = info.flags & SDL_RENDERER_SOFTWARE;
    if (data.is_software_renderer) {
        data.max_texture_size.width = 4096;
        data.max_texture_size.height = 4096;
    } else {
        data.max_texture_size.width = info.max_texture_width;
        data.max_texture_size.height = info.max_texture_height;
    }
    data.paused = 0;
   
#ifdef MAX_TEXTURE_SIZE
#ifdef __EMSCRIPTEN__
    int is_android = EM_ASM_INT(return navigator.userAgent.toLowerCase().indexOf("android") > -1);
    if (is_android) {
#endif
        if (data.max_texture_size.width > MAX_TEXTURE_SIZE) {
            data.max_texture_size.width = MAX_TEXTURE_SIZE;
        }
        if (data.max_texture_size.height > MAX_TEXTURE_SIZE) {
            data.max_texture_size.height = MAX_TEXTURE_SIZE;
        }
#ifdef __EMSCRIPTEN__
    }
#endif
#endif

    SDL_SetRenderDrawColor(data.renderer, 0, 0, 0, 0xff);

    create_renderer_interface();

    return 1;
}

static void destroy_render_texture(void)
{
    if (data.render_texture) {
        SDL_DestroyTexture(data.render_texture);
        data.render_texture = 0;
    }
}

int platform_renderer_create_render_texture(int width, int height)
{
    if (data.paused) {
        return 1;
    }
    destroy_render_texture();

#ifdef USE_TEXTURE_SCALE_MODE
    if (!HAS_TEXTURE_SCALE_MODE) {
#endif
        const char *scale_quality = "linear";
#ifndef __APPLE__
        // Scale using nearest neighbour when we scale a multiple of 100%: makes it look sharper.
        // But not on MacOS: users are used to the linear interpolation since that's what Apple also does.
        if (platform_screen_get_scale() % 100 == 0) {
            scale_quality = "nearest";
        }
#endif
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, scale_quality);
#ifdef USE_TEXTURE_SCALE_MODE
    }
#endif

    SDL_SetRenderTarget(data.renderer, NULL);
    SDL_RenderSetLogicalSize(data.renderer, width, height);

    data.render_texture = SDL_CreateTexture(data.renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_TARGET, width, height);
    if (data.render_texture) {
        SDL_Log("Render texture created (%d x %d)", width, height);
        SDL_SetRenderTarget(data.renderer, data.render_texture);
        SDL_SetRenderDrawBlendMode(data.renderer, SDL_BLENDMODE_BLEND);

#ifdef USE_TEXTURE_SCALE_MODE
        if (HAS_TEXTURE_SCALE_MODE) {
            SDL_ScaleMode scale_quality = SDL_ScaleModeLinear;
#ifndef __APPLE__
            if (platform_screen_get_scale() % 100 == 0) {
                scale_quality = SDL_ScaleModeNearest;
            }
#endif
            SDL_SetTextureScaleMode(data.render_texture, scale_quality);
        } else {
#endif
            SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
#ifdef USE_TEXTURE_SCALE_MODE
        }
#endif

        return 1;
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to create render texture: %s", SDL_GetError());
        return 0;
    }
}

int platform_renderer_lost_render_texture(void)
{
    return !data.render_texture && data.renderer;
}

void platform_renderer_invalidate_target_textures(void)
{
    if (data.custom_textures[CUSTOM_IMAGE_RED_FOOTPRINT].texture) {
        SDL_DestroyTexture(data.custom_textures[CUSTOM_IMAGE_RED_FOOTPRINT].texture);
        data.custom_textures[CUSTOM_IMAGE_RED_FOOTPRINT].texture = 0;
        create_blend_texture(CUSTOM_IMAGE_RED_FOOTPRINT);
    }
    if (data.custom_textures[CUSTOM_IMAGE_GREEN_FOOTPRINT].texture) {
        SDL_DestroyTexture(data.custom_textures[CUSTOM_IMAGE_GREEN_FOOTPRINT].texture);
        data.custom_textures[CUSTOM_IMAGE_GREEN_FOOTPRINT].texture = 0;
        create_blend_texture(CUSTOM_IMAGE_GREEN_FOOTPRINT);
    }
}

void platform_renderer_clear(void)
{
    clear_screen();
}

#ifdef PLATFORM_USE_SOFTWARE_CURSOR
static void draw_software_mouse_cursor(void)
{
    const mouse *mouse = mouse_get();
    if (!mouse->is_touch) {
        cursor_shape current = platform_cursor_get_current_shape();
        if (current == CURSOR_DISABLED) {
            return;
        }
        int size = calc_adjust_with_percentage(data.cursors[current].size,
            calc_percentage(100, platform_screen_get_scale()));
        SDL_Rect dst;
        dst.x = mouse->x - data.cursors[current].hotspot.x;
        dst.y = mouse->y - data.cursors[current].hotspot.y;
        dst.w = size;
        dst.h = size;
        SDL_RenderCopy(data.renderer, data.cursors[current].texture, NULL, &dst);
    }
}
#endif

void platform_renderer_render(void)
{
    if (data.paused) {
        return;
    }
    SDL_SetRenderTarget(data.renderer, NULL);
    SDL_RenderCopy(data.renderer, data.render_texture, NULL, NULL);
#ifdef PLATFORM_USE_SOFTWARE_CURSOR
    draw_software_mouse_cursor();
#endif
    SDL_RenderPresent(data.renderer);
    SDL_SetRenderTarget(data.renderer, data.render_texture);
}

void platform_renderer_generate_mouse_cursor_texture(int cursor_id, int size, const color_t *pixels,
    int hotspot_x, int hotspot_y)
{
    if (data.paused) {
        return;
    }
    if (data.cursors[cursor_id].texture) {
        SDL_DestroyTexture(data.cursors[cursor_id].texture);
        SDL_memset(&data.cursors[cursor_id], 0, sizeof(data.cursors[cursor_id]));
    }
    data.cursors[cursor_id].texture = SDL_CreateTexture(data.renderer,
        SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC,
        size, size);
    if (!data.cursors[cursor_id].texture) {
        return;
    }
    SDL_UpdateTexture(data.cursors[cursor_id].texture, NULL, pixels, size * sizeof(color_t));
    data.cursors[cursor_id].hotspot.x = hotspot_x;
    data.cursors[cursor_id].hotspot.y = hotspot_y;
    data.cursors[cursor_id].size = size;
    SDL_SetTextureBlendMode(data.cursors[cursor_id].texture, SDL_BLENDMODE_BLEND);
}

void platform_renderer_pause(void)
{
    SDL_SetRenderTarget(data.renderer, NULL);
    data.paused = 1;
}

void platform_renderer_resume(void)
{
    data.paused = 0;
    platform_renderer_create_render_texture(screen_width(), screen_height());
    SDL_SetRenderTarget(data.renderer, data.render_texture);
}

void platform_renderer_destroy(void)
{
    destroy_render_texture();
    if (data.renderer) {
        SDL_DestroyRenderer(data.renderer);
        data.renderer = 0;
    }
}

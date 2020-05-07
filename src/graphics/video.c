#include "video.h"

#include "core/dir.h"
#include "core/file.h"
#include "core/smacker.h"
#include "core/time.h"
#include "game/settings.h"
#include "graphics/graphics.h"
#include "sound/device.h"
#include "sound/music.h"
#include "sound/speech.h"

static struct {
    int is_playing;
    int is_ended;

    smacker s;
    struct {
        int width;
        int height;
        int y_scale;
        int micros_per_frame;
        time_millis start_render_millis;
        int current_frame;
    } video;
    struct {
        int has_audio;
        int bitdepth;
        int channels;
        int rate;
    } audio;
} data;

static void close_smk(void)
{
    if (data.s) {
        smacker_close(data.s);
        data.s = 0;
    }
}

static int load_smk(const char *filename)
{
    const char *path = dir_get_file(filename, MAY_BE_LOCALIZED);
    if (!path) {
        return 0;
    }
    FILE *fp = file_open(path, "rb");
    data.s = smacker_open(fp);
    if (!data.s) {
        // smacker_open() closes the stream on error: no need to close fp
        return 0;
    }

    int width, height, y_scale, micros_per_frame;
    smacker_get_frames_info(data.s, 0, &micros_per_frame);
    smacker_get_video_info(data.s, &width, &height, &y_scale);

    data.video.width = width;
    data.video.height = y_scale == SMACKER_Y_SCALE_NONE ? height : height * 2;
    data.video.y_scale = y_scale;
    data.video.current_frame = 0;
    data.video.micros_per_frame = micros_per_frame;

    data.audio.has_audio = 0;
    if (setting_sound(SOUND_EFFECTS)->enabled) {
        int has_track, channels, bitdepth, rate;
        smacker_get_audio_info(data.s, 0, &has_track, &channels, &bitdepth, &rate);
        if (has_track) {
            data.audio.has_audio = 1;
            data.audio.bitdepth = bitdepth;
            data.audio.channels = channels;
            data.audio.rate = rate;
        }
    }

    if (smacker_first_frame(data.s) != SMACKER_FRAME_OK) {
        close_smk();
        return 0;
    }
    return 1;
}

static void end_video(void)
{
    sound_device_use_default_music_player();
    sound_music_update(1);
}

int video_start(const char *filename)
{
    data.is_playing = 0;
    data.is_ended = 0;

    if (load_smk(filename)) {
        sound_music_stop();
        sound_speech_stop();
        data.is_playing = 1;
        return 1;
    } else {
        return 0;
    }
}

void video_size(int *width, int *height)
{
    *width = data.video.width;
    *height = data.video.y_scale == SMACKER_Y_SCALE_NONE ? data.video.height : 2 * data.video.height;
}

void video_init(void)
{
    data.video.start_render_millis = time_get_millis();

    if (data.audio.has_audio) {
        int audio_len = smacker_get_frame_audio_size(data.s, 0);
        if (audio_len > 0) {
            sound_device_use_custom_music_player(
                data.audio.bitdepth, data.audio.channels, data.audio.rate,
                smacker_get_frame_audio(data.s, 0), audio_len
            );
        }
    }
}

int video_is_finished(void)
{
    return data.is_ended;
}

void video_stop(void)
{
    if (data.is_playing) {
        if (!data.is_ended) {
            end_video();
        }
        close_smk();
        data.is_playing = 0;
    }
}

void video_shutdown(void)
{
    if (data.is_playing) {
        close_smk();
        data.is_playing = 0;
    }
}

void video_draw(int x_offset, int y_offset)
{
    if (!data.s) {
        return;
    }
    time_millis now_millis = time_get_millis();

    int frame_no = (now_millis - data.video.start_render_millis) * 1000 / data.video.micros_per_frame;
    int draw_frame = data.video.current_frame == 0;
    while (frame_no > data.video.current_frame) {
        if (smacker_next_frame(data.s) != SMACKER_FRAME_OK) {
            close_smk();
            data.is_ended = 1;
            data.is_playing = 0;
            end_video();
            return;
        }
        data.video.current_frame++;
        draw_frame = 1;

        if (data.audio.has_audio) {
            int audio_len = smacker_get_frame_audio_size(data.s, 0);
            if (audio_len > 0) {
                sound_device_write_custom_music_data(smacker_get_frame_audio(data.s, 0), audio_len);
            }
        }
    }
    if (!draw_frame) {
        return;
    }
    const clip_info *clip = graphics_get_clip_info(x_offset, y_offset, data.video.width, data.video.height);
    if (!clip->is_visible) {
        return;
    }
    const unsigned char *frame = smacker_get_frame_video(data.s);
    const uint32_t *pal = smacker_get_frame_palette(data.s);
    if (frame && pal) {
        for (int y = clip->clipped_pixels_top; y < clip->visible_pixels_y; y++) {
            color_t *pixel = graphics_get_pixel(x_offset + clip->clipped_pixels_left, y + y_offset + clip->clipped_pixels_top);
            int video_y = data.video.y_scale == SMACKER_Y_SCALE_NONE ? y : y / 2;
            const unsigned char *line = frame + (video_y * data.video.width);
            for (int x = clip->clipped_pixels_left; x < clip->visible_pixels_x; x++) {
                *pixel = ALPHA_OPAQUE | pal[line[x]];
                ++pixel;
            }
        }
    }
}

#include "video.h"

#include "core/dir.h"
#include "core/file.h"
#include "core/time.h"
#include "game/settings.h"
#include "graphics/graphics.h"
#include "sound/device.h"
#include "sound/music.h"
#include "sound/speech.h"

#include "libsmacker/smacker.h"

static struct {
    int is_playing;
    int is_ended;

    smk s;
    struct {
        int width;
        int height;
        int micros_per_frame;
        time_millis start_render_millis;
        int current_frame;
        int total_frames;
    } video;
    struct {
        int bitdepth;
        int channels;
        int rate;
    } audio;
} data;

static void close_smk(void)
{
    if (data.s) {
        smk_close(data.s);
        data.s = 0;
    }
}

static int load_smk(const char *filename)
{
    const char *path = dir_get_case_corrected_file(filename);
    if (!path) {
        return 0;
    }
    FILE *fp = file_open(path, "rb");
#ifdef __vita__
    // Vita file i/o is too slow to play videos smoothly from disk, so pre-load
    // the pre-loading causes a short pause before video starts
    data.s = smk_open_filepointer(fp, SMK_MODE_MEMORY);
#else
    data.s = smk_open_filepointer(fp, SMK_MODE_DISK);
#endif
    if (!data.s) {
        // smk_open_filepointer closes the stream on error: no need to close fp
        return 0;
    }

    unsigned long width, height, frames;
    double micros_per_frame;
    unsigned char track_masks, channels[7], bitdepths[7];
    unsigned long bitrates[7];
    smk_info_all(data.s, 0, &frames, &micros_per_frame);
    smk_info_video(data.s, &width, &height, 0);
    smk_info_audio(data.s, &track_masks, channels, bitdepths, bitrates);

    data.video.width = width;
    data.video.height = height;
    data.video.current_frame = 0;
    data.video.total_frames = frames;
    data.video.micros_per_frame = (int) (micros_per_frame);

    smk_enable_video(data.s, 1);

    if (setting_sound(SOUND_EFFECTS)->enabled && track_masks & 1) {
        // only play sound when sound effects are enabled and track 0 is available
        smk_enable_audio(data.s, 0, 1);

        data.audio.bitdepth = bitdepths[0];
        data.audio.channels = channels[0];
        data.audio.rate = bitrates[0];
    }

    if (smk_first(data.s) < 0) {
        close_smk();
        return 0;
    }
    return 1;
}

static void end_video(void)
{
    sound_device_use_default_music_player();
    sound_music_reset();
    sound_music_update();
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

void video_init(void)
{
    data.video.start_render_millis = time_get_millis();

    int audio_len = smk_get_audio_size(data.s, 0);
    if (audio_len > 0) {
        sound_device_use_custom_music_player(
            data.audio.bitdepth, data.audio.channels, data.audio.rate,
            smk_get_audio(data.s, 0), audio_len
        );
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
    if (frame_no > data.video.current_frame) {
        if (smk_next(data.s) == SMK_DONE) {
            close_smk();
            data.is_ended = 1;
            data.is_playing = 0;
            end_video();
            return;
        }
        data.video.current_frame++;

        int audio_len = smk_get_audio_size(data.s, 0);
        if (audio_len > 0) {
            sound_device_write_custom_music_data(smk_get_audio(data.s, 0), audio_len);
        }
    }
    const clip_info *clip = graphics_get_clip_info(x_offset, y_offset, data.video.width, data.video.height);
    if (!clip->is_visible) {
        return;
    }
    const unsigned char *frame = smk_get_video(data.s);
    const unsigned char *pal = smk_get_palette(data.s);
    if (frame && pal) {
        for (int y = clip->clipped_pixels_top; y < clip->visible_pixels_y; y++) {
            color_t *pixel = graphics_get_pixel(x_offset + clip->clipped_pixels_left, y + y_offset + clip->clipped_pixels_top);
            const unsigned char *line = frame + (y * data.video.width);
            for (int x = clip->clipped_pixels_left; x < clip->visible_pixels_x; x++) {
                *pixel = 0xFF000000 |
                    (pal[line[x] * 3] << 16) |
                    (pal[line[x] * 3 + 1] << 8) |
                    (pal[line[x] * 3 + 2]);
                ++pixel;
            }
        }
    }
}

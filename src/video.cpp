#include "video.h"

#include "video/smacker.h"

#include <sound>
#include <data>

#include "core/dir.h"
#include "core/time.h"
#include "game/settings.h"
#include "graphics/color.h"

static struct
{
    int isPlaying;
    int isEnded;

    struct
    {
        smk s;
        int width;
        int height;
        int microsPerFrame;
        time_millis startRenderMillis;
        int currentFrame;
        int totalFrames;
    } video;
    struct
    {
        smk s;
        int bitdepth;
        int channels;
        int rate;
        int firstFrame;
    } audio;
} data;

static void closeSmk(smk *s)
{
    smk_close(*s);
    *s = 0;
}

static void closeAll()
{
    if (data.video.s)
    {
        closeSmk(&data.video.s);
    }
    if (data.audio.s)
    {
        closeSmk(&data.audio.s);
    }
}

static const unsigned char *nextAudioFrame(int *outLen)
{
    if (!data.audio.s)
    {
        return 0;
    }
    if (data.audio.firstFrame)
    {
        data.audio.firstFrame = 0;
        if (smk_first(data.audio.s) < 0)
        {
            closeSmk(&data.audio.s);
            return 0;
        }
    }
    else if (!smk_next(data.audio.s))
    {
        closeSmk(&data.audio.s);
        return 0;
    }
    int audioLen = smk_get_audio_size(data.audio.s, 0);
    if (audioLen > 0)
    {
        *outLen = audioLen;
        return smk_get_audio(data.audio.s, 0);
    }
    return 0;
}

static int loadSmkVideo(const char *filename)
{
    data.video.s = smk_open_file(filename, SMK_MODE_DISK);
    if (!data.video.s)
    {
        return 0;
    }

    unsigned long width, height, frames;
    double microsPerFrame;
    smk_info_all(data.video.s, 0, &frames, &microsPerFrame);
    smk_info_video(data.video.s, &width, &height, 0);

    data.video.width = width;
    data.video.height = height;
    data.video.currentFrame = 0;
    data.video.totalFrames = frames;
    data.video.microsPerFrame = (int) (microsPerFrame);

    smk_enable_all(data.video.s,SMK_VIDEO_TRACK);
    if (smk_first(data.video.s) < 0)
    {
        closeSmk(&data.video.s);
        return 0;
    }
    return 1;
}

static int loadSmkAudio(const char *filename)
{
    if (!setting_sound(SOUND_EFFECTS)->enabled)
    {
        // no sound when sound effects are disabled
        return 1;
    }
    data.audio.s = smk_open_file(filename, SMK_MODE_DISK);
    if (!data.audio.s)
    {
        return 0;
    }

    unsigned char tracks, channels[7], bitdepths[7];
    unsigned long bitrates[7];
    smk_info_audio(data.audio.s, &tracks, channels, bitdepths, bitrates);

    if (tracks != 1)
    {
        // Video has alternate audio tracks, not supported
        closeSmk(&data.audio.s);
        return 0;
    }
    smk_enable_all(data.audio.s, tracks);
    if (smk_first(data.audio.s) < 0)
    {
        closeSmk(&data.audio.s);
        return 0;
    }
    data.audio.bitdepth = bitdepths[0];
    data.audio.channels = channels[0];
    data.audio.rate = bitrates[0];
    data.audio.firstFrame = 1;
    return 1;
}

int loadSmk(const char *filename)
{
    const char *path = dir_get_case_corrected_file(filename);
    if (!path)
    {
        return 0;
    }
    if (loadSmkVideo(path) && loadSmkAudio(path))
    {
        return 1;
    }
    else
    {
        closeAll();
        return 0;
    }
}

static void endVideo()
{
    sound_device_use_default_music_player();
    sound_music_reset();
    sound_music_update();
}

int Video_start(const char *filename)
{
    data.isPlaying = 0;
    data.isEnded = 0;

    if (loadSmk(filename))
    {
        sound_music_stop();
        sound_speech_stop();
        data.isPlaying = 1;
        return 1;
    }
    else
    {
        return 0;
    }
}

void Video_stop()
{
    if (data.isPlaying)
    {
        if (data.video.s)
        {
            closeAll();
        }
        data.isPlaying = 0;
        if (!data.isEnded)
        {
            endVideo();
        }
    }
}

void Video_shutdown()
{
    if (data.isPlaying)
    {
        if (data.video.s)
        {
            closeAll();
        }
        data.isPlaying = 0;
    }
}

int Video_isFinished()
{
    return data.isEnded;
}

void Video_init()
{
    data.video.startRenderMillis = time_get_millis();
    sound_device_use_custom_music_player(data.audio.bitdepth, data.audio.channels, data.audio.rate, nextAudioFrame);
}

void Video_draw(int xOffset, int yOffset)
{
    if (!data.video.s)
    {
        return;
    }
    time_millis nowMillis = time_get_millis();

    int frameNo = (nowMillis - data.video.startRenderMillis) * 1000 / data.video.microsPerFrame;
    if (frameNo > data.video.currentFrame)
    {
        if (smk_next(data.video.s) == SMK_DONE)
        {
            closeSmk(&data.video.s);
            data.isEnded = 1;
            data.isPlaying = 0;
            endVideo();
            return;
        }
        data.video.currentFrame++;
    }

    const unsigned char *frame = smk_get_video(data.video.s);
    const unsigned char *pal = smk_get_palette(data.video.s);
    if (frame && pal)
    {
        for (int y = 0; y < data.video.height; y++)
        {
            for (int x = 0; x < data.video.width; x++)
            {
                color_t color = 0xFF000000 |
                                (pal[frame[(y * data.video.width) + x] * 3] << 16) |
                                (pal[frame[(y * data.video.width) + x] * 3 + 1] << 8) |
                                (pal[frame[(y * data.video.width) + x] * 3 + 2]);
                ScreenPixel(xOffset + x, yOffset + y) = color;
            }
        }
    }
}

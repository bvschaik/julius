#include "core/file.h"
#include "core/log.h"
#include "sound/device.h"
#include "game/settings.h"
#include "SDL.h"
#include "SDL_mixer.h"
#include "platform/vita/vita.h"

#include <stdlib.h>
#include <string.h>

#define AUDIO_RATE 44100
#define AUDIO_FORMAT AUDIO_S16
#define AUDIO_CHANNELS 2
#define AUDIO_BUFFERS 1024

#define MAX_CHANNELS 150

#if SDL_VERSION_ATLEAST(2, 0, 7) 
#define USE_SDL_AUDIOSTREAM 
#endif

static int initialized = 0;
static Mix_Music *music = 0;
static Mix_Chunk *channels[MAX_CHANNELS];
static struct {
#ifdef USE_SDL_AUDIOSTREAM
    SDL_AudioStream *stream;
#else
    SDL_AudioCVT cvt;
    unsigned char *buffer;
    int buffer_size;
    int cur_read;
    int cur_write;
#endif
    SDL_AudioFormat format;
} custom_music;


static int percentage_to_volume(int percentage)
{
    return percentage * SDL_MIX_MAXVOLUME / 100;
}

void sound_device_open(void)
{
    if (0 == Mix_OpenAudio(AUDIO_RATE, AUDIO_FORMAT, AUDIO_CHANNELS, AUDIO_BUFFERS)) {
        initialized = 1;
        for (int i = 0; i < MAX_CHANNELS; i++) {
            channels[i] = 0;
        }
    } else {
        log_error("SOUND: not initialized", 0, 0);
    }
}

void sound_device_close(void)
{
    if (initialized) {
        for (int i = 0; i < MAX_CHANNELS; i++) {
            sound_device_stop_channel(i);
        }
        Mix_CloseAudio();
        initialized = 0;
    }
}

void sound_device_init_channels(int num_channels, char filenames[][CHANNEL_FILENAME_MAX])
{
    if (initialized) {
        if (num_channels > MAX_CHANNELS) {
            num_channels = MAX_CHANNELS;
        }
        Mix_AllocateChannels(num_channels);
        for (int i = 0; i < num_channels; i++) {
            if (filenames[i][0]) {
#ifdef __vita__
                FILE *fp = file_open(filenames[i], "rb");
                SDL_RWops *sdl_fp = SDL_RWFromFP(fp, SDL_TRUE);

                channels[i] = Mix_LoadWAV_RW(sdl_fp, 1);
#else
                channels[i] = Mix_LoadWAV(filenames[i]);
#endif
            }
        }
    }
}

int sound_device_has_channel(int channel)
{
    return channels[channel] ? 1 : 0;
}

int sound_device_is_channel_playing(int channel)
{
    return Mix_Playing(channel);
}

void sound_device_set_music_volume(int volume_percentage)
{
    Mix_VolumeMusic(percentage_to_volume(volume_percentage));
}

void sound_device_set_channel_volume(int channel, int volume_percentage)
{
    if (channels[channel]) {
        Mix_VolumeChunk(channels[channel], percentage_to_volume(volume_percentage));
    }
}

void sound_device_set_channel_panning(int channel, int left_pct, int right_pct)
{
    if (channels[channel]) {
        Mix_SetPanning(channel, left_pct * 255 / 100, right_pct * 255 / 100);
    }
}

int sound_device_play_music(const char *filename)
{
    if (initialized) {
        sound_device_stop_music();

        #ifdef __vita__
        char *resolved_filename = vita_prepend_path(filename); // There is no Mix_LoadMUS equivalent for fp
        #else
        const char *resolved_filename = filename;
        #endif

        music = Mix_LoadMUS(resolved_filename);
        if (music) {
            Mix_PlayMusic(music, -1);
        }

        #ifdef __vita__
        free(resolved_filename);
        #endif

        return music ? 1 : 0;
    }
    return 0;
}

void sound_device_play_file_on_channel(const char *filename, int channel)
{
    if (initialized) {
        if (channels[channel]) {
            sound_device_stop_channel(channel);
        }
#ifdef __vita__
        FILE *fp = file_open(filename, "rb");
        SDL_RWops *sdl_fp = SDL_RWFromFP(fp, SDL_TRUE);

        channels[channel] = Mix_LoadWAV_RW(sdl_fp, 1);
#else
        channels[channel] = Mix_LoadWAV(filename);
#endif
        if (channels[channel]) {
            Mix_PlayChannel(channel, channels[channel], 0);
        }
    }
}

void sound_device_play_channel(int channel)
{
    if (initialized) {
        if (channels[channel]) {
            Mix_PlayChannel(channel, channels[channel], 0);
        }
    }
}

void sound_device_stop_music(void)
{
    if (initialized) {
        if (music) {
            Mix_HaltMusic();
            Mix_FreeMusic(music);
            music = 0;
        }
    }
}

void sound_device_stop_channel(int channel)
{
    if (initialized) {
        if (channels[channel]) {
            Mix_HaltChannel(channel);
            Mix_FreeChunk(channels[channel]);
            channels[channel] = 0;
        }
    }
}

static void free_custom_audio_stream(void)
{
#ifdef USE_SDL_AUDIOSTREAM
    if (custom_music.stream) {
        SDL_FreeAudioStream(custom_music.stream);
        custom_music.stream = 0;
    }
#else
    if (custom_music.buffer) {
        free(custom_music.buffer);
        custom_music.buffer = 0;
    }
#endif
}

static int create_custom_audio_stream(SDL_AudioFormat src_format, Uint8 src_channels, int src_rate,
                                      SDL_AudioFormat dst_format, Uint8 dst_channels, int dst_rate)
{
    free_custom_audio_stream();

#ifdef USE_SDL_AUDIOSTREAM
    custom_music.stream = SDL_NewAudioStream(
        src_format, src_channels, src_rate,
        dst_format, dst_channels, dst_rate
    );
    if (!custom_music.stream) {
        return 0;
    }
#else
    int result = SDL_BuildAudioCVT(
        &custom_music.cvt, src_format, src_channels, src_rate,
        dst_format, dst_channels, dst_rate
    );
    if (result < 0) {
        return 0;
    }

    // Allocate buffer large enough for 2 seconds of 16-bit audio
    custom_music.buffer_size = dst_rate * dst_channels * 2 * 2;
    custom_music.buffer = malloc(custom_music.buffer_size);
    if (!custom_music.buffer) {
        return 0;
    }
    custom_music.cur_read = 0;
    custom_music.cur_write = 0;
#endif

    return 1;
}

static int custom_audio_stream_active(void)
{
#ifdef USE_SDL_AUDIOSTREAM
    return custom_music.stream != 0;
#else
    return custom_music.buffer != 0;
#endif
}

static int put_custom_audio_stream(Uint8 *data, int len)
{
    if (!data || len <= 0 || !custom_audio_stream_active()) {
        return 0;
    }

#ifdef USE_SDL_AUDIOSTREAM
    return SDL_AudioStreamPut(custom_music.stream, data, len) == 0;
#else
    // Convert audio to SDL format
    custom_music.cvt.buf = (Uint8*)malloc((size_t)(len * custom_music.cvt.len_mult));
    if (!custom_music.cvt.buf) {
        return 0;
    }
    memcpy(custom_music.cvt.buf, data, len);
    custom_music.cvt.len = len;
    SDL_ConvertAudio(&custom_music.cvt);
    int converted_len = custom_music.cvt.len_cvt;

    // Copy data to circular buffer
    if (converted_len + custom_music.cur_write <= custom_music.buffer_size) {
        memcpy(&custom_music.buffer[custom_music.cur_write], custom_music.cvt.buf, converted_len);
    } else {
        int end_len = custom_music.buffer_size - custom_music.cur_write;
        memcpy(&custom_music.buffer[custom_music.cur_write], custom_music.cvt.buf, end_len);
        memcpy(custom_music.buffer, &custom_music.cvt.buf[end_len], converted_len - end_len);
    }
    custom_music.cur_write = (custom_music.cur_write + converted_len) % custom_music.buffer_size;

    // Clean up
    free(custom_music.cvt.buf);
    custom_music.cvt.buf = 0;
    custom_music.cvt.len = 0;

    return 1;
#endif
}

static int get_custom_audio_stream(Uint8 *dst, int len)
{
    if (!dst || len <= 0 || !custom_audio_stream_active()) {
        return 0;
    }

#ifdef USE_SDL_AUDIOSTREAM
    return SDL_AudioStreamGet(custom_music.stream, dst, len);
#else
    int bytes_copied = 0;
    if (custom_music.cur_read < custom_music.cur_write) {
        int bytes_available = custom_music.cur_write - custom_music.cur_read;
        int bytes_to_copy = bytes_available < len ? bytes_available : len;
        memcpy(dst, &custom_music.buffer[custom_music.cur_read], bytes_to_copy);
        bytes_copied = bytes_to_copy;
    } else {
        int bytes_available = custom_music.buffer_size - custom_music.cur_read;
        int bytes_to_copy = bytes_available < len ? bytes_available : len;
        memcpy(dst, &custom_music.buffer[custom_music.cur_read], bytes_to_copy);
        bytes_copied = bytes_to_copy;
        if (bytes_copied < len) {
            int second_part_len = len - bytes_copied;
            bytes_available = custom_music.cur_write;
            bytes_to_copy = bytes_available < second_part_len ? bytes_available : second_part_len;
            memcpy(&dst[bytes_copied], custom_music.buffer, bytes_to_copy);
            bytes_copied += bytes_to_copy;
        }
    }
    custom_music.cur_read = (custom_music.cur_read + bytes_copied) % custom_music.buffer_size;

    return bytes_copied;
#endif
}

static void custom_music_callback(void* dummy, Uint8* stream, int len)
{
    int bytes_copied = get_custom_audio_stream(stream, len);

    if (bytes_copied < len) {
        // end of stream, write silence
        memset(&stream[bytes_copied], 0, len - bytes_copied);
    }
}

void sound_device_use_custom_music_player(int bitdepth, int num_channels, int rate,
                                          const unsigned char *data, int len)
{
    SDL_AudioFormat format;
    if (bitdepth == 8) {
        format = AUDIO_U8;
    } else if (bitdepth == 16) {
        format = AUDIO_S16;
    } else {
        log_error("Custom music bitdepth not supported:", 0, bitdepth);
        return;
    }
    int device_rate;
    Uint16 device_format;
    int device_channels;
    Mix_QuerySpec(&device_rate, &device_format, &device_channels);
    custom_music.format = format;

    int result = create_custom_audio_stream(
        format, num_channels, rate,
        device_format, device_channels, device_rate
    );
    if (!result) {
        return;
    }

    sound_device_write_custom_music_data(data, len);

    Mix_HookMusic(custom_music_callback, 0);
}

void sound_device_write_custom_music_data(const unsigned char *data, int len)
{
    if (!data || len <= 0 || !custom_audio_stream_active()) {
        return;
    }
    // Mix audio to sound effect volume
    Uint8 *mix_buffer = (Uint8*)malloc(len);
    if (!mix_buffer) {
        return;
    }
    memset(mix_buffer, (custom_music.format == AUDIO_U8) ? 128 : 0, len);
    SDL_MixAudioFormat(mix_buffer, data,
        custom_music.format, len,
        percentage_to_volume(setting_sound(SOUND_EFFECTS)->volume));

    put_custom_audio_stream(mix_buffer, len);
    free(mix_buffer);
}

void sound_device_use_default_music_player(void)
{
    Mix_HookMusic(0, 0);
    free_custom_audio_stream();
}

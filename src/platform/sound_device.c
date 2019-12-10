#include "core/file.h"
#include "core/log.h"
#include "sound/device.h"
#include "game/settings.h"
#include "platform/android/android.h"
#include "platform/vita/vita.h"
#include "SDL.h"
#include "SDL_mixer.h"

#include <stdlib.h>
#include <string.h>

#define AUDIO_RATE 22050
#define AUDIO_FORMAT AUDIO_S16
#define AUDIO_CHANNELS 2
#define AUDIO_BUFFERS 1024

#define MAX_CHANNELS 150

#if SDL_VERSION_ATLEAST(2, 0, 7) 
#define USE_SDL_AUDIOSTREAM 
#endif

typedef struct {
    const char *filename;
    Mix_Chunk *chunk;
} sound_channel;

static struct {
    int initialized;
    Mix_Music *music;
    sound_channel channels[MAX_CHANNELS];
} data;

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
        data.initialized = 1;
        for (int i = 0; i < MAX_CHANNELS; i++) {
            data.channels[i].chunk = 0;
        }
    } else {
        log_error("SOUND: not initialized", 0, 0);
    }
}

void sound_device_close(void)
{
    if (data.initialized) {
        for (int i = 0; i < MAX_CHANNELS; i++) {
            sound_device_stop_channel(i);
        }
        Mix_CloseAudio();
        data.initialized = 0;
    }
}

static Mix_Chunk *load_chunk(const char *filename)
{
    if (filename[0]) {
#ifdef __vita__
        FILE *fp = file_open(filename, "rb");
        SDL_RWops *sdl_fp = SDL_RWFromFP(fp, SDL_TRUE);
        return Mix_LoadWAV_RW(sdl_fp, 1);
#elif ANDROID
        char resolved_filename[FILE_NAME_MAX];
        sprintf(resolved_filename, "%s/%s", android_get_c3_path(), filenames[i]);
        return Mix_LoadWAV(resolved_filename);
#else
        return Mix_LoadWAV(filename);
#endif
    } else {
        return NULL;
    }
}

static int load_channel(sound_channel *channel)
{
    if (!channel->chunk && channel->filename) {
        channel->chunk = load_chunk(channel->filename);
    }
    return channel->chunk ? 1 : 0;
}

void sound_device_init_channels(int num_channels, char filenames[][CHANNEL_FILENAME_MAX])
{
    if (data.initialized) {
        if (num_channels > MAX_CHANNELS) {
            num_channels = MAX_CHANNELS;
        }
        Mix_AllocateChannels(num_channels);
        log_info("Loading audio files", 0, 0);
        for (int i = 0; i < num_channels; i++) {
            data.channels[i].chunk = 0;
            data.channels[i].filename = filenames[i][0] ? filenames[i] : 0;
        }
    }
}

int sound_device_is_channel_playing(int channel)
{
    return data.channels[channel].chunk && Mix_Playing(channel);
}

void sound_device_set_music_volume(int volume_pct)
{
    Mix_VolumeMusic(percentage_to_volume(volume_pct));
}

void sound_device_set_channel_volume(int channel, int volume_pct)
{
    if (data.channels[channel].chunk) {
        Mix_VolumeChunk(data.channels[channel].chunk, percentage_to_volume(volume_pct));
    }
}

int sound_device_play_music(const char *filename, int volume_pct)
{
    if (data.initialized) {
        sound_device_stop_music();

        #ifdef __vita__
        char *resolved_filename = vita_prepend_path(filename); // There is no Mix_LoadMUS equivalent for fp
        #elif __ANDROID__
        char resolved_filename[FILE_NAME_MAX];
        sprintf(resolved_filename, "%s/%s", android_get_c3_path(), filename);
        #else
        const char *resolved_filename = filename;
        #endif

        data.music = Mix_LoadMUS(resolved_filename);
        if (!data.music) {
            SDL_LogWarn(SDL_LOG_CATEGORY_AUDIO, "Error opening music file '%s'. Reason: %s", resolved_filename, Mix_GetError());
        } else {
            if (Mix_PlayMusic(data.music, -1) == -1) {
                data.music = 0;
                SDL_LogWarn(SDL_LOG_CATEGORY_AUDIO, "Error playing music file '%s'. Reason: %s", resolved_filename, Mix_GetError());
            } else {
                sound_device_set_music_volume(volume_pct);
            }
        }

        #ifdef __vita__
        free(resolved_filename);
        #endif

        return data.music ? 1 : 0;
    }
    return 0;
}

void sound_device_play_file_on_channel(const char *filename, int channel, int volume_pct)
{
    if (data.initialized) {
        sound_device_stop_channel(channel);
        data.channels[channel].chunk = load_chunk(filename);
        if (data.channels[channel].chunk) {
            sound_device_set_channel_volume(channel, volume_pct);
            Mix_PlayChannel(channel, data.channels[channel].chunk, 0);
        }
    }
}

void sound_device_play_channel(int channel, int volume_pct)
{
    if (data.initialized) {
        sound_channel *ch = &data.channels[channel];
        if (load_channel(ch)) {
            sound_device_set_channel_volume(channel, volume_pct);
            Mix_PlayChannel(channel, ch->chunk, 0);
        }
    }
}

void sound_device_play_channel_panned(int channel, int volume_pct, int left_pct, int right_pct)
{
    if (data.initialized) {
        sound_channel *ch = &data.channels[channel];
        if (load_channel(ch)) {
            Mix_SetPanning(channel, left_pct * 255 / 100, right_pct * 255 / 100);
            sound_device_set_channel_volume(channel, volume_pct);
            Mix_PlayChannel(channel, ch->chunk, 0);
        }
    }
}

void sound_device_stop_music(void)
{
    if (data.initialized) {
        if (data.music) {
            Mix_HaltMusic();
            Mix_FreeMusic(data.music);
            data.music = 0;
        }
    }
}

void sound_device_stop_channel(int channel)
{
    if (data.initialized) {
        sound_channel *ch = &data.channels[channel];
        if (ch->chunk) {
            Mix_HaltChannel(channel);
            Mix_FreeChunk(ch->chunk);
            ch->chunk = 0;
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

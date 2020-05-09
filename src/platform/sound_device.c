#include "core/file.h"
#include "core/log.h"
#include "sound/device.h"
#include "game/settings.h"
#include "SDL.h"
#include "SDL_mixer.h"
#include "platform/platform.h"
#include "platform/vita/vita.h"

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
#define HAS_AUDIOSTREAM() (platform_sdl_version_at_least(2, 0, 7))

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
    SDL_AudioFormat format;
#ifdef USE_SDL_AUDIOSTREAM
    SDL_AudioStream *stream;
    int use_audiostream;
#endif
    SDL_AudioCVT cvt;
    unsigned char *buffer;
    int buffer_size;
    int cur_read;
    int cur_write;
} custom_music;

static int percentage_to_volume(int percentage)
{
    return percentage * SDL_MIX_MAXVOLUME / 100;
}

static void init_channels(void)
{
    data.initialized = 1;
    for (int i = 0; i < MAX_CHANNELS; i++) {
        data.channels[i].chunk = 0;
    }
}

void sound_device_open(void)
{
#ifdef USE_SDL_AUDIOSTREAM
    custom_music.use_audiostream = HAS_AUDIOSTREAM();
#endif
    if (0 == Mix_OpenAudio(AUDIO_RATE, AUDIO_FORMAT, AUDIO_CHANNELS, AUDIO_BUFFERS)) {
        init_channels();
        return;
    }
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Sound failed to initialize using default driver: %s", Mix_GetError());
    // Try to work around SDL choosing the wrong driver on Windows sometimes
    for (int i = 0; i < SDL_GetNumAudioDrivers(); i++) {
        const char *driver_name = SDL_GetAudioDriver(i);
        if (SDL_strcmp(driver_name, "disk") == 0 || SDL_strcmp(driver_name, "dummy") == 0) {
            // Skip "write-to-disk" and dummy drivers
            continue;
        }
        if (0 == SDL_AudioInit(driver_name) &&
            0 == Mix_OpenAudio(AUDIO_RATE, AUDIO_FORMAT, AUDIO_CHANNELS, AUDIO_BUFFERS)) {
            SDL_Log("Using audio driver: %s", driver_name);
            init_channels();
            return;
        } else {
            SDL_Log("Not using audio driver %s, reason: %s", driver_name, SDL_GetError());
        }
    }
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Sound failed to initialize: %s", Mix_GetError());
    int max = SDL_GetNumAudioDevices(0);
    SDL_Log("Number of audio devices: %d", max);
    for (int i = 0; i < max; i++) {
        SDL_Log("Audio device: %s", SDL_GetAudioDeviceName(i, 0));
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
        if (!fp) {
            return NULL;
        }
        SDL_RWops *sdl_fp = SDL_RWFromFP(fp, SDL_TRUE);
        return Mix_LoadWAV_RW(sdl_fp, 1);
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
        FILE *fp = file_open(filename, "rb");
        if (!fp) {
            return 0;
        }
        SDL_RWops *sdl_fp = SDL_RWFromFP(fp, SDL_TRUE);
        data.music = Mix_LoadMUSType_RW(sdl_fp, file_has_extension(filename, "mp3") ? MUS_MP3 : MUS_WAV, SDL_TRUE);
#else
        data.music = Mix_LoadMUS(filename);
#endif
        if (!data.music) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Error opening music file '%s'. Reason: %s", filename, Mix_GetError());
        } else {
            if (Mix_PlayMusic(data.music, -1) == -1) {
                data.music = 0;
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Error playing music file '%s'. Reason: %s", filename, Mix_GetError());
            } else {
                sound_device_set_music_volume(volume_pct);
            }
        }
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
    if (custom_music.use_audiostream) {
        if (custom_music.stream) {
            SDL_FreeAudioStream(custom_music.stream);
            custom_music.stream = 0;
        }
        return;
    }
#endif

    if (custom_music.buffer) {
        free(custom_music.buffer);
        custom_music.buffer = 0;
    }
}

static int create_custom_audio_stream(SDL_AudioFormat src_format, Uint8 src_channels, int src_rate,
                                      SDL_AudioFormat dst_format, Uint8 dst_channels, int dst_rate)
{
    free_custom_audio_stream();

#ifdef USE_SDL_AUDIOSTREAM
    if (custom_music.use_audiostream) {
        custom_music.stream = SDL_NewAudioStream(
            src_format, src_channels, src_rate,
            dst_format, dst_channels, dst_rate
        );
        return custom_music.stream != 0;
    }
#endif

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
    return 1;
}

static int custom_audio_stream_active(void)
{
#ifdef USE_SDL_AUDIOSTREAM
    if (custom_music.use_audiostream) {
        return custom_music.stream != 0;
    }
#endif
    return custom_music.buffer != 0;
}

static int put_custom_audio_stream(Uint8 *audio_data, int len)
{
    if (!audio_data || len <= 0 || !custom_audio_stream_active()) {
        return 0;
    }

#ifdef USE_SDL_AUDIOSTREAM
    if (custom_music.use_audiostream) {
        return SDL_AudioStreamPut(custom_music.stream, audio_data, len) == 0;
    }
#endif

    // Convert audio to SDL format
    custom_music.cvt.buf = (Uint8*)malloc((size_t)(len * custom_music.cvt.len_mult));
    if (!custom_music.cvt.buf) {
        return 0;
    }
    memcpy(custom_music.cvt.buf, audio_data, len);
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
}

static int get_custom_audio_stream(Uint8 *dst, int len)
{
    if (!dst || len <= 0 || !custom_audio_stream_active()) {
        return 0;
    }

#ifdef USE_SDL_AUDIOSTREAM
    if (custom_music.use_audiostream) {
        return SDL_AudioStreamGet(custom_music.stream, dst, len);
    }
#endif

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
                                          const unsigned char *audio_data, int len)
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

    sound_device_write_custom_music_data(audio_data, len);

    Mix_HookMusic(custom_music_callback, 0);
}

void sound_device_write_custom_music_data(const unsigned char *audio_data, int len)
{
    if (!audio_data || len <= 0 || !custom_audio_stream_active()) {
        return;
    }
    // Mix audio to sound effect volume
    Uint8 *mix_buffer = (Uint8*)malloc(len);
    if (!mix_buffer) {
        return;
    }
    memset(mix_buffer, (custom_music.format == AUDIO_U8) ? 128 : 0, len);
    SDL_MixAudioFormat(mix_buffer, audio_data,
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

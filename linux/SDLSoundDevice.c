#include "SoundDevice.h"
#include "SDL.h"
#include "SDL_mixer.h"

#include <stdio.h>

#define AUDIO_RATE 22050
#define AUDIO_FORMAT AUDIO_S16MSB
#define AUDIO_CHANNELS 2
#define AUDIO_BUFFERS 4096

#define MAX_CHANNELS 150

static int initialized = 0;
static Mix_Music *music = 0;
static Mix_Chunk *channels[MAX_CHANNELS];
static struct
{
    SDL_AudioCVT cvt;
    const unsigned char *(*callback)(int *);
    unsigned char *data;
    int cur;
    int len;
} customMusic;


static int percentageToVolume(int percentage)
{
    return percentage * 128 / 100;
}

void SoundDevice_open()
{
    if (0 == Mix_OpenAudio(AUDIO_RATE, AUDIO_FORMAT, AUDIO_CHANNELS, AUDIO_BUFFERS))
    {
        initialized = 1;
        for (int i = 0; i < MAX_CHANNELS; i++)
        {
            channels[i] = 0;
        }
        printf("SOUND: OK\n");
    }
    else
    {
        printf("SOUND: not initialized\n");
    }
}

void SoundDevice_close()
{
    if (initialized)
    {
        for (int i = 0; i < MAX_CHANNELS; i++)
        {
            SoundDevice_stopChannel(i);
        }
        Mix_CloseAudio();
        initialized = 0;
    }
}

void SoundDevice_initChannels(int numChannels, const char filenames[][32])
{
    if (initialized)
    {
        if (numChannels > MAX_CHANNELS)
        {
            numChannels = MAX_CHANNELS;
        }
        Mix_AllocateChannels(numChannels);
        for (int i = 0; i < numChannels; i++)
        {
            if (filenames[i])
            {
                channels[i] = Mix_LoadWAV(filenames[i]);
            }
        }
    }
}

int SoundDevice_hasChannel(int channel)
{
    return channels[channel] ? 1 : 0;
}
int SoundDevice_isChannelPlaying(int channel)
{
    return Mix_Playing(channel);
}

void SoundDevice_setMusicVolume(int volumePercentage)
{
    Mix_VolumeMusic(percentageToVolume(volumePercentage));
}

void SoundDevice_setChannelVolume(int channel, int volumePercentage)
{
    if (channels[channel])
    {
        Mix_VolumeChunk(channels[channel], percentageToVolume(volumePercentage));
    }
}

void SoundDevice_setChannelPanning(int channel, int leftPct, int rightPct)
{
    if (channels[channel])
    {
        Mix_SetPanning(channel, leftPct * 255 / 100, rightPct * 255 / 100);
    }
}

void SoundDevice_playMusic(const char *filename)
{
    printf("SOUND: trying music file: %s\n", filename);
    if (initialized)
    {
        SoundDevice_stopMusic();
        music = Mix_LoadMUS(filename);
        if (music)
        {
            printf("SOUND: playing music file: %s\n", filename);
            Mix_PlayMusic(music, -1);
        }
    }
}

void SoundDevice_playSoundOnChannel(const char *filename, int channel)
{
    if (initialized)
    {
        if (channels[channel])
        {
            SoundDevice_stopChannel(channel);
        }
        channels[channel] = Mix_LoadWAV(filename);
        if (channels[channel])
        {
            printf("SOUND: playing %s on channel %d\n", filename, channel);
            Mix_PlayChannel(channel, channels[channel], 0);
        }
    }
}

void SoundDevice_playChannel(int channel)
{
    if (initialized)
    {
        if (channels[channel])
        {
            printf("SOUND: playing channel %d\n", channel);
            Mix_PlayChannel(channel, channels[channel], 0);
        }
    }
}

void SoundDevice_stopMusic()
{
    if (initialized)
    {
        if (music)
        {
            Mix_HaltMusic();
            Mix_FreeMusic(music);
            music = 0;
        }
    }
}

void SoundDevice_stopChannel(int channel)
{
    if (initialized)
    {
        if (channels[channel])
        {
            Mix_HaltChannel(channel);
            Mix_FreeChunk(channels[channel]);
            channels[channel] = 0;
        }
    }
}


static int nextAudioFrame()
{
    if (customMusic.data)
    {
        free(customMusic.data);
        customMusic.data = 0;
    }

    int audioLen;
    const unsigned char *data = customMusic.callback(&audioLen);
    if (!data || audioLen <= 0)
    {
        return 0;
    }

    if (audioLen > 0)
    {
        // convert audio to SDL format
        customMusic.cvt.buf = (Uint8*) malloc(audioLen * customMusic.cvt.len_mult);
        customMusic.cvt.len = audioLen;
        memcpy(customMusic.cvt.buf, data, audioLen);
        SDL_ConvertAudio(&customMusic.cvt);
        customMusic.cur = 0;
        customMusic.len = customMusic.cvt.len_cvt;
        customMusic.data = customMusic.cvt.buf;
        customMusic.cvt.buf = 0;
        customMusic.cvt.len = 0;
    }
    return audioLen;
}

static int copyAudioFromBuffer(Uint8 *stream, int len)
{
    if (!customMusic.data || customMusic.cur >= customMusic.len)
    {
        return 0;
    }
    // push existing bytes
    int toWrite = customMusic.len - customMusic.cur;
    if (toWrite > len)
    {
        toWrite = len;
    }
    memcpy(stream, &customMusic.data[customMusic.cur], toWrite);
    customMusic.cur += toWrite;
    return toWrite;
}

static void customMusicCallback(void *dummy, Uint8 *stream, int len)
{
    int canContinue = 1;
    do
    {
        int copied = copyAudioFromBuffer(stream, len);
        if (copied)
        {
            len -= copied;
            stream += copied;
        }
        if (len == 0)
        {
            canContinue = 0;
        }
        else
        {
            canContinue = nextAudioFrame();
        }
    }
    while (canContinue);
    if (len)
    {
        // end of stream, write silence
        memset(stream, 0, len);
    }
}

void SoundDevice_useCustomMusicPlayer(int bitdepth, int channels, int rate, const unsigned char *(*callback)(int *outLen))
{
    SDL_BuildAudioCVT(&customMusic.cvt, bitdepth, channels, rate, AUDIO_FORMAT, AUDIO_CHANNELS, AUDIO_RATE);
    customMusic.callback = callback;
    Mix_HookMusic(customMusicCallback, 0);
}

void SoundDevice_useDefaultMusicPlayer()
{
    if (customMusic.data)
    {
        free(customMusic.data);
        customMusic.data = 0;
        customMusic.len = 0;
        customMusic.cur = 0;
    }
    Mix_HookMusic(0, 0);
}

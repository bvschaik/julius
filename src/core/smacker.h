#ifndef CORE_SMACKER_H
#define CORE_SMACKER_H

#include "graphics/color.h"

#include <stdio.h>
#include <stdint.h>

/** Smacker object struct pointer */
typedef struct smacker_t *smacker;

enum {
    SMACKER_Y_SCALE_NONE = 0,
    SMACKER_Y_SCALE_INTERLACE = 1,
    SMACKER_Y_SCALE_DOUBLE = 2
};

typedef enum {
    SMACKER_FRAME_ERROR = 0,
    SMACKER_FRAME_OK = 1,
    SMACKER_FRAME_DONE = 2
} smacker_frame_status;

/**
 * Open SMK file from file pointer.
 * Smacker takes ownership of the file pointer and will close it when done.
 * @param file File
 * @return Smacker object if opening succeeded, otherwise NULL
 */
smacker smacker_open(FILE *file);

/**
 * Close SMK file and clean up memory
 * @param s Smacker object
 */
void smacker_close(smacker s);

/**
 * Get frame information
 * @param s Smacker object
 * @param frame_count Total number of frames
 * @param usf Microseconds per frame
 */
void smacker_get_frames_info(const smacker s, int *frame_count, int *usf);

/**
 * Get video information
 * @param s Smacker object
 * @param width Width of the video
 * @param height Unscaled height of the video
 * @param y_scale_mode Scale mode
 */
void smacker_get_video_info(const smacker s, int *width, int *height, int *y_scale_mode);

/**
 * Get audio information for specified track
 * @param s Smacker object
 * @param track Audio track (0-6)
 * @param enabled Whether this track is enabled
 * @param channels Number of channels: 1 = mono, 2 = stereo
 * @param bitdepth Bit depth: 8 or 16
 * @param audio_rate Rate in Hz, typically 22050 or 44100
 */
void smacker_get_audio_info(const smacker s, int track, int *enabled, int *channels, int *bitdepth, int *audio_rate);

/**
 * Go to first frame and unpack
 * @param s Smacker object
 * @return Status, one of SMACKER_FRAME_* constants
 */
smacker_frame_status smacker_first_frame(smacker s);

/**
 * Go to next frame and unpack
 * @param s Smacker object
 * @return Status, one of SMACKER_FRAME_* constants
 */
smacker_frame_status smacker_next_frame(smacker s);

/**
 * Get palette as array of 256 colors for the current frame
 * @param s Smacker object
 * @return Pointer to 256 colors
 */
const color_t *smacker_get_frame_palette(const smacker s);

/**
 * Get video data for the current frame.
 * Note that you need to scale the video vertically according to y_scale_mode.
 * @param s Smacker object
 * @return width * height bytes that refer to a color in the palette
 */
const uint8_t *smacker_get_frame_video(const smacker s);

/**
 * Get length of audio data for the current frame and track
 * @param s Smacker object
 * @param track Audio track (0-6)
 * @return Number of audio bytes returned by smacker_get_audio()
 */
int smacker_get_frame_audio_size(const smacker s, int track);

/**
 * Get audio data for the current frame
 * @param s Smacker object
 * @param track Audio track (0-6)
 * @return Audio data
 */
const uint8_t *smacker_get_frame_audio(const smacker s, int track);

#endif // CORE_SMACKER_H

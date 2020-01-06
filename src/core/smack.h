#ifndef CORE_SMACK_H
#define CORE_SMACK_H

#include "graphics/color.h"

#include <stdio.h>
#include <stdint.h>

/** SMK object struct pointer */
typedef struct smk_t* smk;

typedef enum {
    SMK_MODE_DISK = 1,
    SMK_MODE_MEMORY = 2
} smk_file_mode;

enum {
    SMK_Y_SCALE_NONE = 0,
    SMK_Y_SCALE_INTERLACE = 1,
    SMK_Y_SCALE_DOUBLE = 2
};

typedef enum {
    SMK_FRAME_ERROR = 0,
    SMK_FRAME_OK = 1,
    SMK_FRAME_DONE = 2
} smk_frame_status;

/**
 * Open SMK file from file pointer.
 * SMK takes ownership of the file pointer and will close it when done.
 * @param file File
 * @param mode Open mode, one of SMK_MODE_* constants
 * @return SMK object if opening succeeded, otherwise NULL
 */
smk smk_open(FILE* file, smk_file_mode mode);

/**
 * Close SMK file and clean up memory
 * @param s SMK object
 */
void smk_close(smk s);

/**
 * Get frame information
 * @param s SMK object
 * @param frame_count Total number of frames
 * @param usf Microseconds per frame
 */
void smk_info_frames(const smk s, int *frame_count, int *usf);

/**
 * Get video information
 * @param s SMK object
 * @param width Width of the video
 * @param height Unscaled height of the video
 * @param y_scale_mode Scale mode
 */
void smk_info_video(const smk s, int *width, int *height, int *y_scale_mode);

/**
 * Get audio information for specified track
 * @param s SMK object
 * @param track Audio track (0-6)
 * @param enabled Whether this track is enabled
 * @param channels Number of channels: 1 = mono, 2 = stereo
 * @param bitdepth Bit depth: 8 or 16
 * @param audio_rate Rate in Hz, typically 22050 or 44100
 */
void smk_info_audio(const smk s, int track, int *enabled, int *channels, int *bitdepth, int *audio_rate);

/**
 * Go to first frame and unpack
 * @param s SMK object
 * @return Status, one of SMK_FRAME_* constants
 */
smk_frame_status smk_first(smk s);

/**
 * Go to next frame and unpack
 * @param s SMK object
 * @return Status, one of SMK_FRAME_* constants
 */
smk_frame_status smk_next(smk s);

/**
 * Get palette as array of 256 colors for the current frame
 * @param s SMK object
 * @return Pointer to 256 colors
 */
const color_t *smk_get_palette(const smk s);

/**
 * Get video data for the current frame.
 * Note that you need to scale the video vertically according to y_scale_mode.
 * @param s SMK object
 * @return width * height bytes that refer to a color in the palette
 */
const uint8_t *smk_get_video(const smk s);

/**
 * Get length of audio data for the current frame and track
 * @param s SMK object
 * @param track Audio track (0-6)
 * @return Number of audio bytes returned by smk_get_audio()
 */
int smk_get_audio_size(const smk s, int track);

/**
 * @param s SMK object
 * @param track Audio track (0-6)
 * @return Audio data
 */
const uint8_t *smk_get_audio(const smk s, int track);

#endif // CORE_SMACK_H

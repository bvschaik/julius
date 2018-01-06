#ifndef GRAPHICS_VIDEO_H
#define GRAPHICS_VIDEO_H

/**
 * Start playing the video
 * @param filename Video file
 * @return True if the video could be loaded
 */
int video_start(const char* filename);

/**
 * Initializes the video subsystem, necessary just before a video is drawn for the first time
 */
void video_init();

/**
 * Checks whether the video has finished playing
 */
int video_is_finished();

/**
 * Stop playing the currently playing video
 */
void video_stop();

/**
 * Shut down the video subsystem
 */
void video_shutdown();

/**
 * Draws a frame of the current video at the specified position
 * @param x_offset X offset
 * @param y_offset Y offset
 */
void video_draw(int x_offset, int y_offset);

#endif // GRAPHICS_VIDEO_H

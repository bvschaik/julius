#ifndef GRAPHICS_VIDEO_H
#define GRAPHICS_VIDEO_H

/**
 * Start playing the video
 * @param filename Video file
 * @return True if the video could be loaded
 */
int video_start(const char *filename);

/**
 * Get size of the video
 * Call only after succesful video_start()
 * @param width Out: video width
 * @param height Out: video height
 */
void video_size(int *width, int *height);

/**
 * Initializes the video subsystem, necessary just before a video is drawn for the first time
 */
void video_init(void);

/**
 * Checks whether the video has finished playing
 */
int video_is_finished(void);

/**
 * Stop playing the currently playing video
 */
void video_stop(void);

/**
 * Shut down the video subsystem
 */
void video_shutdown(void);

/**
 * Draws a frame of the current video at the specified position
 * @param x_offset X offset
 * @param y_offset Y offset
 */
void video_draw(int x_offset, int y_offset);

#endif // GRAPHICS_VIDEO_H

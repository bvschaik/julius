#ifndef PLATFORM_DPI_H
#define PLATFORM_DPI_H

#include "SDL.h"

void platform_dpi_init(SDL_Window *window);
void platform_dpi_check_change(void);
void platform_dpi_shutdown(void);
int platform_dpi_get_title_height(void);
double platform_dpi_get_scale(void);
Uint32 platform_dpi_enabled(void);

#endif // PLATFORM_DPI_H

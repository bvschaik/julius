#include "platform.h"

#include "SDL.h"

int platform_sdl_version_at_least(int major, int minor, int patch)
{
    SDL_version v;
    SDL_GetVersion(&v);
    return SDL_VERSIONNUM(v.major, v.minor, v.patch) >= SDL_VERSIONNUM(major, minor, patch);
}

#include "platform.h"

#include "platform/emscripten/emscripten.h"

#include "SDL.h"

#include <stdlib.h>

static SDL_version version;

int platform_sdl_version_at_least(int major, int minor, int patch)
{
    if (version.major == 0) {
        SDL_GetVersion(&version);
    }
    return SDL_VERSIONNUM(version.major, version.minor, version.patch) >= SDL_VERSIONNUM(major, minor, patch);
}

void exit_with_status(int status)
{
#ifdef __EMSCRIPTEN__
    EM_ASM(Module.quitGame($0), status);
#endif
    exit(status);
}

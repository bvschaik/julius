#ifndef PLATFORM_EMSCRIPTEN_H
#define PLATFORM_EMSCRIPTEN_H

#ifdef __EMSCRIPTEN__

#include <emscripten.h>

#define PLATFORM_NO_USER_DIRECTORIES

#endif // __EMSCRIPTEN__
#endif // PLATFORM_EMSCRIPTEN_H

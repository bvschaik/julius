#include "dpi.h"
#include "game/system.h"

#define DPI_SDL_MINIMUM_VERSION SDL_VERSIONNUM(2, 0, 5)

/* The Mac OS executable works well without DPI support, and not as well with DPI awareness enabled. */
#if !defined(__APPLE__) && (SDL_COMPILEDVERSION >= DPI_SDL_MINIMUM_VERSION)
#define DPI_SUPPORTED
#endif

static const int DEFAULT_TITLE_HEIGHT = 35;

static struct
{
    const float DEFAULT;
    double scale;
    Uint32 flag;
    int t;
    SDL_Window *window;
} DPI = { 96.0f, 1.0f, 0, 0, NULL };

#if defined(_USING_X11)
#include <dlfcn.h>
#include <X11/Xresource.h>
#include "SDL_syswm.h"

static struct
{
    void *handler;
    Display *(*XOpenDisplay)(char*);
    void(*XCloseDisplay)(Display*);
    void(*XrmInitialize)(void);
    char *(*XResourceManagerString)(Display*);
    XrmDatabase(*XrmGetStringDatabase)(const char*);
    int(*XrmGetResource)(XrmDatabase, const char*, const char*, char**, XrmValue*);
    void(*XrmDestroyDatabase)(XrmDatabase);
} XLib;

static void load_xlib_symbols(void)
{
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    if (!SDL_GetWindowWMInfo(DPI.window, &info) || info.subsystem != SDL_SYSWM_X11) {
        return;
    }

    XLib.handler = dlopen(NULL, RTLD_NOW | RTLD_GLOBAL);
    if (!XLib.handler) {
        SDL_Log("Could not load the X11 library to get the proper DPI value. The reported DPI may be wrong.\n");
        return;
    }

    XLib.XOpenDisplay = dlsym(XLib.handler, "XOpenDisplay");
    XLib.XCloseDisplay = dlsym(XLib.handler, "XCloseDisplay");
    XLib.XrmInitialize = dlsym(XLib.handler, "XrmInitialize");
    XLib.XResourceManagerString = dlsym(XLib.handler, "XResourceManagerString");
    XLib.XrmGetStringDatabase = dlsym(XLib.handler, "XrmGetStringDatabase");
    XLib.XrmGetResource = dlsym(XLib.handler, "XrmGetResource");
    XLib.XrmDestroyDatabase = dlsym(XLib.handler, "XrmDestroyDatabase");

    if (!XLib.XOpenDisplay || !XLib.XCloseDisplay || !XLib.XrmInitialize || !XLib.XResourceManagerString
        || !XLib.XrmGetStringDatabase || !XLib.XrmGetResource || !XLib.XrmDestroyDatabase) {
        dlclose(XLib.handler);
        XLib.handler = NULL;
        SDL_Log("Could not load the X11 library to get the proper DPI value. The reported DPI may be wrong.\n");
    }
}

static float get_dpi_from_x11(void)
{
    if (!XLib.handler) {
        return DPI.DEFAULT;
    }
    /* We need to create a new X server connection, otherwise the resource manager won't get the updated DPI value */
    Display *display = XLib.XOpenDisplay(NULL);
    if (!display) {
        return DPI.DEFAULT;
    }
    XLib.XrmInitialize();
    char *resource_string = XLib.XResourceManagerString(display);
    XrmDatabase db = XLib.XrmGetStringDatabase(resource_string);
    XrmValue value;
    char *type = NULL;
    float dpi = DPI.DEFAULT;

    if (resource_string) {
        if (XLib.XrmGetResource(db, "Xft.dpi", "String", &type, &value)) {
            if (type && strcmp(type, "String") == 0 && value.addr) {
                dpi = atof(value.addr);
            }
        }
     }

    XLib.XrmDestroyDatabase(db);
    XLib.XCloseDisplay(display);

    return (dpi != 0.0f) ? dpi : DPI.DEFAULT;
}
#endif // defined (_USING_X11)

static void set_dpi(void)
{
#ifndef DPI_SUPPORTED
    return;
#else
    int display = SDL_GetWindowDisplayIndex(DPI.window);
    float current_dpi = 0.0f;

    if (SDL_GetDisplayDPI(display, NULL, &current_dpi, NULL) != 0 || current_dpi == 0.0f) {
#if defined (_USING_X11)
        /* It might be possible to request DPI information directly from the X11 server */
        current_dpi = get_dpi_from_x11();
#else
        SDL_Log("Unable to get the system DPI! Using default DPI.\n");
        current_dpi = DPI.DEFAULT;
#endif
    }

    if (current_dpi == DPI.DEFAULT) {
        DPI.scale = 1.0f;
    } else {
        DPI.scale = current_dpi / DPI.DEFAULT;
    }
#endif
}

void platform_dpi_init(SDL_Window *window)
{
#ifndef DPI_SUPPORTED
    SDL_Log("Your SDL version does not support DPI scaling. Using default DPI.");
    return;
#else
    DPI.flag = SDL_WINDOW_ALLOW_HIGHDPI;
    DPI.window = window;
#if defined (_USING_X11)
    /* Try to load the X11 lib dynamically as it may actually not be used (i.e. if using Wayland) */
    load_xlib_symbols();
#endif
    set_dpi();
    if (DPI.scale != 1.0f) {
        SDL_Log("Custom DPI detected. Scaling window by %f\n", DPI.scale);
    }
#endif
}

void platform_dpi_check_change(void)
{
    if (!DPI.flag) {
        return;
    }

    int title_height = platform_dpi_get_title_height();

    /* DPI changes cause the the borders and title to get smaller or bigger, so when the border size changes we check for DPI changes */
    /* This won't work for fullscreen, but that seems like an edge case rare enough to let pass */
    if (title_height && DPI.t && title_height != DPI.t) {
        double old_scale = DPI.scale;
        set_dpi();
        if (old_scale != DPI.scale) {
            system_rescale();
        }
    }
    DPI.t = title_height;
}

void platform_dpi_shutdown(void)
{
#if defined (_USING_X11)
    if (XLib.handler) {
        dlclose(XLib.handler);
    }
#endif
}

double platform_dpi_get_scale(void)
{
    return DPI.scale;
}

Uint32 platform_dpi_enabled(void)
{
    return DPI.flag;
}

int platform_dpi_get_title_height(void)
{
#ifndef DPI_SUPPORTED
    return DEFAULT_TITLE_HEIGHT;
#else
    if (!DPI.window) {
        return DEFAULT_TITLE_HEIGHT;
    }
    int top;
    SDL_GetWindowBordersSize(DPI.window, &top, NULL, NULL, NULL);
    return top;
#endif
}

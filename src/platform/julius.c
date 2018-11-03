#include "SDL.h"

#include "core/lang.h"
#include "core/time.h"
#include "game/settings.h"
#include "game/game.h"
#include "game/system.h"
#include "graphics/graphics.h" // debug for fps counter
#include "graphics/screen.h"
#include "graphics/text.h" // debug for fps counter
#include "graphics/window.h"
#include "input/mouse.h"
#include "platform/keyboard_input.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(__GNUC__) && !defined(__MINGW32__)
#include <execinfo.h>
#endif

#ifdef _MSC_VER
#include <direct.h>
#define chdir _chdir
#define getcwd _getcwd
#else
#include <unistd.h>
#endif

static struct {
    int x;
    int y;
} window_pos;

static struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
} SDL;

enum {
    USER_EVENT_REFRESH = 0,
    USER_EVENT_QUIT = 1,
    USER_EVENT_RESIZE = 2,
    USER_EVENT_FULLSCREEN = 3,
    USER_EVENT_WINDOWED = 4,
};

static void handler(int sig) {
#if defined(__GNUC__) && !defined(__MINGW32__)
    void *array[100];
    size_t size;
    
    // get void*'s for all entries on the stack
    size = backtrace(array, 100);
    
    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
#else
    fprintf(stderr, "Oops, crashed with signal %d :(\n", sig);
#endif
    exit(1);
}

void system_exit()
{
    SDL_Event event;
    event.user.type = SDL_USEREVENT;
    event.user.code = USER_EVENT_QUIT;
    SDL_PushEvent(&event);
}

void system_resize(int width, int height)
{
    static int s_width;
    static int s_height;
    s_width = width;
    s_height = height;
    SDL_Event event;
    event.user.type = SDL_USEREVENT;
    event.user.code = USER_EVENT_RESIZE;
    event.user.data1 = &s_width;
    event.user.data2 = &s_height;
    SDL_PushEvent(&event);
}

void system_toggle_fullscreen()
{
    SDL_Event event;
    event.user.type = SDL_USEREVENT;
    if (setting_fullscreen()) {
        event.user.code = USER_EVENT_WINDOWED;
    } else {
        event.user.code = USER_EVENT_FULLSCREEN;
    }
    SDL_PushEvent(&event);
}

static Uint32 last;

static void refresh()
{
    static Uint32 lastFpsTime = 0;
    static int lastFps = 0;
    static int numFrames = 0;
    
    Uint32 now = SDL_GetTicks();
    time_set_millis(now);
    game_run();
    
    // debug
    Uint32 then = SDL_GetTicks();
    
    game_draw();
    numFrames++;
    Uint32 then2 = SDL_GetTicks();
    if (then2 - lastFpsTime > 1000) {
        lastFps = numFrames;
        lastFpsTime = then2;
        numFrames = 0;
    }
    int s_width = screen_width();
    graphics_fill_rect(s_width - 120, 0, s_width, 20, COLOR_WHITE);
    text_draw_number_colored(lastFps, 'f', "", s_width - 120, 5, FONT_NORMAL_PLAIN, COLOR_RED);
    text_draw_number_colored(then - now, 'g', "", s_width - 70, 5, FONT_NORMAL_PLAIN, COLOR_RED);
    text_draw_number_colored(then2 - then, 'd', "", s_width - 40, 5, FONT_NORMAL_PLAIN, COLOR_RED);
    
    SDL_UpdateTexture(SDL.texture, NULL, graphics_canvas(), s_width * 4);
    SDL_RenderCopy(SDL.renderer, SDL.texture, NULL, NULL);
    SDL_RenderPresent(SDL.renderer);
    last = now;
}

static void createWindowAndRenderer(int width, int height, int fullscreen)
{
    printf("Fullscreen? %d\n", fullscreen);
    if (SDL.window) {
        SDL_DestroyWindow(SDL.window);
        SDL.window = 0;
    }
    if (SDL.renderer) {
        SDL_DestroyRenderer(SDL.renderer);
        SDL.renderer = 0;
    }
    
    const char *title = (const char*)lang_get_string(9, 0);
    Uint32 flags = SDL_WINDOW_RESIZABLE;
    if (fullscreen) {
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
    SDL.window = SDL_CreateWindow(title,
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width, height, flags);
    
    SDL.renderer = SDL_CreateRenderer(SDL.window, -1, SDL_RENDERER_PRESENTVSYNC);
}

static void createSurface(int width, int height, int fullscreen)
{
    if (SDL.texture) {
        SDL_DestroyTexture(SDL.texture);
        SDL.texture = 0;
    }
    
    setting_set_display(fullscreen, width, height);
    SDL.texture = SDL_CreateTexture(SDL.renderer,
        SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
        width, height);
    if (SDL.texture) {
        printf("Texture created (%d x %d)\n", width, height);// with scanline %d\n", surface->pitch);
        /*printf("  flags: %d %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s\n",
        surface->flags,
        surface->flags & SDL_SWSURFACE ? "SDL_SWSURFACE" : "",
        surface->flags & SDL_SWSURFACE ? "SDL_HWSURFACE" : "",
        surface->flags & SDL_ASYNCBLIT ? "SDL_ASYNCBLIT" : "",
        surface->flags & SDL_ANYFORMAT ? "SDL_ANYFORMAT" : "",
        surface->flags & SDL_HWPALETTE ? "SDL_HWPALETTE" : "",
        surface->flags & SDL_DOUBLEBUF ? "SDL_DOUBLEBUF" : "",
        surface->flags & SDL_FULLSCREEN ? "SDL_FULLSCREEN" : "",
        surface->flags & SDL_OPENGL ? "SDL_OPENGL" : "",
        surface->flags & SDL_OPENGLBLIT ? "SDL_OPENGLBLIT" : "",
        surface->flags & SDL_RESIZABLE ? "SDL_RESIZABLE" : "",
        surface->flags & SDL_HWACCEL ? "SDL_HWACCEL" : "",
        surface->flags & SDL_SRCCOLORKEY ? "SDL_SRCCOLORKEY" : "",
        surface->flags & SDL_RLEACCEL ? "SDL_RLEACCEL" : "",
        surface->flags & SDL_SRCALPHA ? "SDL_SRCALPHA" : "",
        surface->flags & SDL_PREALLOC ? "SDL_PREALLOC" : "");
        printf("  bpp: %d\n", surface->format->BitsPerPixel);
        printf("  Rmask %x, Gmask %x, Bmask %x, Amask %x\n",
            surface->format->Rmask,
            surface->format->Gmask,
            surface->format->Bmask,
            surface->format->Amask);
        printf("  Rshift %d, Gshift %d, Bshift %d, Ashift %d\n",
            surface->format->Rshift,
            surface->format->Gshift,
            surface->format->Bshift,
            surface->format->Ashift);
        */
        screen_set_resolution(width, height);
    } else {
        printf("Unable to create texture: %s\n", SDL_GetError());
    }
}

static void set_fullscreen()
{
    SDL_GetWindowPosition(SDL.window, &window_pos.x, &window_pos.y);
    int orig_w, orig_h;
    SDL_GetWindowSize(SDL.window, &orig_w, &orig_h);
    SDL_DisplayMode mode;
    SDL_GetDesktopDisplayMode(SDL_GetWindowDisplayIndex(SDL.window), &mode);
    printf("User to fullscreen %d x %d\n", mode.w, mode.h);
    if (0 != SDL_SetWindowFullscreen(SDL.window, SDL_WINDOW_FULLSCREEN_DESKTOP)) {
        SDL_Log("Unable to enter fullscreen: %s\n", SDL_GetError());
        return;
    }
    SDL_SetWindowDisplayMode(SDL.window, &mode);
    setting_set_display(1, mode.w, mode.h);
}


static void set_windowed()
{
    int width, height;
    setting_window(&width, &height);
    printf("User to windowed %d x %d\n", width, height);
    SDL_SetWindowFullscreen(SDL.window, 0);
    SDL_SetWindowSize(SDL.window, width, height);
    SDL_SetWindowPosition(SDL.window, window_pos.x, window_pos.y);
    setting_set_display(0, width, height);
}

static void set_window_size(int width, int height)
{
    if (setting_fullscreen()) {
        SDL_SetWindowFullscreen(SDL.window, 0);
    }
    SDL_SetWindowSize(SDL.window, width, height);
    SDL_SetWindowPosition(SDL.window, window_pos.x, window_pos.y);
    printf("User resize to %d x %d\n", width, height);
    setting_set_display(0, width, height);
}

static void handle_mouse_button(SDL_MouseButtonEvent *event, int is_down)
{
    mouse_set_position(event->x, event->y);
    if (event->button == SDL_BUTTON_LEFT) {
        mouse_set_left_down(is_down);
    } else if (event->button == SDL_BUTTON_RIGHT) {
        mouse_set_right_down(is_down);
    }
}

static void mainLoop()
{
    SDL_Event event;
    SDL_Event refreshEvent;
    refreshEvent.user.type = SDL_USEREVENT;
    refreshEvent.user.code = USER_EVENT_REFRESH;
    mouse_set_inside_window(1);
    
    refresh();
    int active = 1;
    while (1) {
        /* Process event queue */
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_WINDOWEVENT:
                    switch (event.window.event) {
                        case SDL_WINDOWEVENT_ENTER:
                            mouse_set_inside_window(1);
                            break;
                        case SDL_WINDOWEVENT_LEAVE:
                            mouse_set_inside_window(0);
                            break;
                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                            printf("Window resized to %d x %d\n", event.window.data1, event.window.data2);
                            createSurface(event.window.data1, event.window.data2, setting_fullscreen());
                            window_invalidate();
                            break;
                        case SDL_WINDOWEVENT_RESIZED:
                            printf("System resize to %d x %d\n", event.window.data1, event.window.data2);
                            break;

                        case SDL_WINDOWEVENT_SHOWN:
                            SDL_Log("Window %d shown", event.window.windowID);
                            active = 1;
                            break;
                        case SDL_WINDOWEVENT_HIDDEN:
                            SDL_Log("Window %d hidden", event.window.windowID);
                            active = 0;
                            break;
                    }
                    break;
                
                case SDL_KEYDOWN:
                    platform_handle_key_down(&event.key);
                    break;
                
                case SDL_KEYUP:
                    platform_handle_key_up(&event.key);
                    break;
                
                case SDL_TEXTINPUT:
                    platform_handle_text(&event.text);
                    break;
                
                case SDL_MOUSEMOTION:
                    mouse_set_position(event.motion.x, event.motion.y);
                    break;
                
                case SDL_MOUSEBUTTONDOWN:
                    handle_mouse_button(&event.button, 1);
                    break;
                
                case SDL_MOUSEBUTTONUP:
                    handle_mouse_button(&event.button, 0);
                    break;
                
                case SDL_MOUSEWHEEL:
                    mouse_set_scroll(event.wheel.y > 0 ? SCROLL_UP : event.wheel.y < 0 ? SCROLL_DOWN : SCROLL_NONE);
                    break;

                case SDL_QUIT:
                    return;
                
                case SDL_USEREVENT:
                    if (event.user.code == USER_EVENT_QUIT) {
                        return;
                    } else if (event.user.code == USER_EVENT_RESIZE) {
                        set_window_size(*(int*)event.user.data1, *(int*)event.user.data2);
                    } else if (event.user.code == USER_EVENT_FULLSCREEN) {
                        set_fullscreen();
                    } else if (event.user.code == USER_EVENT_WINDOWED) {
                        set_windowed();
                    }
                    break;
                
                default:
                    //printf("Unknown event: %d\n", event.type);
                    break;
            }
        }
        if (active) {
            // Push user refresh event
            SDL_PushEvent(&refreshEvent);
            refresh();
        } else {
            SDL_WaitEvent(NULL);
        }
    }
}

static void initSdl()
{
    printf("Initializing SDL.\n");
    
    // Initialize defaults, Video and Audio
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) != 0) {
        printf("Could not initialize SDL: %s.\n", SDL_GetError());
        exit(-1);
    }
    
    printf("SDL initialized.\n");
}

int main(int argc, char **argv)
{
    signal(SIGSEGV, handler);
    
    initSdl();
    
    // C3 setup
    if (!game_pre_init()) {
        if (argc > 1 && argv[1]) {
            if (chdir(argv[1]) != 0) {
                printf("%s: directory not found!\n", argv[1]);
                return 1;
            }
        } else {
            if (chdir("../data") != 0) {
                printf("../data: directory not found!\n");
                return 1;
            }
        }
        if (!game_pre_init()) {
            return 1;
        }
    }

    int width, height;
    int fullscreen = setting_fullscreen();
    if (fullscreen) {
        SDL_DisplayMode mode;
        SDL_GetDesktopDisplayMode(0, &mode);
        width = mode.w;
        height = mode.h;
    } else {
        setting_window(&width, &height);
    }
    createWindowAndRenderer(width, height, fullscreen);
    createSurface(width, height, fullscreen);

    if (!game_init()) {
        return 2;
    }

    mainLoop();
    printf("Quiting SDL.\n");
    
    game_exit();

    // Shutdown all subsystems
    SDL_Quit();
    
    printf("Quiting....\n");
    
    exit(0);
}

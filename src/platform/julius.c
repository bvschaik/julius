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
#include "platform/screen.h"

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
    // TODO two resizes at the same time = trouble
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
    
    platform_screen_render(graphics_canvas());
    last = now;
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

static void main_loop()
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
                            platform_screen_resize(event.window.data1, event.window.data2, setting_fullscreen());
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
                        platform_screen_set_window_size(*(int*)event.user.data1, *(int*)event.user.data2);
                    } else if (event.user.code == USER_EVENT_FULLSCREEN) {
                        platform_screen_set_fullscreen();
                    } else if (event.user.code == USER_EVENT_WINDOWED) {
                        platform_screen_set_windowed();
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

static int init_sdl()
{
    SDL_Log("Initializing SDL");
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not initialize SDL: %s", SDL_GetError());
        return 0;
    }
    SDL_Log("SDL initialized");
    return 1;
}

static int pre_init(const char *custom_data_dir)
{
    if (custom_data_dir) {
        if (chdir(custom_data_dir) != 0) {
            SDL_Log("%s: directory not found", custom_data_dir);
            return 0;
        }
        return game_pre_init();
    }
    SDL_Log("Loading game from working directory");
    if (game_pre_init()) {
        return 1;
    }
    if (chdir("../data") != 0) {
        SDL_Log("../data: directory not found");
        return 1;
    }
    SDL_Log("Loading game from data directory");
    return game_pre_init();
}

static void setup(const char *custom_data_dir)
{
    signal(SIGSEGV, handler);

    if (!init_sdl()) {
        exit(-1);
    }

    if (!pre_init(custom_data_dir)) {
        exit(1);
    }

    const char *title = (const char*)lang_get_string(9, 0);
    platform_screen_create(title);

    if (!game_init()) {
        exit(2);
    }
}

static void teardown()
{
    SDL_Log("Exiting game");
    game_exit();
    SDL_Quit();
}

int main(int argc, char **argv)
{
    const char *custom_data_dir = (argc > 1 && argv[1]) ? argv[1] : NULL;
    setup(custom_data_dir);

    main_loop();

    teardown();
    return 0;
}

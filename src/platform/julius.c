#include "SDL.h"

#include "core/lang.h"
#include "core/time.h"
#include "game/game.h"
#include "graphics/window.h"
#include "input/mouse.h"
#include "platform/keyboard_input.h"
#include "platform/screen.h"
#include "platform/version.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_WIN32)
#include <string.h>
#endif

#if defined(__GNUC__) && !defined(__MINGW32__) && !defined(__OpenBSD__)
#include <execinfo.h>
#endif

#ifdef _MSC_VER
#include <direct.h>
#define chdir _chdir
#define getcwd _getcwd
#else
#include <unistd.h>
#endif

#define INTPTR(d) (*(int*)(d))

enum {
    USER_EVENT_QUIT,
    USER_EVENT_RESIZE,
    USER_EVENT_FULLSCREEN,
    USER_EVENT_WINDOWED,
    USER_EVENT_CENTER_WINDOW,
};

static void handler(int sig) {
#if defined(__GNUC__) && !defined(__MINGW32__) && !defined(__OpenBSD__)
    void *array[100];
    size_t size;
    
    // get void*'s for all entries on the stack
    size = backtrace(array, 100);
    
    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
#else
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Oops, crashed with signal %d :(", sig);
#endif
    exit(1);
}

#if defined(_WIN32)
/* Log to separate file on windows, since we don't have a console there */
static FILE *log_file = 0;

static void write_log(void *userdata, int category, SDL_LogPriority priority, const char *message)
{
    if (log_file) {
        if (priority == SDL_LOG_PRIORITY_ERROR) {
            fwrite("ERROR: ", sizeof(char), 7, log_file);
        } else {
            fwrite("INFO: ", sizeof(char), 6, log_file);
        }
        fwrite(message, sizeof(char), strlen(message), log_file);
        fwrite("\r\n", sizeof(char), 2, log_file);
        fflush(log_file);
    }
}

static void setup_logging(void)
{
    log_file = fopen("julius-log.txt", "w");
    SDL_LogSetOutputFunction(write_log, NULL);
}

static void teardown_logging(void)
{
    if (log_file) {
        fclose(log_file);
    }
}

#else
static void setup_logging(void) {}
static void teardown_logging(void) {}
#endif

static void post_event(int code)
{
    SDL_Event event;
    event.user.type = SDL_USEREVENT;
    event.user.code = code;
    SDL_PushEvent(&event);
}

void system_exit(void)
{
    post_event(USER_EVENT_QUIT);
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

void system_center(void)
{
    post_event(USER_EVENT_CENTER_WINDOW);
}

void system_set_fullscreen(int fullscreen)
{
    post_event(fullscreen ? USER_EVENT_FULLSCREEN : USER_EVENT_WINDOWED);
}

static void run_and_draw(void)
{
    time_set_millis(SDL_GetTicks());

    game_run();
    game_draw();

    platform_screen_render();
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

static void handle_window_event(SDL_WindowEvent *event, int *window_active)
{
    switch (event->event) {
        case SDL_WINDOWEVENT_ENTER:
            mouse_set_inside_window(1);
            break;
        case SDL_WINDOWEVENT_LEAVE:
            mouse_set_inside_window(0);
            break;
        case SDL_WINDOWEVENT_SIZE_CHANGED:
            SDL_Log("Window resized to %d x %d\n", event->data1, event->data2);
            platform_screen_resize(event->data1, event->data2);
            break;
        case SDL_WINDOWEVENT_RESIZED:
            SDL_Log("System resize to %d x %d\n", event->data1, event->data2);
            break;

        case SDL_WINDOWEVENT_SHOWN:
            SDL_Log("Window %d shown", event->windowID);
            *window_active = 1;
            break;
        case SDL_WINDOWEVENT_HIDDEN:
            SDL_Log("Window %d hidden", event->windowID);
            *window_active = 0;
            break;
    }
}

static void handle_event(SDL_Event *event, int *active, int *quit)
{
    switch (event->type) {
        case SDL_WINDOWEVENT:
            handle_window_event(&event->window, active);
            break;

        case SDL_KEYDOWN:
            platform_handle_key_down(&event->key);
            break;
        case SDL_KEYUP:
            platform_handle_key_up(&event->key);
            break;
        case SDL_TEXTINPUT:
            platform_handle_text(&event->text);
            break;

        case SDL_MOUSEMOTION:
            mouse_set_position(event->motion.x, event->motion.y);
            break;
        case SDL_MOUSEBUTTONDOWN:
            handle_mouse_button(&event->button, 1);
            break;
        case SDL_MOUSEBUTTONUP:
            handle_mouse_button(&event->button, 0);
            break;
        case SDL_MOUSEWHEEL:
            mouse_set_scroll(event->wheel.y > 0 ? SCROLL_UP : event->wheel.y < 0 ? SCROLL_DOWN : SCROLL_NONE);
            break;

        case SDL_QUIT:
            *quit = 1;
            break;

        case SDL_USEREVENT:
            if (event->user.code == USER_EVENT_QUIT) {
                *quit = 1;
            } else if (event->user.code == USER_EVENT_RESIZE) {
                platform_screen_set_window_size(INTPTR(event->user.data1), INTPTR(event->user.data2));
            } else if (event->user.code == USER_EVENT_FULLSCREEN) {
                platform_screen_set_fullscreen();
            } else if (event->user.code == USER_EVENT_WINDOWED) {
                platform_screen_set_windowed();
            } else if (event->user.code == USER_EVENT_CENTER_WINDOW) {
                platform_screen_center_window();
            }
            break;

        default:
            break;
    }
}

static void main_loop(void)
{
    SDL_Event event;
    mouse_set_inside_window(1);
    
    run_and_draw();
    int active = 1;
    int quit = 0;
    while (!quit) {
        /* Process event queue */
        while (SDL_PollEvent(&event)) {
            handle_event(&event, &active, &quit);
        }
        if (!quit) {
            if (active) {
                run_and_draw();
            } else {
                SDL_WaitEvent(NULL);
            }
        }
    }
}

static int init_sdl(void)
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
        SDL_Log("Loading game from %s", custom_data_dir);
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
    if (chdir("../data") == 0) {
        SDL_Log("Loading game from data directory");
        return game_pre_init();
    }
    SDL_Log("Julius requires the original files from Caesar 3 to run.");
    SDL_Log("Move the Julius executable to the directory containing an existing Caesar 3 installation, or run:");
    SDL_Log("julius path-to-c3-directory");
    return 0;
}

static void setup(const char *custom_data_dir)
{
    signal(SIGSEGV, handler);
    setup_logging();

    if (!init_sdl()) {
        SDL_Log("Exiting: SDL init failed");
        exit(-1);
    }

    if (!pre_init(custom_data_dir)) {
        SDL_Log("Exiting: game pre-init failed");
        exit(1);
    }

    if (!platform_screen_create((const char*)lang_get_string(9, 0))) {
        SDL_Log("Exiting: SDL create window failed");
        exit(-2);
    }

    if (!game_init()) {
        SDL_Log("Exiting: game init failed");
        exit(2);
    }
}

static void teardown(void)
{
    SDL_Log("Exiting game");
    game_exit();
    platform_screen_destroy();
    SDL_Quit();
    teardown_logging();
}

int main(int argc, char **argv)
{
    SDL_Log("Built with git commit: %s\n", GIT_COMMIT_HASH);

    const char *custom_data_dir = (argc > 1 && argv[1]) ? argv[1] : NULL;
    setup(custom_data_dir);

    main_loop();

    teardown();
    return 0;
}

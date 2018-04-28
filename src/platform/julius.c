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
#include "input/cursor.h"
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
    int width;
    int height;
} Desktop;

static struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Cursor *cursors[CURSOR_MAX];
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

void system_set_cursor(int cursor_id)
{
    SDL_SetCursor(SDL.cursors[cursor_id]);
}

static SDL_Cursor *init_cursor(const cursor *c)
{
    Uint8 data[4*32];
    Uint8 mask[4*32];
    int b = -1;
    for (int i = 0; i < 32 * 32; i++) {
        if (i % 8 ) {
            data[b] <<= 1;
            mask[b] <<= 1;
        } else {
            b++;
            data[b] = mask[b] = 0;
        }
        switch (c->data[i]) {
            case 'X':
                data[b] |= 0x01;
                // fallthrough
            case '.':
                mask[b] |= 0x01;
                break;
        }
    }
    return SDL_CreateCursor(data, mask, 32, 32, c->hotspot_x, c->hotspot_y);
}

void system_init_cursors()
{
    for (int i = 0; i < CURSOR_MAX; i++) {
        SDL.cursors[i] = init_cursor(input_cursor_data(i));
    }
    system_set_cursor(CURSOR_ARROW);
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

static void createWindowAndRenderer(int fullscreen)
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
	if (fullscreen) {
		SDL.window = SDL_CreateWindow(title,
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			Desktop.width, Desktop.height,
			SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN);
	} else {
	    int width, height;
	    setting_window(&width, &height);
		SDL.window = SDL_CreateWindow(title,
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			width, height, SDL_WINDOW_RESIZABLE);
	}
	
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
		printf("Texture created\n");// with scanline %d\n", surface->pitch);
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

static void mainLoop()
{
	SDL_Event event;
	SDL_Event refreshEvent;
	refreshEvent.user.type = SDL_USEREVENT;
	refreshEvent.user.code = USER_EVENT_REFRESH;
	mouse_set_inside_window(1);
	
	refresh();
	while (1) {
		int active = 1;
		/* Process event queue */
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_WINDOWEVENT:
					switch (event.window.event) {
						case SDL_WINDOWEVENT_ENTER:
							active = 1;
							mouse_set_inside_window(1);
							break;
						case SDL_WINDOWEVENT_LEAVE:
							mouse_set_inside_window(0);
							active = 0;
							break;
						case SDL_WINDOWEVENT_SIZE_CHANGED:
						//case SDL_WINDOWEVENT_RESIZED:
							printf("System resize to %d x %d\n", event.window.data1, event.window.data2);
							createSurface(event.window.data1, event.window.data2, setting_fullscreen());
							window_invalidate();
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
					mouse_set_position(event.motion.x, event.motion.y);
					if (event.button.button == SDL_BUTTON_LEFT) {
						mouse_set_left_down(1);
					} else if (event.button.button == SDL_BUTTON_RIGHT) {
						mouse_set_right_down(1);
					}
					break;
				
				case SDL_MOUSEBUTTONUP:
					mouse_set_position(event.button.x, event.button.y);
					if (event.button.button == SDL_BUTTON_LEFT) {
						mouse_set_left_down(0);
					} else if (event.button.button == SDL_BUTTON_RIGHT) {
						mouse_set_right_down(0);
					}
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
						SDL_SetWindowSize(SDL.window, *(int*)event.user.data1, *(int*)event.user.data2);
						SDL_SetWindowFullscreen(SDL.window, 0);
						printf("User resize to %d x %d\n", *(int*)event.user.data1, *(int*)event.user.data2);
						setting_set_display(0, *(int*)event.user.data1, *(int*)event.user.data2);
						//createSurface(*(int*)event.user.data1, *(int*)event.user.data2, 0);
						//window_invalidate();
					} else if (event.user.code == USER_EVENT_FULLSCREEN) {
						printf("User to fullscreen %d x %d\n", Desktop.width, Desktop.height);
						SDL_SetWindowSize(SDL.window, Desktop.width, Desktop.height);
						SDL_DisplayMode mode;
						mode.w = Desktop.width;
						mode.h = Desktop.height;
						SDL_SetWindowDisplayMode(SDL.window, &mode);
						SDL_SetWindowFullscreen(SDL.window, SDL_WINDOW_FULLSCREEN);
						setting_set_display(1, Desktop.width, Desktop.height);
						//createSurface(Desktop.width, Desktop.height, 1);
						//window_invalidate();
					} else if (event.user.code == USER_EVENT_WINDOWED) {
					    int width, height;
					    setting_window(&width, &height);
						printf("User to windowed %d x %d\n", width, height);
						SDL_SetWindowSize(SDL.window, width, height);
						SDL_SetWindowFullscreen(SDL.window, 0);
						setting_set_display(0, width, height);
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
	
	SDL_DisplayMode mode;
	SDL_GetDesktopDisplayMode(0, &mode);
	
	printf("Current resolution: %d x %d\n", mode.w, mode.h);
	Desktop.width = mode.w;
	Desktop.height = mode.h;
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
	setting_window(&width, &height);
	createWindowAndRenderer(setting_fullscreen());
	if (setting_fullscreen()) {
		createSurface(Desktop.width, Desktop.height, 1);
	} else {
		createSurface(width, height, 0);
	}
	
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

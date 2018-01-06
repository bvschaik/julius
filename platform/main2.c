#include "SDL.h"
#include <stdio.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "../src/UI/Window.h"
#include "../src/core/time.h"
#include "../src/Screen.h"
#include "../src/input/cursor.h"
#include "../src/input/hotkey.h"
#include "../src/input/keyboard.h"
#include "../src/Widget.h" // debug
#include "../src/Graphics.h" // debug
#include "../src/System.h"
#include "../src/Game.h"

// debug data:
#include "../src/Data/CityInfo.h"
#include "../src/Data/Screen.h"

#include "core/lang.h"
#include "game/settings.h"
#include "game/file.h"
#include "input/mouse.h"

#ifndef __MINGW32__
#include <execinfo.h>
#endif

#include <signal.h>

static struct {
	int width;
	int height;
} Desktop;

static struct {
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
} SDL;

static int autopilot = 0;

static SDL_Cursor *Cursors[CURSOR_MAX];

enum {
	UserEventRefresh = 0,
	UserEventQuit = 1,
	UserEventResize = 2,
	UserEventFullscreen = 3,
	UserEventWindowed = 4,
};

void handler(int sig) {
#ifndef __MINGW32__
	void *array[100];
	size_t size;
	
	// get void*'s for all entries on the stack
	size = backtrace(array, 100);
	
	// print out all the frames to stderr
	fprintf(stderr, "Error: signal %d:\n", sig);
	backtrace_symbols_fd(array, size, STDERR_FILENO);
	exit(1);
#endif
}

void System_exit()
{
	SDL_Event event;
	event.user.type = SDL_USEREVENT;
	event.user.code = UserEventQuit;
	SDL_PushEvent(&event);
}

void System_resize(int width, int height)
{
	static int sWidth;
	static int sHeight;
	sWidth = width;
	sHeight = height;
	SDL_Event event;
	event.user.type = SDL_USEREVENT;
	event.user.code = UserEventResize;
	event.user.data1 = &sWidth;
	event.user.data2 = &sHeight;
	SDL_PushEvent(&event);
}

void System_toggleFullscreen()
{
	SDL_Event event;
	event.user.type = SDL_USEREVENT;
	if (setting_fullscreen()) {
		event.user.code = UserEventWindowed;
	} else {
		event.user.code = UserEventFullscreen;
	}
	SDL_PushEvent(&event);
}

void System_setCursor(int cursorId)
{
	if (autopilot) return;
	SDL_SetCursor(Cursors[cursorId]);
}

static SDL_Cursor *initCursor(const cursor *c)
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
				mask[b] |= 0x01;
				break;
			case '.':
				mask[b] |= 0x01;
				break;
		}
	}
	return SDL_CreateCursor(data, mask, 32, 32, c->hotspotX, c->hotspotY);
}

void System_initCursors()
{
	if (autopilot) return;
	for (int i = 0; i < CURSOR_MAX; i++) {
		Cursors[i] = initCursor(input_cursor_data(i));
	}
	System_setCursor(CURSOR_ARROW);
}

static void runTicks(int ticks)
{
	int originalSpeed = setting_game_speed();
	setting_reset_speeds(100, setting_scroll_speed());
	time_set_millis(0);
	for (int i = 1; i <= ticks; i++) {
		UI_Window_goTo(Window_City);
		time_set_millis(2 * i);
		Game_run();
	}
	setting_reset_speeds(originalSpeed, setting_scroll_speed());
}

static int runAutopilot(const char *savedGameToLoad, const char *savedGameToWrite, int ticksToRun)
{
	autopilot = 1;
	printf("Running autopilot: %s --> %s in %d ticks\n", savedGameToLoad, savedGameToWrite, ticksToRun);
	signal(SIGSEGV, handler);
	
	// C3 setup
	chdir("../data");
	
	if (!Game_preInit()) {
		return 1;
	}
	
	if (!Game_init()) {
		return 2;
	}
	
	game_file_load_saved_game(savedGameToLoad);
	runTicks(ticksToRun);
	printf("Saving game to %s\n", savedGameToWrite);
	game_file_write_saved_game(savedGameToWrite);
	printf("Done\n");
	
	Game_exit();

	return 0;
}

static Uint32 last;

static void refresh()
{
	static Uint32 lastFpsTime = 0;
	static int lastFps = 0;
	static int numFrames = 0;
	
	Uint32 now = SDL_GetTicks();
	time_set_millis(now);
	Game_run();
	
	// debug
	Uint32 then = SDL_GetTicks();
	
	Game_draw();
	numFrames++;
	Uint32 then2 = SDL_GetTicks();
	if (then2 - lastFpsTime > 1000) {
		lastFps = numFrames;
		lastFpsTime = then2;
		numFrames = 0;
	}
	Graphics_fillRect(Data_Screen.width - 120, 0, Data_Screen.width, 20, COLOR_WHITE);
	Widget_Text_drawNumberColored(lastFps, 'f', "", Data_Screen.width - 120, 5, FONT_NORMAL_PLAIN, COLOR_RED);
	Widget_Text_drawNumberColored(then - now, 'g', "", Data_Screen.width - 70, 5, FONT_NORMAL_PLAIN, COLOR_RED);
	Widget_Text_drawNumberColored(then2 - then, 'd', "", Data_Screen.width - 40, 5, FONT_NORMAL_PLAIN, COLOR_RED);
	
	SDL_UpdateTexture(SDL.texture, NULL, Data_Screen.drawBuffer, Data_Screen.width * 4);
	SDL_RenderCopy(SDL.renderer, SDL.texture, NULL, NULL);
	SDL_RenderPresent(SDL.renderer);
	last = now;
}

static void handleKey(SDL_KeyboardEvent *event)
{
	switch (event->keysym.sym) {
		case SDLK_RETURN:
		case SDLK_KP_ENTER:
			keyboard_return();
			break;
		case SDLK_BACKSPACE:
			keyboard_backspace();
			break;
		case SDLK_DELETE:
			keyboard_delete();
			break;
		case SDLK_INSERT:
			keyboard_insert();
			break;
		case SDLK_LEFT:
			keyboard_left();
			hotkey_left();
			break;
		case SDLK_RIGHT:
			keyboard_right();
			hotkey_right();
			break;
		case SDLK_UP:
			keyboard_left();
			hotkey_up();
			break;
		case SDLK_DOWN:
			keyboard_right();
			hotkey_down();
			break;
		case SDLK_HOME:
			keyboard_home();
			hotkey_home();
			break;
		case SDLK_END:
			keyboard_end();
			hotkey_end();
			break;
		case SDLK_ESCAPE:
			hotkey_esc();
			break;
		case SDLK_F1: hotkey_func(1); break;
		case SDLK_F2: hotkey_func(2); break;
		case SDLK_F3: hotkey_func(3); break;
		case SDLK_F4: hotkey_func(4); break;
		case SDLK_F5: hotkey_func(5); break;
		case SDLK_F6: hotkey_func(6); break;
		case SDLK_F7: hotkey_func(7); break;
		case SDLK_F8: hotkey_func(8); break;
		case SDLK_F9: hotkey_func(9); break;
		case SDLK_F10: hotkey_func(10); break;
		case SDLK_F11: hotkey_func(11); break;
		case SDLK_F12: hotkey_func(12); break;
		case SDLK_LCTRL:
		case SDLK_RCTRL:
			hotkey_ctrl(1);
			break;
		case SDLK_LALT:
		case SDLK_RALT:
			hotkey_alt(1);
			break;
		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
			hotkey_shift(1);
			break;
		default:
			if ((event->keysym.sym & SDLK_SCANCODE_MASK) == 0) {
				hotkey_character(event->keysym.sym);
			}
			break;
	}
}

static void handleKeyUp(SDL_KeyboardEvent *event)
{
	switch (event->keysym.sym) {
		case SDLK_LCTRL:
		case SDLK_RCTRL:
			hotkey_ctrl(0);
			break;
		case SDLK_LALT:
		case SDLK_RALT:
			hotkey_alt(0);
			break;
		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
			hotkey_shift(0);
			break;
		default:
			break;
	}
}

static void handleText(SDL_TextInputEvent *event)
{
	if (event->text[0] && !event->text[1]) {
		keyboard_character(event->text[0]);
	}
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
		Screen_setResolution(width, height);
	} else {
		printf("Unable to create texture: %s\n", SDL_GetError());
	}
}

static void mainLoop()
{
	SDL_Event event;
	SDL_Event refreshEvent;
	refreshEvent.user.type = SDL_USEREVENT;
	refreshEvent.user.code = UserEventRefresh;
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
							UI_Window_requestRefresh();
							break;
					}
					break;
				
				case SDL_KEYDOWN:
					printf("Key: sym %d\n", event.key.keysym.sym);
					handleKey(&event.key);
					break;
				
				case SDL_KEYUP:
					handleKeyUp(&event.key);
					break;
				
				case SDL_TEXTINPUT:
					handleText(&event.text);
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
					if (event.user.code == UserEventQuit) {
						return;
					} else if (event.user.code == UserEventResize) {
						SDL_SetWindowSize(SDL.window, *(int*)event.user.data1, *(int*)event.user.data2);
						SDL_SetWindowFullscreen(SDL.window, 0);
						printf("User resize to %d x %d\n", *(int*)event.user.data1, *(int*)event.user.data2);
						setting_set_display(0, *(int*)event.user.data1, *(int*)event.user.data2);
						//createSurface(*(int*)event.user.data1, *(int*)event.user.data2, 0);
						//UI_Window_requestRefresh();
					} else if (event.user.code == UserEventFullscreen) {
						printf("User to fullscreen %d x %d\n", Desktop.width, Desktop.height);
						SDL_SetWindowSize(SDL.window, Desktop.width, Desktop.height);
						SDL_DisplayMode mode;
						mode.w = Desktop.width;
						mode.h = Desktop.height;
						SDL_SetWindowDisplayMode(SDL.window, &mode);
						SDL_SetWindowFullscreen(SDL.window, SDL_WINDOW_FULLSCREEN);
						setting_set_display(1, Desktop.width, Desktop.height);
						//createSurface(Desktop.width, Desktop.height, 1);
						//UI_Window_requestRefresh();
					} else if (event.user.code == UserEventWindowed) {
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
	if (argc == 4) {
		return runAutopilot(argv[1], argv[2], atoi(argv[3]));{}
	}
	signal(SIGSEGV, handler);
	
	initSdl();
	
	// C3 setup
	char cwd[200];
	getcwd(cwd, 200);
	printf("current working directory: %s\n", cwd);
	if (chdir("../data") != 0) {
		printf("data directory not found!!\n");
	}

	if (!Game_preInit()) {
		return 1;
	}
	
	int width, height;
	setting_window(&width, &height);
	createWindowAndRenderer(setting_fullscreen());
	if (setting_fullscreen()) {
		createSurface(Desktop.width, Desktop.height, 1);
	} else {
		createSurface(width, height, 0);
	}
	
	if (!Game_init()) {
		return 2;
	}

	mainLoop();
	
	printf("Quiting SDL.\n");
	
	Game_exit();

	// Shutdown all subsystems
	SDL_Quit();
	
	printf("Quiting....\n");
	
	exit(0);
}

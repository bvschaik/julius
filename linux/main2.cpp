#include "SDL.h"
#include <stdio.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "../src/UI/Window.h"
#include "../src/Time.h"
#include "../src/Runner.h"
#include "../src/Screen.h"
#include "../src/Data/AllData.h"
#include "../src/Cursor.h"
#include "../src/Language.h"
#include "../src/KeyboardInput.h"
#include "../src/KeyboardHotkey.h"
#include "../src/Widget.h" // debug
#include "../src/Graphics.h" // debug
#include "../src/System.h"
#include "../src/Game.h"

#include <execinfo.h>
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

static SDL_Cursor *Cursors[3];

enum {
	UserEventRefresh = 0,
	UserEventQuit = 1,
	UserEventResize = 2,
	UserEventFullscreen = 3,
	UserEventWindowed = 4,
};

void handler(int sig) {
	void *array[100];
	size_t size;
	
	// get void*'s for all entries on the stack
	size = backtrace(array, 100);
	
	// print out all the frames to stderr
	fprintf(stderr, "Error: signal %d:\n", sig);
	backtrace_symbols_fd(array, size, STDERR_FILENO);
	exit(1);
}

void assert(const char *msg, int expected, int actual)
{
	if (expected != actual) {
		printf("Assert failed: %s; expected: %d, actual %d\n", msg, expected, actual);
	}
}

void sanityCheck()
{
	assert("Scenario settings", 1720, sizeof(Data_Scenario));
	assert("Empire object", 64, sizeof(struct Data_Empire_Object));
	assert("Empire object list", 12800, sizeof(Data_Empire_Objects));
	assert("Trade city", 66, sizeof(struct Data_Empire_City));
	assert("Trade city list", 2706, sizeof(Data_Empire_Cities));
	assert("City info", 2*18068, sizeof(Data_CityInfo));
	assert("Building object", 128, sizeof(struct Data_Building));
	assert("City sound", 128, sizeof(struct Data_Sound_City));
	assert("Formation", 128, sizeof(struct Data_Formation));
	assert("Storage", 32, sizeof(struct Data_Building_Storage));
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
	if (Data_Settings.fullscreen) {
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

static SDL_Cursor *initCursor(const struct Cursor *cursor)
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
		switch (cursor->data[i]) {
			case 'X':
				data[b] |= 0x01;
				mask[b] |= 0x01;
				break;
			case '.':
				mask[b] |= 0x01;
				break;
		}
	}
	return SDL_CreateCursor(data, mask, 32, 32, cursor->hotspotX, cursor->hotspotY);
}

void System_initCursors()
{
	if (autopilot) return;
	for (int i = 0; i < 3; i++) {
		Cursors[i] = initCursor(Cursor_getData(i));
	}
	System_setCursor(0);
}
/*
typedef struct{
	SDL_Palette *palette;
	Uint8  BitsPerPixel;
	Uint8  BytesPerPixel;
	Uint32 Rmask, Gmask, Bmask, Amask;
	Uint8  Rshift, Gshift, Bshift, Ashift;
	Uint8  Rloss, Gloss, Bloss, Aloss;
	Uint32 colorkey;
	Uint8  alpha;
} SDL_PixelFormat;
*/

#include "../src/GameFile.h"

void runTicks(int ticks)
{
	int originalSpeed = Data_Settings.gameSpeed;
	Data_Settings.gameSpeed = 100;
	Time_setMillis(0);
	for (int i = 1; i <= ticks; i++) {
		UI_Window_goTo(Window_City);
		Time_setMillis(2 * i);
		Runner_run();
	}
	Data_Settings.gameSpeed = originalSpeed;
}

int runAutopilot(const char *savedGameToLoad, const char *savedGameToWrite, int ticksToRun)
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
	
	GameFile_loadSavedGame(savedGameToLoad);
	runTicks(ticksToRun);
	printf("Saving game to %s\n", savedGameToWrite);
	GameFile_writeSavedGame(savedGameToWrite);
	printf("Done\n");
	
	Game_exit();

	return 0;
}

Uint32 last;

void refresh()
{
	static Uint32 lastFpsTime = 0;
	static int lastFps = 0;
	static int numFrames = 0;
	
	Uint32 now = SDL_GetTicks();
	Time_setMillis(now);
	Runner_run();
	
	// debug
	Uint32 then = SDL_GetTicks();
	
	Runner_draw();
	numFrames++;
	Uint32 then2 = SDL_GetTicks();
	if (then2 - lastFpsTime > 1000) {
		lastFps = numFrames;
		lastFpsTime = then2;
		numFrames = 0;
	}
	Graphics_fillRect(Data_Screen.width - 120, 0, Data_Screen.width, 20, Color_White);
	Widget_Text_drawNumberColored(lastFps, 'f', "", Data_Screen.width - 120, 5, Font_NormalPlain, 0xf800);
	Widget_Text_drawNumberColored(then - now, 'g', "", Data_Screen.width - 70, 5, Font_NormalPlain, 0xf800);
	Widget_Text_drawNumberColored(then2 - then, 'd', "", Data_Screen.width - 40, 5, Font_NormalPlain, 0xf800);
	
	SDL_UpdateTexture(SDL.texture, NULL, Data_Screen.drawBuffer, Data_Screen.width * 4);
	SDL_RenderClear(SDL.renderer);
	SDL_RenderCopy(SDL.renderer, SDL.texture, NULL, NULL);
	SDL_RenderPresent(SDL.renderer);
	last = now;
}

void handleKey(SDL_KeyboardEvent *event)
{
	switch (event->keysym.sym) {
		case SDLK_RETURN:
		case SDLK_KP_ENTER:
			KeyboardInput_return();
			break;
		case SDLK_BACKSPACE:
			KeyboardInput_backspace();
			break;
		case SDLK_DELETE:
			KeyboardInput_delete();
			break;
		case SDLK_INSERT:
			KeyboardInput_insert();
			break;
		case SDLK_LEFT:
			KeyboardInput_left();
			KeyboardHotkey_left();
			break;
		case SDLK_RIGHT:
			KeyboardInput_right();
			KeyboardHotkey_right();
			break;
		case SDLK_UP:
			KeyboardInput_left();
			KeyboardHotkey_up();
			break;
		case SDLK_DOWN:
			KeyboardInput_right();
			KeyboardHotkey_down();
			break;
		case SDLK_HOME:
			KeyboardInput_home();
			KeyboardHotkey_home();
			break;
		case SDLK_END:
			KeyboardInput_end();
			KeyboardHotkey_end();
			break;
		case SDLK_ESCAPE:
			KeyboardHotkey_esc();
			break;
		case SDLK_F1: KeyboardHotkey_func(1); break;
		case SDLK_F2: KeyboardHotkey_func(2); break;
		case SDLK_F3: KeyboardHotkey_func(3); break;
		case SDLK_F4: KeyboardHotkey_func(4); break;
		case SDLK_F5: KeyboardHotkey_func(5); break;
		case SDLK_F6: KeyboardHotkey_func(6); break;
		case SDLK_F7: KeyboardHotkey_func(7); break;
		case SDLK_F8: KeyboardHotkey_func(8); break;
		case SDLK_F9: KeyboardHotkey_func(9); break;
		case SDLK_F10: KeyboardHotkey_func(10); break;
		case SDLK_F11: KeyboardHotkey_func(11); break;
		case SDLK_F12: KeyboardHotkey_func(12); break;
		case SDLK_LCTRL:
		case SDLK_RCTRL:
			KeyboardHotkey_ctrl(1);
			break;
		case SDLK_LALT:
		case SDLK_RALT:
			KeyboardHotkey_alt(1);
			break;
		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
			KeyboardHotkey_shift(1);
			break;
		default:
			if ((event->keysym.sym & SDLK_SCANCODE_MASK) == 0) {
				KeyboardHotkey_character(event->keysym.sym);
			}
			break;
	}
}

static void handleKeyUp(SDL_KeyboardEvent *event)
{
	switch (event->keysym.sym) {
		case SDLK_LCTRL:
		case SDLK_RCTRL:
			KeyboardHotkey_ctrl(0);
			break;
		case SDLK_LALT:
		case SDLK_RALT:
			KeyboardHotkey_alt(0);
			break;
		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
			KeyboardHotkey_shift(0);
			break;
		default:
			break;
	}
}

void handleText(SDL_TextInputEvent *event)
{
	if (event->text[0] && !event->text[1]) {
		KeyboardInput_character(event->text[0]);
	}
}

void createWindowAndRenderer(int fullscreen)
{
	if (SDL.window) {
		SDL_DestroyWindow(SDL.window);
		SDL.window = 0;
	}
	if (SDL.renderer) {
		SDL_DestroyRenderer(SDL.renderer);
		SDL.renderer = 0;
	}
	
	const char *title = Language_getString(9, 0);
	if (fullscreen) {
		SDL.window = SDL_CreateWindow(title,
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			0, 0,
			SDL_WINDOW_FULLSCREEN_DESKTOP);
	} else {
		SDL.window = SDL_CreateWindow(title,
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			Data_Settings.windowedWidth, Data_Settings.windowedHeight,
			SDL_WINDOW_RESIZABLE);
	}
	
	SDL.renderer = SDL_CreateRenderer(SDL.window, -1, 0);
}

void createSurface(int width, int height, int fullscreen)
{
	/*Uint32 flags = SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_ANYFORMAT;
	if (fullscreen) {
		flags |= SDL_FULLSCREEN;
	} else {
		flags |= SDL_RESIZABLE;
	}*/
	if (SDL.texture) {
		SDL_DestroyTexture(SDL.texture);
		SDL.texture = 0;
	}
	
	Data_Settings.fullscreen = fullscreen;
	if (!fullscreen) {
		Data_Settings.windowedWidth = width;
		Data_Settings.windowedHeight = height;
	}
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

void mainLoop()
{
	SDL_Event event;
	SDL_Event refreshEvent;
	refreshEvent.user.type = SDL_USEREVENT;
	refreshEvent.user.code = UserEventRefresh;
	Data_Mouse.isInsideWindow = 1;
	
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
							Data_Mouse.isInsideWindow = 1;
							break;
						case SDL_WINDOWEVENT_LEAVE:
							Data_Mouse.isInsideWindow = 0;
							active = 0;
							break;
						case SDL_WINDOWEVENT_RESIZED:
							printf("System resize to %d x %d\n", event.window.data1, event.window.data2);
							createSurface(event.window.data1, event.window.data2, Data_Settings.fullscreen);
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
					Data_Mouse.x = event.motion.x;
					Data_Mouse.y = event.motion.y;
					break;
				
				case SDL_MOUSEBUTTONDOWN:
					Data_Mouse.x = event.button.x;
					Data_Mouse.y = event.button.y;
					if (event.button.button == SDL_BUTTON_LEFT) {
						Data_Mouse.leftDown = 1;
					} else if (event.button.button == SDL_BUTTON_RIGHT) {
						Data_Mouse.rightDown = 1;
					}
					break;
				
				case SDL_MOUSEBUTTONUP:
					Data_Mouse.x = event.button.x;
					Data_Mouse.y = event.button.y;
					if (event.button.button == SDL_BUTTON_LEFT) {
						Data_Mouse.leftDown = 0;
					} else if (event.button.button == SDL_BUTTON_RIGHT) {
						Data_Mouse.rightDown = 0;
					}
					break;
				
				case SDL_MOUSEWHEEL:
					if (event.wheel.y > 0) {
						Data_Mouse.scrollUp = 1;
					} else if (event.wheel.y < 0) {
						Data_Mouse.scrollDown = 1;
					}
					break;

				
				/*case SDL_VIDEORESIZE:
					printf("Resize to %d x %d\n", event.resize.w, event.resize.h);
					surface = createSurface(event.resize.w, event.resize.h, 0);
					UI_Window_requestRefresh();
					break;*/
				
				case SDL_QUIT:
					return;
				
				case SDL_USEREVENT:
					if (event.user.code == UserEventQuit) {
						return;
					} else if (event.user.code == UserEventResize) {
						SDL_SetWindowFullscreen(SDL.window, 0);
						SDL_SetWindowSize(SDL.window, *(int*)event.user.data1, *(int*)event.user.data2);
						printf("User resize to %d x %d\n", *(int*)event.user.data1, *(int*)event.user.data2);
						createSurface(*(int*)event.user.data1, *(int*)event.user.data2, 0);
						UI_Window_requestRefresh();
					} else if (event.user.code == UserEventFullscreen) {
						SDL_SetWindowFullscreen(SDL.window, SDL_WINDOW_FULLSCREEN_DESKTOP);
						SDL_DisplayMode mode;
						mode.w = Desktop.width;
						mode.h = Desktop.height;
						SDL_SetWindowDisplayMode(SDL.window, &mode);
						createSurface(Desktop.width, Desktop.height, 1);
						UI_Window_requestRefresh();
					} else if (event.user.code == UserEventWindowed) {
						SDL_SetWindowFullscreen(SDL.window, 0);
						SDL_SetWindowSize(SDL.window, Data_Settings.windowedWidth, Data_Settings.windowedHeight);
						createSurface(Data_Settings.windowedWidth, Data_Settings.windowedHeight, 0);
						UI_Window_requestRefresh();
					}
					break;
				
				default:
					printf("Unknown event: %d\n", event.type);
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

void initSdl()
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
	
	/*int closestMode = SDL_VideoModeOK(800, 600, 16, 0);
	if (closestMode) {
		printf("VIDEO OK with bpp: %d\n", closestMode);
	} else {
		printf("VIDEO NOT OK\n");
	}*/
}

int main(int argc, char **argv)
{
	if (argc == 4) {
		return runAutopilot(argv[1], argv[2], atoi(argv[3]));
	}
	signal(SIGSEGV, handler);
	
	sanityCheck();
	
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
	
	createWindowAndRenderer(Data_Settings.fullscreen);
	if (Data_Settings.fullscreen) {
		createSurface(Desktop.width, Desktop.height, 1);
	} else {
		createSurface(Data_Settings.windowedWidth, Data_Settings.windowedHeight, 0);
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

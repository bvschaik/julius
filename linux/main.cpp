#include "SDL.h"   /* All SDL App's need this */
#include <stdio.h>

#include <SDL/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "../src/UI/Window.h"
#include "../src/Data/Screen.h"
#include "../src/Data/Mouse.h"
#include "../src/Data/CityInfo.h"
#include "../src/Data/Scenario.h"
#include "../src/Data/Settings.h"
#include "../src/Loader.h"
#include "../src/Empire.h"
#include "../src/Language.h"
#include "../src/Graphics.h"
#include "../src/GameFile.h"
#include "../src/Time.h"
#include "../src/Animation.h"
#include "../src/Sound.h"
#include "../src/CityView.h"
#include "../src/Data/AllData.h"
#include "../src/KeyboardInput.h"

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

void refresh(SDL_Surface *surface) {
	if (SDL_MUSTLOCK(surface)) {
		if (SDL_LockSurface(surface) < 0) {
			printf("Error locking surface: %s\n", SDL_GetError());
			abort();
		}
    }
	
	Data_Screen.drawBuffer = (Color*)surface->pixels;
	
	Time_setMillis(SDL_GetTicks());
	Animation_updateTimers();
	UI_Window_refresh(1);
	//UI_MainMenu_drawBackground();
	//UI_MainMenu_drawForeground();
	//UI_MainMenu_handleMouse();
	
	if (SDL_MUSTLOCK(surface)) {
		SDL_UnlockSurface(surface);
	}
	//SDL_UpdateRect(surface, 0, 0, 0, 0);
	SDL_Flip(surface);
	//printf("Refresh: %d ms\n", SDL_GetTicks() - Time_getMillis());
}

void handleKey(SDL_KeyboardEvent *event)
{
	switch (event->keysym.sym) {
		case SDLK_RETURN:
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
			break;
		case SDLK_RIGHT:
			KeyboardInput_right();
			break;
		case SDLK_HOME:
			KeyboardInput_home();
			break;
		case SDLK_END:
			KeyboardInput_end();
			break;
		default:
			if (event->keysym.unicode) {
				KeyboardInput_character(event->keysym.unicode);
			}
			break;
	}
}

void mainLoop(SDL_Surface *surface)
{
	SDL_Event event;
	SDL_Event refreshEvent;
	refreshEvent.user.type = SDL_USEREVENT;
	
	refresh(surface);
    /* While the program is running */
	SDL_EnableUNICODE(1);
    while (1) {
		int active = 1;
        /* Process event queue */
        while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_ACTIVEEVENT:
					if (event.active.gain) {
						active = 1;
					} else {
						active = 0;
					}
					break;
				
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_q || event.key.keysym.sym == SDLK_ESCAPE) {
						return;
					}
					handleKey(&event.key);
					printf("Key: %d (%c)\n", event.key.keysym.unicode, event.key.keysym.unicode);
					break;
				
				case SDL_KEYUP:
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
					} else if (event.button.button == SDL_BUTTON_WHEELUP) {
						Data_Mouse.scrollUp = 1;
					} else if (event.button.button == SDL_BUTTON_WHEELDOWN) {
						Data_Mouse.scrollDown = 1;
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
				
				case SDL_VIDEORESIZE:
					break;
				
				case SDL_QUIT:
					return;
				
				case SDL_USEREVENT:
					break;
				
				default:
					printf("Unknown event: %d\n", event.type);
					break;
			}
        }
		// Push user refresh event
		SDL_PushEvent(&refreshEvent);
		if (active) {
			refresh(surface);
		} else {
			SDL_WaitEvent(NULL);
		}
    }
}

int main()
{
	sanityCheck();
	printf("Initializing SDL.\n");
	
	// Initialize defaults, Video and Audio
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) == -1) {
		printf("Could not initialize SDL: %s.\n", SDL_GetError());
		exit(-1);
	}
	
	printf("SDL initialized.\n");
	
	SDL_PixelFormat format;
	format.palette = 0;
	format.BitsPerPixel = 32;
	
	SDL_Rect **modes;
	
	const SDL_VideoInfo *vidInfo = SDL_GetVideoInfo();
	printf("Current resolution: %d x %d\n", vidInfo->current_w, vidInfo->current_h);
	
	int closestMode = SDL_VideoModeOK(800, 600, 16, 0);
	if (closestMode) {
		printf("VIDEO OK with bpp: %d\n", closestMode);
	} else {
		printf("VIDEO NOT OK\n");
	}
	
	//SDL_Surface *surface = SDL_SetVideoMode(800, 600, 16, /*SDL_FULLSCREEN|*/SDL_HWSURFACE|SDL_DOUBLEBUF);
	SDL_Surface *surface = SDL_SetVideoMode(
	//	vidInfo->current_w, vidInfo->current_h, 16, /*SDL_FULLSCREEN*/0);
		1680, 1050, 32, SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_ANYFORMAT);
		//1920, 1200, 32, SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_ANYFORMAT|SDL_FULLSCREEN);
	//	1440, 900, 16, 0);
	if (surface) {
		printf("Surface created with scanline %d\n", surface->pitch);
		printf("  flags: %d %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s\n",
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
	}
	
	// Get available fullscreen/hardware modes
	modes = SDL_ListModes(&format, SDL_HWSURFACE|SDL_DOUBLEBUF);
	
	// Check is there are any modes available
	if (modes == (SDL_Rect **) 0) {
		printf("No modes available!\n");
		exit(-1);
	}
	
	// Check if or resolution is restricted
	if (modes == (SDL_Rect **) -1) {
		printf("All resolutions available.\n");
	} else {
		// Print valid modes
		printf("Available Modes\n");
		for (int i = 0; modes[i]; ++i) {
			printf("  %d x %d\n", modes[i]->w, modes[i]->h);
		}
	}
	
	
	
	// C3 setup
	
	chdir("../data");
	Sound_init();
	Data_Screen.format = 565; // TODO derive later
	Data_Screen.width = vidInfo->current_w;
	Data_Screen.height = vidInfo->current_h;
	Data_Screen.offset640x480.x = (Data_Screen.width - 640) / 2;
	Data_Screen.offset640x480.y = (Data_Screen.height - 480) / 2;
	
	//TODO real settings loading
	Data_Settings.soundEffectsEnabled = 1;
	Data_Settings.soundMusicEnabled = 1;
	Data_Settings.soundSpeechEnabled = 1;
	Data_Settings.soundCityEnabled = 1;
	Data_Settings.soundEffectsPercentage = 100;
	Data_Settings.soundMusicPercentage = 100;
	Data_Settings.soundSpeechPercentage = 100;
	Data_Settings.soundCityPercentage = 100;
	// end settings
	
	Graphics_setClipRectangle(0, 0, Data_Screen.width, Data_Screen.height);
	
	Loader_Graphics_initGraphics();
	
	Loader_GameState_init();
	
	printf("Load images: %d\n", Loader_Graphics_loadMainGraphics(2));
	printf("Load model: %d\n", Loader_Model_loadC3ModelTxt());
	printf("Load language: %d\n", Language_load("c3.eng", "c3_mm.eng"));
	UI_Window_goTo(Window_MainMenu);
	
	GameFile_loadSavedGame("1.sav");
	printf("map width = %d\n", Data_Settings_Map.width);
	CityView_calculateLookup(); // TODO should be part of loading file
	Empire_load(0, Data_Scenario.empireId);
	//Data_CityInfo.godWrathMercury = 40;
	// end C3 setup
	
	mainLoop(surface);
	
	//SDL_Delay(5000);
	
	printf("Quiting SDL.\n");
	
	// Shutdown all subsystems
	Sound_shutdown();
	SDL_Quit();
	
	printf("Quiting....\n");
	
	exit(0);
}

#import "ios.h"

#import "GameDataPickerController.h"
#import "SDL.h"
#import "SDL_messagebox.h"
#import "SDL_system.h"
#import "SDL_syswm.h"
#import "SDL_video.h"

#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

#define GAME_PATH_MAX 300
static int has_directory;
static char path[GAME_PATH_MAX];
static UIWindow *window = nil;
static UIWindow *alertwindow = nil;

void c3_path_chosen(char *new_path) {
    unsigned long len = strlen(new_path);
    snprintf(path, GAME_PATH_MAX, "%s", new_path);
    has_directory = true;
}

const char *ios_get_base_path(void) {
    NSURL *gameDataDirectory = [[[[NSFileManager defaultManager] URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask] lastObject] URLByAppendingPathComponent:@"C3" isDirectory:YES];
    
    [gameDataDirectory.path getFileSystemRepresentation:path maxLength:300];
    
    return path;
}

const char *ios_show_c3_path_dialog(int again)
{
    if (window == nil) {
        window = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
        window.rootViewController = [UIViewController new];
        window.windowLevel = UIWindowLevelAlert;

        [window makeKeyAndVisible];
    }
    
    
    GameDataPickerController *controller = [[GameDataPickerController alloc] initWithWindow: window];
    [controller showInstructions];
    
    
    while (!has_directory) {
        [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:5.0]];
    }

    if (window) {
        window.hidden = YES;
    }
    
    return path;
}

#import "ios.h"
#import "SDL.h"
#import "SDL_syswm.h"
#import "SDL_video.h"
#import "SDL_system.h"
#import "SDL_messagebox.h"
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

#import "GameDataPickerController.h"

#define GAME_PATH_MAX 300
static int has_directory;
static char path[GAME_PATH_MAX];

void c3_path_chosen(char *new_path) {
    unsigned long len = strlen(new_path);
    snprintf(path, GAME_PATH_MAX, "%s", new_path);
    has_directory = true;
}

const char* ios_get_base_path(void) {
    NSURL *gameDataDirectory = [[[[NSFileManager defaultManager] URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask] lastObject] URLByAppendingPathComponent:@"C3" isDirectory:YES];
    
    [gameDataDirectory.path getFileSystemRepresentation:path maxLength:300];
    
    return path;
}

int ios_set_base_path(const char *path) {
    CFURLRef url = (__bridge CFURLRef)[NSURL fileURLWithPath: [NSString stringWithCString:path]];
    
    return 1;
}


const char *ios_show_c3_path_dialog(int again)
{
    UIWindow *window = nil;
    UIWindow *alertwindow = nil;

    GameDataPickerController *delegate = [[GameDataPickerController alloc] init];
    UIDocumentPickerViewController *picker = [[UIDocumentPickerViewController alloc] initForOpeningContentTypes: @[UTTypeFolder]];
    
    picker.delegate = delegate;

    NSError *error = nil;
    picker.directoryURL = [[NSFileManager defaultManager] URLForDirectory:NSDocumentDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:YES error:nil];

    if (error) {
        // Handle the error.
    }

    if (window == nil || window.rootViewController == nil) {
        alertwindow = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
        alertwindow.rootViewController = [UIViewController new];
        alertwindow.windowLevel = UIWindowLevelAlert;

        window = alertwindow;

        [alertwindow makeKeyAndVisible];
        
    }

    [window.rootViewController presentViewController:picker animated:YES completion:nil];
    
    while (!has_directory) {
        [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]];
    }

    if (alertwindow) {
        alertwindow.hidden = YES;
    }
    
    return path;
}

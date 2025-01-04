#import "GameDataPickerController.h"

#import "ios.h"
#import "platform/screen.h"
#import "SDL.h"
#import "SDL_system.h"
#import "SDL_syswm.h"
#import "SDL_video.h"

#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

@implementation GameDataPickerController

NSURL *url;

- (void)documentPicker:(UIDocumentPickerViewController *)controller didPickDocumentsAtURLs:(NSArray<NSURL *> *)urls {
    
    NSLog(@"urls: %@", urls);
    NSURL *importDirectory = urls.firstObject;
    [importDirectory startAccessingSecurityScopedResource];
    
    NSURL *gameDataDirectory = [[[[NSFileManager defaultManager] URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask] lastObject] URLByAppendingPathComponent:@"C3" isDirectory:YES];
    
    NSError *error = nil;
    BOOL success = false;
    success = [[NSFileManager defaultManager] copyItemAtPath:importDirectory.path toPath:gameDataDirectory.path error:&error];
    
    if(success) {
        NSLog(@"Files copied to: %@", gameDataDirectory);
    } else {
        NSLog(@"Error copying data files: %@", error);
        return;
    }
    
    [importDirectory stopAccessingSecurityScopedResource];
    
    char path[300];
    [gameDataDirectory.path getFileSystemRepresentation:path maxLength:300];
    c3_path_chosen(path);
}

- (void) documentPickerWasCancelled:(UIDocumentPickerViewController *)controller {
    
}

@end


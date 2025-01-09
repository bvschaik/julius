#import "GameDataPickerController.h"

#import "ios.h"
#import "platform/screen.h"
#import "SDL.h"
#import "SDL_system.h"
#import "SDL_syswm.h"
#import "SDL_video.h"

#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

@interface GameDataPickerController ()
@property NSURL *url;
@property UIWindow *window;
@property UIDocumentPickerViewController *picker;
@end

@implementation GameDataPickerController

- (instancetype)initWithWindow:(UIWindow *)window {
    self = [super init];
    if (self) {
        self.window = window;
    }
    return self;
}

- (void)showInstructions {
   UIAlertController* alert = [UIAlertController alertControllerWithTitle:@"Game Data Required"
                                   message:@"Julius cannot continue without game data. Please select a valid C3 Game Data folder."
                                   preferredStyle:UIAlertControllerStyleAlert];
     
    UIAlertAction* defaultAction = [UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleDefault
                                                          handler:^(UIAlertAction * action) {
        [self showDocumentPicker];
    }];
    
    [alert addAction:defaultAction];
    [self.window.rootViewController presentViewController:alert animated:YES completion:nil];
}

- (void)showDocumentPicker {
    self.picker = [[UIDocumentPickerViewController alloc] initForOpeningContentTypes: @[UTTypeFolder]];
    self.picker.delegate = self;

    [self.window.rootViewController presentViewController:self.picker animated:YES completion:nil];
}

#pragma mark - UIDocumentPickerDelegate methods
- (void)documentPicker:(UIDocumentPickerViewController *)controller didPickDocumentsAtURLs:(NSArray<NSURL *> *)urls {
    
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
        
        // Something has gone horribly wrong.
        UIAlertController* alert = [UIAlertController alertControllerWithTitle:@"Error"
                                       message:[NSString stringWithFormat:@"Could not copy data files. \nReason: %@", error]
                                       preferredStyle:UIAlertControllerStyleAlert];
         
        UIAlertAction* defaultAction = [UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleDefault
                                                              handler:^(UIAlertAction * action) {
            [self showInstructions];
        }];
         
        [alert addAction:defaultAction];
        [self.window.rootViewController presentViewController:alert animated:YES completion:nil];
        
        return;
    }
    
    [importDirectory stopAccessingSecurityScopedResource];
    
    char path[300];
    [gameDataDirectory.path getFileSystemRepresentation:path maxLength:300];
    c3_path_chosen(path);
}

- (void) documentPickerWasCancelled:(UIDocumentPickerViewController *)controller {
    [self showInstructions];
}

@end


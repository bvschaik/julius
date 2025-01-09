#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

@interface GameDataPickerController : NSObject<UIDocumentPickerDelegate>

- (instancetype) initWithWindow:(UIWindow *)window;
- (void) showInstructions;
- (void) showDocumentPicker;

@end

NS_ASSUME_NONNULL_END

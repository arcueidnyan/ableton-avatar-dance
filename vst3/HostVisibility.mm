#import <AppKit/AppKit.h>
bool avatarHostMinimized()
{
    if ([NSApp isHidden]) return true;
    for (NSWindow* window in [NSApp windows])
        if ([window isMiniaturized] && ([window styleMask] & NSWindowStyleMaskResizable))
            return true;
    return false;
}

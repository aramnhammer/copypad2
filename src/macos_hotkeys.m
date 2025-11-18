#ifdef __APPLE__
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <Carbon/Carbon.h>
#import "macos_hotkeys.h"

static bool windowHidden = false;
static EventHotKeyRef hotKeyRef;

OSStatus hotKeyHandler(EventHandlerCallRef nextHandler, EventRef theEvent, void *userData) {
    @autoreleasepool {
        NSArray *windows = [NSApp windows];
        if (windows.count > 0) {
            NSWindow *window = [windows objectAtIndex:0];
            if (windowHidden) {
                [window makeKeyAndOrderFront:nil];
                [NSApp activateIgnoringOtherApps:YES];
                windowHidden = false;
            } else {
                [window orderOut:nil];
                windowHidden = true;
            }
        }
    }
    return noErr;
}

void RegisterGlobalHotkey(void) {
    EventTypeSpec eventType;
    eventType.eventClass = kEventClassKeyboard;
    eventType.eventKind = kEventHotKeyPressed;
    
    InstallApplicationEventHandler(&hotKeyHandler, 1, &eventType, NULL, NULL);
    
    EventHotKeyID hotKeyID;
    hotKeyID.signature = 'htk1';
    hotKeyID.id = 1;
    
    RegisterEventHotKey(kVK_ANSI_H, cmdKey + shiftKey, hotKeyID, 
                       GetApplicationEventTarget(), 0, &hotKeyRef);
}


void UnregisterGlobalHotkey(void) {
    if (hotKeyRef) {
        UnregisterEventHotKey(hotKeyRef);
    }
}

bool IsAppHidden(void) {
    return windowHidden;
}

#endif
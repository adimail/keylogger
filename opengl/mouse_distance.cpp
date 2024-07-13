#include <iostream>
#include <fstream>
#include <ApplicationServices/ApplicationServices.h>
#include <cmath>

// Function to calculate the distance between two points
double calculateDistance(double x1, double y1, double x2, double y2)
{
    return std::sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

// Callback function to handle mouse events
CGEventRef mouseCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon)
{
    static double lastX = -1;
    static double lastY = -1;
    static double totalDistance = 0;

    // Get the current mouse position
    CGPoint currentPosition = CGEventGetLocation(event);

    // If this is the first event, just record the position
    if (lastX == -1 && lastY == -1)
    {
        lastX = currentPosition.x;
        lastY = currentPosition.y;
    }
    else
    {
        // Calculate the distance traveled since the last event
        double distance = calculateDistance(lastX, lastY, currentPosition.x, currentPosition.y);
        totalDistance += distance;

        // Update the last position
        lastX = currentPosition.x;
        lastY = currentPosition.y;

        // Convert the distance to meters (assuming screen resolution of 96 DPI or 37.7952755906 pixels/cm)
        double distanceInMeters = totalDistance / 37.7952755906 / 100.0;

        // Write the total distance to the file
        std::ofstream distFile;
        distFile.open("cursordist.txt", std::ios::trunc);
        distFile << distanceInMeters;
        distFile.close();
    }

    return event;
}

int main()
{
    // Create an event tap for mouse movement
    CFMachPortRef eventTap = CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap, kCGEventTapOptionDefault,
                                              CGEventMaskBit(kCGEventMouseMoved) | CGEventMaskBit(kCGEventLeftMouseDragged) | CGEventMaskBit(kCGEventRightMouseDragged),
                                              mouseCallback, NULL);
    if (!eventTap)
    {
        std::cerr << "Failed to create event tap." << std::endl;
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

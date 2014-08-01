/*
http://stackoverflow.com/questions/614185/window-move-and-resize-apis-in-os-x
*/

/* Carbon includes everything necessary for Accessibilty API */
#include <Carbon/Carbon.h>

static bool isAuthorized() {
    return AXAPIEnabled() || AXIsProcessTrusted();
}

static AXUIElementRef getFrontMostApp() {
    pid_t pid;
    ProcessSerialNumber psn;

    GetFrontProcess(&psn);
    GetProcessPID(&psn, &pid);
    return AXUIElementCreateApplication(pid);
}

int main (int argc, char ** argv) {
    int i;
    AXValueRef temp;
    CGSize windowSize;
    CGPoint windowPosition;
    CFStringRef windowTitle;
    AXUIElementRef frontMostApp;
    AXUIElementRef frontMostWindow;

    if(!isAuthorized()) {
        printf("Can't use accessibility API!\n");
        return 1;
    }

    /* Here we go. Find out which process is front-most */
    frontMostApp = getFrontMostApp();

    /* Get the front most window. We could also get an array of all windows
     * of this process and ask each window if it is front most, but that is
     * quite inefficient if we only need the front most window.
     */
    AXUIElementCopyAttributeValue(
        frontMostApp, kAXFocusedWindowAttribute, (CFTypeRef *)&frontMostWindow
    );

    /* Get the title of the window */
    AXUIElementCopyAttributeValue(
        frontMostWindow, kAXTitleAttribute, (CFTypeRef *)&windowTitle
    );

    /* Get the window size and position */
    AXUIElementCopyAttributeValue(
        frontMostWindow, kAXSizeAttribute, (CFTypeRef *)&temp
    );
    AXValueGetValue(temp, kAXValueCGSizeType, &windowSize);
    CFRelease(temp);

    AXUIElementCopyAttributeValue(
        frontMostWindow, kAXPositionAttribute, (CFTypeRef *)&temp
    );
    AXValueGetValue(temp, kAXValueCGPointType, &windowPosition);
    CFRelease(temp);

    /* Print everything */
    printf("\n");
    CFShow(windowTitle);
    printf(
        "Window is at (%f, %f) and has dimension of (%f, %f)\n",
        windowPosition.x,
        windowPosition.y,
        windowSize.width,
        windowSize.height
    );

    /* Move the window to the right by 25 pixels */
    windowPosition.x += 25;
    temp = AXValueCreate(kAXValueCGPointType, &windowPosition);
    AXUIElementSetAttributeValue(frontMostWindow, kAXPositionAttribute, temp);
    CFRelease(temp);

    /* Clean up */
    CFRelease(frontMostWindow);
    CFRelease(frontMostApp);
    return 0;
}

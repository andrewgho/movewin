#include "winutils.h"

#define ME "movewin"
#define USAGE "usage: " ME " title x y [width height]\n"

typedef struct {
    int x;
    int y;
    int width;
    int height;
} WindowGeometry;

WindowGeometry WGCreate(int x, int y, int width, int height) {
    WindowGeometry geo;

    geo.x = x;
    geo.y = y;
    geo.width = width;
    geo.height = height;

    return geo;
}

bool WGEqual(WindowGeometry geo1, WindowGeometry geo2) {
    return
        geo1.x == geo2.x &&
        geo1.y == geo2.y &&
        geo1.width == geo2.width &&
        geo1.height == geo2.height;
}

static bool isAuthorized() {
    return AXAPIEnabled() || AXIsProcessTrusted();
}

WindowGeometry CFDictionaryGetBounds(CFDictionaryRef theDict) {
    WindowGeometry geo;

    geo.x = CFDictionaryGetInt(theDict, CFSTR("X"));
    geo.y = CFDictionaryGetInt(theDict, CFSTR("Y"));
    geo.width = CFDictionaryGetInt(theDict, CFSTR("Width"));
    geo.height = CFDictionaryGetInt(theDict, CFSTR("Height"));

    return geo;
}

WindowGeometry AXWindowGetBounds(AXUIElementRef window) {
    AXValueRef attrValue;
    CGPoint windowPosition;
    CGSize windowSize;
    WindowGeometry geo;

    AXUIElementCopyAttributeValue(
        window, kAXPositionAttribute, (CFTypeRef *)&attrValue
    );
    AXValueGetValue(attrValue, kAXValueCGPointType, &windowPosition);
    CFRelease(attrValue);
    geo.x = windowPosition.x;
    geo.y = windowPosition.y;

    AXUIElementCopyAttributeValue(
        window, kAXSizeAttribute, (CFTypeRef *)&attrValue
    );
    AXValueGetValue(attrValue, kAXValueCGSizeType, &windowSize);
    CFRelease(attrValue);
    geo.width = windowSize.width;
    geo.height = windowSize.height;

    return geo;
}

void AXWindowSetBounds(AXUIElementRef window, WindowGeometry geo) {
    CGPoint windowPosition;
    AXValueRef attrValue;
    CGSize windowSize;

    windowPosition.x = geo.x;
    windowPosition.y = geo.y;
    attrValue = AXValueCreate(kAXValueCGPointType, &windowPosition);
    AXUIElementSetAttributeValue(window, kAXPositionAttribute, attrValue);
    CFRelease(attrValue);

    if(geo.width != -1 && geo.height != -1) {
        windowSize.width = geo.width;
        windowSize.height = geo.height;
        attrValue = AXValueCreate(kAXValueCGPointType, &windowSize);
        AXUIElementSetAttributeValue(window, kAXPositionAttribute, attrValue);
        CFRelease(attrValue);
    }
}

void moveWindow(CFDictionaryRef window, WindowGeometry newGeo) {
    WindowGeometry geo, appWindowGeo;
    pid_t pid;
    AXUIElementRef app, appWindow;
    CFArrayRef appWindowList;
    int i;
    CFStringRef windowTitle;

    /* If new coordinates and size match, then there is nothing to do */
    geo = CFDictionaryGetBounds(CFDictionaryGetValue(window, kCGWindowBounds));
    if(WGEqual(geo, newGeo)) return;

    /* Otherwise, load accessibility application from this PID */
    pid = CFDictionaryGetInt(window, kCGWindowOwnerPID);
    app = AXUIElementCreateApplication(pid);
    AXUIElementCopyAttributeValue(
        app, kAXWindowsAttribute, (CFTypeRef *)&appWindowList
    );

    /* Search application windows for first matching title, position, size */
    for(i = 0; i < CFArrayGetCount(appWindowList); i++) {
        appWindow = CFArrayGetValueAtIndex(appWindowList, i);
        AXUIElementCopyAttributeValue(
            appWindow, kAXTitleAttribute, (CFTypeRef *)&windowTitle
        );
        /* TODO: check that title matches */

        appWindowGeo = AXWindowGetBounds(appWindow);
        if(!WGEqual(geo, appWindowGeo)) continue;

        AXWindowSetBounds(appWindow, newGeo);

        break;
    }
}

int main(int argc, char **argv) {
    char *find_title, *app_name, *window_name, *title;
    WindowGeometry newGeo;
    int found, i, layer, titleSize;
    CFArrayRef windowList;
    CFDictionaryRef window;

#define WARN(msg) { fprintf(stderr, ME ": " msg "\n"); }
#define DIE(msg) { fprintf(stderr, ME ": " msg "\n"); exit(1); }
#define DIE_USAGE(msg) { fprintf(stderr, ME ": " msg "\n" USAGE); exit(1); }

    /* Parse and sanitize command line arguments */
    if(argc < 2) DIE_USAGE("missing required window title");
    if(argc < 4) DIE_USAGE("missing required window x and y coordinates");
    if(argc == 5) DIE_USAGE("height is required if width is present");
    find_title = argv[1];
    if(!find_title || !*find_title) DIE_USAGE("missing required title");
    newGeo.x = atoi(argv[2]);
    newGeo.y = atoi(argv[3]);
    if(argc > 5) {
        newGeo.width = atoi(argv[4]);
        newGeo.height = atoi(argv[5]);
        if(newGeo.width <= 0) DIE("width must be positive integer");
        if(newGeo.height <= 0) DIE("height must be positive integer");
    } else {
        newGeo.width = newGeo.height = -1;
    }
    if(argc > 6) WARN("ignoring extraneous arguments");

    /* Die if we are not authorized to use OS X accessibility */
    if(!isAuthorized()) DIE("not authorized to use accessibility API");

    windowList = CGWindowListCopyWindowInfo(
        (kCGWindowListOptionOnScreenOnly|kCGWindowListExcludeDesktopElements),
        kCGNullWindowID
    );

    found = 0;
    for(i = 0; i < CFArrayGetCount(windowList); i++) {
        window = CFArrayGetValueAtIndex(windowList, i);
        layer = CFDictionaryGetInt(window, kCGWindowLayer);
        if(layer > 0) continue;

        app_name = CFDictionaryCopyCString(window, kCGWindowOwnerName);
        window_name = CFDictionaryCopyCString(window, kCGWindowName);
        titleSize = strlen(app_name) + strlen(" - ") + strlen(window_name) + 1;
        title = (char *)malloc(titleSize);
        snprintf(title, titleSize, "%s - %s", app_name, window_name);

        if(fnmatch(find_title, title, 0) == 0) {
            moveWindow(window, newGeo);
            found = 1;
        }

        free(title);
        free(window_name);
        free(app_name);

        if(found) break;
    }
    if(!found) WARN("no window matching title");

    return found == 1 ? 0 : 1;

#undef DIE_USAGE
#undef DIE
#undef WARN
}

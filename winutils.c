#include "winutils.h"

/* Search windows for match (NULL for all), run function (NULL for none) */
int windowList(
    char *pattern,
    void(*callback)(CFDictionaryRef window, void *callback_data),
    void *callback_data
) {
    CFArrayRef windowList;
    int count, i, layer, titleSize;
    CFDictionaryRef window;
    char *appName, *windowName, *title;

    windowList = CGWindowListCopyWindowInfo(
        (kCGWindowListOptionOnScreenOnly|kCGWindowListExcludeDesktopElements),
        kCGNullWindowID
    );
    count = 0;
    for(i = 0; i < CFArrayGetCount(windowList); i++) {
        window = CFArrayGetValueAtIndex(windowList, i);
        layer = CFDictionaryGetInt(window, kCGWindowLayer);
        if(layer > 0) continue;

        appName = CFDictionaryCopyCString(window, kCGWindowOwnerName);
        windowName = CFDictionaryCopyCString(window, kCGWindowName);
        titleSize = strlen(appName) + strlen(" - ") + strlen(windowName) + 1;
        title = (char *)malloc(titleSize);
        snprintf(title, titleSize, "%s - %s", appName, windowName);

        if(!pattern || fnmatch(pattern, title, 0) == 0) {
            if(callback) (*callback)(window, callback_data);
            count++;
        }

        free(title);
        free(windowName);
        free(appName);
    }

    return count;
}

/* Fetch an integer value from a CFDictionary */
int CFDictionaryGetInt(CFDictionaryRef dict, const void *key) {
    int isSuccess, value;

    isSuccess = CFNumberGetValue(
        CFDictionaryGetValue(dict, key), kCFNumberIntType, &value
    );

    return isSuccess ? value : 0;
}

/* Copy a string value from a CFDictionary into a newly allocated string */
char *CFDictionaryCopyCString(CFDictionaryRef dict, const void *key) {
    const void *dictValue;
    CFIndex length;
    int maxSize, isSuccess;
    char *value;

    dictValue = CFDictionaryGetValue(dict, key);
    if(dictValue == (void *)NULL) return (char *)NULL;

    length = CFStringGetLength(dictValue);
    maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8);
    if(length == 0 || maxSize == 0) {
        value = (char *)malloc(1);
        *value = '\0';
        return value;
    }

    value = (char *)malloc(maxSize);
    isSuccess = CFStringGetCString(
        dictValue, value, maxSize, kCFStringEncodingUTF8
    );

    return isSuccess ? value : (char *)NULL;
}

/* Given window dictionary from CGWindowList, return position */
CGPoint CGWindowGetPosition(CFDictionaryRef window) {
    CFDictionaryRef bounds;
    int x, y;

    bounds = CFDictionaryGetValue(window, kCGWindowBounds);
    x = CFDictionaryGetInt(bounds, CFSTR("X"));
    y = CFDictionaryGetInt(bounds, CFSTR("Y"));

    return CGPointMake(x, y);
}

/* Given window dictionary from CGWindowList, return size */
CGSize CGWindowGetSize(CFDictionaryRef window) {
    CFDictionaryRef bounds;
    int width, height;

    bounds = CFDictionaryGetValue(window, kCGWindowBounds);
    width = CFDictionaryGetInt(bounds, CFSTR("Width"));
    height = CFDictionaryGetInt(bounds, CFSTR("Height"));

    return CGSizeMake(width, height);
}

/* Given window dictionary from CGWindowList, return accessibility object */
AXUIElementRef AXWindowFromCGWindow(CFDictionaryRef window) {
    return NULL;
}

/* Get position of window via accessibility object */
CGPoint AXWindowGetPosition(AXUIElementRef window) {
    CGPoint point;

    return point;
}

/* Set position of window via accessibility object */
void AXWindowSetPosition(AXUIElementRef window, CGPoint position) {
}

/* Get size of window via accessibility object */
CGSize AXWindowGetSize(AXUIElementRef window) {
    CGSize size;

    return size;
}

/* Set size of window via accessibility object */
void AXWindowSetSize(AXUIElementRef window, CGSize size) {
}

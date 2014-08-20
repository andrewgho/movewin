#include <Carbon/Carbon.h>
#include <fnmatch.h>

typedef struct {
    int x;
    int y;
    int width;
    int height;
} WindowGeometry;

int CFDictionaryGetInt(CFDictionaryRef theDict, const void *key) {
    int isSuccess, theInt;

    isSuccess = CFNumberGetValue(
        CFDictionaryGetValue(theDict, key),
        kCFNumberIntType,
        &theInt
    );

    return isSuccess ? theInt : 0;
}

char *CFDictionaryCopyCString(CFDictionaryRef theDict, const void *key) {
    const void *value;
    CFIndex length;
    int maxSize, isSuccess;
    char *theStringCopy;

    value = CFDictionaryGetValue(theDict, key);
    if(value == (void *)NULL) return (char *)NULL;

    length = CFStringGetLength(value);
    maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8);
    if(length == 0 || maxSize == 0) {
        theStringCopy = (char *)malloc(1);
        *theStringCopy = '\0';
        return theStringCopy;
    }

    theStringCopy = (char *)malloc(maxSize);
    isSuccess = CFStringGetCString(
        CFDictionaryGetValue(theDict, key),
        theStringCopy,
        maxSize,
        kCFStringEncodingUTF8
    );

    return isSuccess ? theStringCopy : (char *)NULL;
}

WindowGeometry CFDictionaryGetBounds(CFDictionaryRef theDict) {
    WindowGeometry theGeometry;

    theGeometry.x = CFDictionaryGetInt(theDict, CFSTR("X"));
    theGeometry.y = CFDictionaryGetInt(theDict, CFSTR("Y"));
    theGeometry.width = CFDictionaryGetInt(theDict, CFSTR("Width"));
    theGeometry.height = CFDictionaryGetInt(theDict, CFSTR("Height"));

    return theGeometry;
}

int main(int argc, char **argv) {
    char *find_title, *app_name, *window_name, *title;
    CFArrayRef windowList;
    int found, i, layer, titleSize;
    CFDictionaryRef window;
    WindowGeometry geometry;

    if(argc > 1 && *argv[1]) {
        find_title = argv[1];
    } else {
        find_title = (char *)NULL;
    }

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

        if(!find_title || fnmatch(find_title, title, 0) == 0) {
            geometry = CFDictionaryGetBounds(
                CFDictionaryGetValue(window, kCGWindowBounds)
            );
            printf(
                "%s - %d %d %d %d\n",
                title,
                geometry.x, geometry.y, geometry.width, geometry.height
            );
            found++;
        }

        free(title);
        free(window_name);
        free(app_name);
    }

    return found > 0 || !find_title ? 0 : 1;
}

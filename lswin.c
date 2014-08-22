#include "winutils.h"

void printWindow(CFDictionaryRef window, void *unused) {
    char *appName, *windowName;
    CGPoint position;
    CGSize size;

    appName = CFDictionaryCopyCString(window, kCGWindowOwnerName);
    windowName = CFDictionaryCopyCString(window, kCGWindowName);
    position = CGWindowGetPosition(window);
    size = CGWindowGetSize(window);

    printf(
        "%s - %s - %d %d %d %d\n",
        appName, windowName,
        (int)position.x, (int)position.y,
        (int)size.width, (int)size.height
    );

    free(windowName);
    free(appName);
}

int main(int argc, char **argv) {
    char *pattern;
    int count;

    if(argc > 1 && *argv[1]) {
        pattern = argv[1];
    } else {
        pattern = (char *)NULL;
    }
    count = windowList(pattern, printWindow, (void *)NULL);

    return count > 0 || !pattern ? 0 : 1;
}

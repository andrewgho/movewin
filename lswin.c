#include "winutils.h"

/* Callback for windowList() prints title of each window it encounters */
void PrintWindow(CFDictionaryRef window, void *unused) {
    char *appName = CFDictionaryCopyCString(window, kCGWindowOwnerName);
    char *windowName = CFDictionaryCopyCString(window, kCGWindowName);
    CGPoint position = CGWindowGetPosition(window);
    CGSize size = CGWindowGetSize(window);

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
    char *pattern = NULL;
    int count;

    if(argc > 1 && *argv[1]) pattern = argv[1];
    count = EnumerateWindows(pattern, PrintWindow, NULL);

    /* Return success if found any windows, or no windows but also no query */
    return count > 0 || !pattern ? 0 : 1;
}

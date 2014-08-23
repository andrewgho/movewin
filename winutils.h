#ifndef WINUTILS_H
#define WINUTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Carbon/Carbon.h>

/* Search windows for match (NULL for all), run function (NULL for none) */
int EnumerateWindows(
    char *pattern,
    void(*callback)(CFDictionaryRef window, void *callback_data),
    void *callback_data
);

/* Fetch an integer value from a CFDictionary */
int CFDictionaryGetInt(CFDictionaryRef dict, const void *key);

/* Copy a string value from a CFDictionary into a newly allocated string */
char *CFDictionaryCopyCString(CFDictionaryRef dict, const void *key);

/* Given window dictionary from CGWindowList, return position, size */
CGPoint CGWindowGetPosition(CFDictionaryRef window);
CGSize CGWindowGetSize(CFDictionaryRef window);

/* Given window dictionary from CGWindowList, return accessibility object */
AXUIElementRef AXWindowFromCGWindow(CFDictionaryRef window);

/* Get a value from an accessibility object */
void AXWindowGetValue(
    AXUIElementRef window,
    CFStringRef attrName,
    void *valuePtr
);

/* Get or set position of window via accessibility object */
CGPoint AXWindowGetPosition(AXUIElementRef window);
void AXWindowSetPosition(AXUIElementRef window, CGPoint position);

/* Get or set size of window via accessibility object */
CGSize AXWindowGetSize(AXUIElementRef window);
void AXWindowSetSize(AXUIElementRef window, CGSize size);

#ifdef __cplusplus
}
#endif

#endif  /* !WINUTILS_H */

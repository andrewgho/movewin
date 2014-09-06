/* ========================================================================
 * winutils.c - utility functions for listing and moving windows
 * Andrew Ho (andrew@zeuscat.com)
 *
 * Copyright (c) 2014, Andrew Ho.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * Neither the name of the author nor the names of its contributors may
 * be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ========================================================================
 */

#include <fnmatch.h>
#include "winutils.h"

/* Search windows for match (NULL for all), run function (NULL for none) */
int EnumerateWindows(
    char *pattern,
    void(*callback)(CFDictionaryRef window, void *callback_data),
    void *callback_data
) {
    int patternLen, subPatternLen, count, i, layer, titleSize;
    char *subPattern, *starL, *starR, *appName, *windowName, *title;
    CFArrayRef windowList;
    CFDictionaryRef window;

    /* Add asterisks to left/right of pattern, if they are not already there */
    if(pattern && *pattern) {
        patternLen = strlen(pattern);
        starL = (*pattern == '*') ? "" : "*";
        starR = (*pattern + (patternLen - 1) == '*') ? "" : "*";
        subPatternLen = patternLen + strlen(starL) + strlen(starR) + 1;
        subPattern = (char *)malloc(subPatternLen);
        snprintf(subPattern, subPatternLen, "%s%s%s", starL, pattern, starR);
    } else {
        subPattern = pattern;
    }

    /* Iterate through list of all windows, run callback on pattern matches */
    windowList = CGWindowListCopyWindowInfo(
        (kCGWindowListOptionOnScreenOnly|kCGWindowListExcludeDesktopElements),
        kCGNullWindowID
    );
    count = 0;
    for(i = 0; i < CFArrayGetCount(windowList); i++) {
        window = CFArrayGetValueAtIndex(windowList, i);

        /* Skip windows that are not on the desktop layer */
        layer = CFDictionaryGetInt(window, kCGWindowLayer);
        if(layer > 0) continue;

        /* Turn application name and title into string to match against */
        appName = windowName = title = NULL;
        appName = CFDictionaryCopyCString(window, kCGWindowOwnerName);
        if(!appName || !*appName) goto skip;
        windowName = CFDictionaryCopyCString(window, kCGWindowName);
        if(!windowName || !*windowName) goto skip;
        titleSize = strlen(appName) + strlen(" - ") + strlen(windowName) + 1;
        title = (char *)malloc(titleSize);
        snprintf(title, titleSize, "%s - %s", appName, windowName);

        /* If no pattern, or pattern matches, run callback */
        if(!pattern || fnmatch(subPattern, title, 0) == 0) {
            if(callback) (*callback)(window, callback_data);
            count++;
        }

      skip:
        if(title) free(title);
        if(windowName) free(windowName);
        if(appName) free(appName);
    }
    if(subPattern != pattern) free(subPattern);

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
    if(dictValue == NULL) return NULL;

    /* If empty value, allocate and return empty string */
    length = CFStringGetLength(dictValue);
    maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8);
    if(length == 0 || maxSize == 0) {
        value = (char *)malloc(1);
        *value = '\0';
        return value;
    }

    /* Otherwise, allocate string and copy value into it */
    value = (char *)malloc(maxSize);
    isSuccess = CFStringGetCString(
        dictValue, value, maxSize, kCFStringEncodingUTF8
    );

    return isSuccess ? value : NULL;
}

/* Given window dictionary from CGWindowList, return position */
CGPoint CGWindowGetPosition(CFDictionaryRef window) {
    CFDictionaryRef bounds = CFDictionaryGetValue(window, kCGWindowBounds);
    int x = CFDictionaryGetInt(bounds, CFSTR("X"));
    int y = CFDictionaryGetInt(bounds, CFSTR("Y"));
    return CGPointMake(x, y);
}

/* Given window dictionary from CGWindowList, return size */
CGSize CGWindowGetSize(CFDictionaryRef window) {
    CFDictionaryRef bounds = CFDictionaryGetValue(window, kCGWindowBounds);
    int width = CFDictionaryGetInt(bounds, CFSTR("Width"));
    int height = CFDictionaryGetInt(bounds, CFSTR("Height"));
    return CGSizeMake(width, height);
}

/* Return true if and only if we are authorized to call accessibility APIs */
bool isAuthorized() {
#if MAC_OS_X_VERSION_MIN_REQUIRED < 1090
    return AXAPIEnabled() || AXIsProcessTrusted();
#else
    /* Mavericks and later have only per-process accessibility permissions */
    return AXIsProcessTrusted();
#endif
}

/* Given window dictionary from CGWindowList, return accessibility object */
AXUIElementRef AXWindowFromCGWindow(CFDictionaryRef window, int minIdx) {
    CFStringRef targetWindowName, actualWindowTitle;
    CGPoint targetPosition, actualPosition;
    CGSize targetSize, actualSize;
    pid_t pid;
    AXUIElementRef app, appWindow, foundAppWindow;
    CFArrayRef appWindowList;
    int matchIdx, i;

    /* Save the window name, position, and size we are looking for */
    targetWindowName = CFDictionaryGetValue(window, kCGWindowName);
    targetPosition = CGWindowGetPosition(window);
    targetSize = CGWindowGetSize(window);

    /* Load accessibility application from window PID */
    pid = CFDictionaryGetInt(window, kCGWindowOwnerPID);
    app = AXUIElementCreateApplication(pid);
    AXUIElementCopyAttributeValue(
        app, kAXWindowsAttribute, (CFTypeRef *)&appWindowList
    );

    /* Search application windows for first matching title, position, size:
     * http://stackoverflow.com/questions/6178860/getting-window-number-through-osx-accessibility-api
     */
    matchIdx = 0;
    foundAppWindow = NULL;
    for(i = 0; i < CFArrayGetCount(appWindowList); i++) {
        appWindow = CFArrayGetValueAtIndex(appWindowList, i);

        /* Window name must match */
        AXUIElementCopyAttributeValue(
            appWindow, kAXTitleAttribute, (CFTypeRef *)&actualWindowTitle
        );
        if( !actualWindowTitle ||
            CFStringCompare(targetWindowName, actualWindowTitle, 0) != 0)
        {
            continue;
        }

        /* Position and size must match */
        actualPosition = AXWindowGetPosition(appWindow);
        if(!CGPointEqualToPoint(targetPosition, actualPosition)) continue;
        actualSize = AXWindowGetSize(appWindow);
        if(!CGSizeEqualToSize(targetSize, actualSize)) continue;

        /* Multiple windows may match, caller chooses which match to return */
        if(matchIdx >= minIdx) {
            /* Found the first matching window, save it and break */
            foundAppWindow = appWindow;
            break;
        } else {
            matchIdx++;
        }
    }

    return foundAppWindow;
}

/* Get a value from an accessibility object */
void AXWindowGetValue(
    AXUIElementRef window,
    CFStringRef attrName,
    void *valuePtr
) {
    AXValueRef attrValue;
    AXUIElementCopyAttributeValue(window, attrName, (CFTypeRef *)&attrValue);
    AXValueGetValue(attrValue, AXValueGetType(attrValue), valuePtr);
    CFRelease(attrValue);
}

/* Get position of window via accessibility object */
CGPoint AXWindowGetPosition(AXUIElementRef window) {
    CGPoint position;
    AXWindowGetValue(window, kAXPositionAttribute, &position);
    return position;
}

/* Set position of window via accessibility object */
void AXWindowSetPosition(AXUIElementRef window, CGPoint position) {
    AXValueRef attrValue = AXValueCreate(kAXValueCGPointType, &position);
    AXUIElementSetAttributeValue(window, kAXPositionAttribute, attrValue);
    CFRelease(attrValue);
}

/* Get size of window via accessibility object */
CGSize AXWindowGetSize(AXUIElementRef window) {
    CGSize size;
    AXWindowGetValue(window, kAXSizeAttribute, &size);
    return size;
}

/* Set size of window via accessibility object */
void AXWindowSetSize(AXUIElementRef window, CGSize size) {
    AXValueRef attrValue = AXValueCreate(kAXValueCGSizeType, &size);
    AXUIElementSetAttributeValue(window, kAXSizeAttribute, attrValue);
    CFRelease(attrValue);
}


/* ======================================================================== */

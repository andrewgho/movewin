/* ========================================================================
 * winutils.h - utility functions for listing and moving windows
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

/* Return true if and only if we are authorized to call accessibility APIs */
bool isAuthorized();

/* Given window dictionary from CGWindowList, return accessibility object */
AXUIElementRef AXWindowFromCGWindow(CFDictionaryRef window, int minIdx);

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


/* ======================================================================== */

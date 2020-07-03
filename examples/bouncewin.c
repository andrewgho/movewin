/* ========================================================================
 * bouncewin.c - bounce a window around the display
 * Andrew Ho (andrew@zeuscat.com)
 *
 * Copyright (c) 2014-2020, Andrew Ho.
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

#include "winutils.h"

static AXUIElementRef appWindow;

/* Callback for EnumerateWindows() records the first window it encounters */
void FindWindow(CFDictionaryRef window, void *ignored) {
    /* If we already found a window, skip all subsequent ones */
    if(appWindow) return;

    /* Get AXUIElementRef handle to window */
    appWindow = AXWindowFromCGWindow(window);
}

int main(int argc, char **argv) {
    useconds_t delay;
    char *pattern = NULL;
    CGSize size;
    CGRect displayBounds;
    int minX, minY, maxX, maxY, dX, dY;
    CGPoint position;

    /* Die if we are not authorized to use OS X accessibility */
    if(!isAuthorized()) {
        fputs("bouncewin: not authorized to use accessibility API\n", stderr);
        return 1;
    }

    /* Try to find a window */
    if(argc > 1 && *argv[1]) pattern = argv[1];
    appWindow = NULL;
    EnumerateWindows(pattern, FindWindow, (void *)appWindow);

    /* Return failure if we found no window */
    if(!appWindow) return 1;

    /* Get bounds of current display, calculate max bounds */
    displayBounds = CGDisplayBounds(CGMainDisplayID());
    size = AXWindowGetSize(appWindow);
    minX = 0;
    minY = 22;  /* TODO: this happens to be the height of my menubar */
    maxX = CGRectGetMaxX(displayBounds) - (size.width);
    maxY = CGRectGetMaxY(displayBounds) - (size.height);

    /* Move that window, move it move it */
    dX = dY = 1;
    delay = 1;
    while(1) {
        position = AXWindowGetPosition(appWindow);
        if(position.x <= minX || position.x >= maxX) dX = -dX;
        if(position.y <= minY || position.y >= maxY) dY = -dY;
        position.x += dX;
        position.y += dY;
        AXWindowSetPosition(appWindow, position);
        usleep(delay);
    }
}


/* ======================================================================== */

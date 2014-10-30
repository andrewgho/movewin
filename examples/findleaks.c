/* ========================================================================
 * findleaks.c - run winutils functions in a loop to help find memory leaks
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

#include "winutils.h"
#define ME "findleaks"

/* Callback for EnumerateWindows() that exercises various functions */
void TestWindow(CFDictionaryRef window, void *unused) {
    char *appName = CFDictionaryCopyCString(window, kCGWindowOwnerName);
    char *windowName = CFDictionaryCopyCString(window, kCGWindowName);
    CGPoint position = CGWindowGetPosition(window);
    CGSize size = CGWindowGetSize(window);
    size_t throwaway;
    AXUIElementRef appWindow = NULL;

    /* Reference values to silence -Wall "unused variable" warnings */
    throwaway = strlen(appName);
    throwaway = strlen(windowName);
    throwaway = (size_t)position.x;
    throwaway = (size_t)size.width;

    appWindow = AXWindowFromCGWindow(window);

    free(windowName);
    free(appName);
}

int main(int argc, char **argv) {
    int i, j;

#define DELAY_MICROSEC 1
#define RUNS 1024
#define TICKS 60
#define SHOW_EVERY ((int)((RUNS) / (TICKS)))
#define SHOW_MEMORY() \
system("/bin/ps auxw | grep '" ME "$' | grep -v grep | awk '{ print $5, $6 }'")

    SHOW_MEMORY();
    for(i = 0; i < RUNS; i++) {
        EnumerateWindows(NULL, TestWindow, NULL);
        if(i == 0) {
            SHOW_MEMORY();
            fprintf(stderr, "%d runs, %d runs/tick\n", RUNS, SHOW_EVERY);
            fputs("Progress: [", stderr);
            for(j = 0; j < RUNS; j++) {
                if(j % SHOW_EVERY == 0) fputc('-', stderr);
            }
            fputs("]\rProgress: [", stderr);
        }
        if(i % SHOW_EVERY == 0) fputc('*', stderr);
        usleep(DELAY_MICROSEC);
    }
    fputs("]\n", stderr);
    SHOW_MEMORY();

#undef SHOW_MEMORY
#undef SHOW_EVERY
#undef TICKS
#undef RUNS
#undef DELAY

    return 0;
}


/* ======================================================================== */

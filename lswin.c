/* ========================================================================
 * lswin.c - list windows
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

#define ME "lswin"
#define USAGE "usage: " ME " [-h] [-l] [-i id] [title]\n"
#define FULL_USAGE USAGE \
    "    -h       display this help text and exit\n" \
    "    -l       long display, include window ID column in output\n" \
    "    -i id    show only windows with this window ID (-1 for all)\n" \
    "    title    pattern to match \"Application - Title\" against\n"

typedef struct {
    int longDisplay;   /* include window ID column in output */
    int id;            /* show only windows with this window ID (-1 for all) */
    int numFound;      /* out parameter, number of windows found */
} LsWinCtx;

/* Callback for EnumerateWindows() prints title of each window it encounters */
void PrintWindow(CFDictionaryRef window, void *ctxPtr) {
    LsWinCtx *ctx = (LsWinCtx *)ctxPtr;
    int windowId = CFDictionaryGetInt(window, kCGWindowNumber);
    char *appName = CFDictionaryCopyCString(window, kCGWindowOwnerName);
    char *windowName = CFDictionaryCopyCString(window, kCGWindowName);
    char *title = windowTitle(appName, windowName);
    CGPoint position = CGWindowGetPosition(window);
    CGSize size = CGWindowGetSize(window);

    if(ctx->id == -1 || ctx->id == windowId) {
        if(ctx->longDisplay) printf("%d - ", windowId);
        printf(
            "%s - %d %d %d %d\n", title,
            (int)position.x, (int)position.y,
            (int)size.width, (int)size.height
        );
        ctx->numFound++;
    }
    free(title);
    free(windowName);
    free(appName);
}

int main(int argc, char **argv) {
    LsWinCtx ctx;
    int ch;
    char *pattern = NULL;

#define DIE_OPT(msg) \
    { fprintf(stderr, ME ": " msg " -- %c\n" USAGE, optopt); return 1; }

    /* Parse and sanitize command line arguments */
    ctx.longDisplay = 0;
    ctx.id = -1;
    ctx.numFound = 0;
    while((ch = getopt(argc, argv, ":hli:")) != -1) {
        switch(ch) {
            case 'h':
                 printf(FULL_USAGE);
                 return 0;
            case 'l':
                ctx.longDisplay = 1;
                break;
            case 'i':
                ctx.id = atoi(optarg);
                break;
            case ':':
                DIE_OPT("option requires an argument");
            default:
                DIE_OPT("illegal option");
         }
    }
    argc -= optind;
    argv += optind;
    if(argc > 0) pattern = argv[0];

    /* Print matching windows */
    EnumerateWindows(pattern, PrintWindow, (void *)&ctx);

    /* Return success if found any windows, or no windows but also no query */
    return (ctx.numFound > 0 || (pattern == NULL && ctx.id == -1)) ? 0 : 1;

#undef DIE_OPT
}


/* ======================================================================== */

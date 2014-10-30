/* ========================================================================
 * movewin.c - move windows
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

#define ME "movewin"
#define USAGE \
"usage: " ME " [-h] [-n] [-i idx] title x y [width height]\n"
#define FULL_USAGE USAGE \
"    -h            display this help text and exit\n" \
"    -n            negative x y is off screen (default from bottom right)\n" \
"    -i idx        for duplicate titles, move window at idx (default 0)\n" \
"    title         pattern to match \"Application - Title\" against\n" \
"    x y           required, position to move window to\n" \
"    width height  optional, new size to resize window to\n"

/* Hold target position, optional size, mutex so we only move first window */
typedef struct {
    int skipWindows;     /* for duplicate titles, skip this many windows */
    int fromRight;       /* x coordinate is offset from right, not left */
    int fromBottom;      /* y coordinate is offset from bottom, not top */
    CGPoint position;    /* move window to this position */
    CGSize size;         /* resize window to this size */
    int hasSize;         /* only resize if this is true */
    int movedWindow;     /* set to true if we have moved any window */
} MoveWinCtx;

/* Return true if string starts with minus sign */
static bool startsWithMinus(char *s) {
    char *p = s;
    while(*p && isspace((unsigned char)*p)) p++;  /* skip leading whitespace */
    return *p == '-';
}

/* Callback for EnumerateWindows() moves the first window it encounters */
void MoveWindow(CFDictionaryRef window, void *ctxPtr) {
    MoveWinCtx *ctx = (MoveWinCtx *)ctxPtr;
    CGPoint newPosition, actualPosition;
    CGSize newSize, actualSize;
    CGRect displayBounds;
    AXUIElementRef appWindow = NULL;
    int minIdx;

    /* If we already moved a window, skip all subsequent ones */
    if(ctx->movedWindow) return;

    /* If we are supposed to skip some windows, skip this one */
    if(ctx->skipWindows > 0) {
        ctx->skipWindows--;
        return;
    }

    /* Recalculate target window position if we got negative values */
    newPosition = ctx->position;
    actualSize = CGWindowGetSize(window);
    newSize = ctx->hasSize ? ctx->size : actualSize;
    if(ctx->fromRight || ctx->fromBottom) {
        displayBounds = CGDisplayBounds(CGMainDisplayID());
        if(ctx->fromRight) {
            newPosition.x = CGRectGetMaxX(displayBounds) -
                (newSize.width + abs(newPosition.x));
        }
        if(ctx->fromBottom) {
            newPosition.y = CGRectGetMaxY(displayBounds) -
                (newSize.height + abs(newPosition.y));
        }
    }

    /* Move window, unless positions already match */
    actualPosition = CGWindowGetPosition(window);
    if(!CGPointEqualToPoint(newPosition, actualPosition)) {
        if(!appWindow) appWindow = AXWindowFromCGWindow(window, minIdx = 0);
        AXWindowSetPosition(appWindow, newPosition);
    }

    /* If size was specified, resize window, unless sizes already match */
    if(ctx->hasSize) {
        if(!CGSizeEqualToSize(newSize, actualSize)) {
            if(!appWindow) appWindow = AXWindowFromCGWindow(window, minIdx = 0);
            AXWindowSetSize(appWindow, newSize);
        }
    }

    /* Record that we moved a window, so we will skip all subsequent ones */
    ctx->movedWindow = 1;
}

int main(int argc, char **argv) {
    int ch, negativeOffScreen = 0;
    char *pattern;
    MoveWinCtx ctx;

#define WARN(msg) { fprintf(stderr, ME ": " msg "\n"); }
#define DIE(msg) { fprintf(stderr, ME ": " msg "\n"); exit(1); }
#define DIE_USAGE(msg) { fprintf(stderr, ME ": " msg "\n" USAGE); exit(1); }
#define DIE_OPT(msg) \
    { fprintf(stderr, ME ": " msg " -- %c\n" USAGE, optopt); return 1; }

    /* Parse and sanitize command line arguments */
    ctx.skipWindows = 0;
    while((ch = getopt(argc, argv, ":hni:")) != -1) {
        switch(ch) {
            case 'h':
                 printf(FULL_USAGE);
                 return 0;
            case 'n':
                negativeOffScreen = 1;
                break;
            case 'i':
                ctx.skipWindows = atoi(optarg);
                break;
            case ':':
                DIE_OPT("option requires an argument");
            default:
                DIE_OPT("illegal option");
         }
    }
    argc -= optind;
    argv += optind;
    if(argc < 1) DIE_USAGE("missing required window title");
    if(argc < 3) DIE_USAGE("missing required window x and y coordinates");
    if(argc == 4) DIE_USAGE("height is required if width is present");
    pattern = argv[0];
    if(!pattern || !*pattern) DIE_USAGE("missing required title");
    ctx.position.x = atoi(argv[1]);
    ctx.position.y = atoi(argv[2]);
    if(negativeOffScreen) {
        ctx.fromRight = ctx.fromBottom = 0;
    } else {
        ctx.fromRight = startsWithMinus(argv[1]);
        ctx.fromBottom = startsWithMinus(argv[2]);
        ctx.position.x = abs(ctx.position.x);
        ctx.position.y = abs(ctx.position.y);
    }
    if(argc > 4) {
        ctx.size.width = atoi(argv[3]);
        ctx.size.height = atoi(argv[4]);
        if(ctx.size.width <= 0) DIE("width must be positive integer");
        if(ctx.size.height <= 0) DIE("height must be positive integer");
        ctx.hasSize = 1;
    } else {
        ctx.size.width = ctx.size.height = 0;
        ctx.hasSize = 0;
    }
    if(argc > 5) WARN("ignoring extraneous arguments");

    /* Die if we are not authorized to use OS X accessibility */
    if(!isAuthorized()) DIE("not authorized to use accessibility API");

    /* Try to move a window */
    ctx.movedWindow = 0;
    EnumerateWindows(pattern, MoveWindow, (void *)&ctx);

    /* Return success if we moved any window, failure otherwise */
    return ctx.movedWindow ? 0 : 1;

#undef DIE_OPT
#undef DIE_USAGE
#undef DIE
#undef WARN
}


/* ======================================================================== */

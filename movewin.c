/* ========================================================================
 * movewin.c - move windows
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

#define ME "movewin"
#define USAGE \
"usage: " ME " [-h] [-n] [-i id | title] x y [width height]\n"
#define FULL_USAGE USAGE \
"    -h            display this help text and exit\n" \
"    -n            negative x y is off screen (default from bottom right)\n" \
"    -i id         window ID to move (one of title or ID is required)\n" \
"    title         pattern to match \"Application - Title\" against\n" \
"    x y           required, position to move window to\n" \
"    width height  optional, new size to resize window to\n"

/* Undocumented accessibility API to get window ID, see winutils.c */
extern AXError _AXUIElementGetWindow(AXUIElementRef, CGWindowID *out);

/* Hold target position, optional size, mutex so we only move first window */
typedef struct {
    int id;              /* window ID to search for */
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
    int windowId = CFDictionaryGetInt(window, kCGWindowNumber);
    CGPoint newPosition, actualPosition;
    CGSize newSize, actualSize;
    CGRect displayBounds;
    AXUIElementRef appWindow = NULL;

    /* If we already moved a window, skip all subsequent ones */
    if(ctx->movedWindow) return;

    /* If a windowId was specified, and this isn't that window, skip it */
    if(ctx->id != -1 && ctx->id != windowId) return;

    /* Recalculate target window position if we got negative values */
    newPosition = ctx->position;
    actualSize = CGWindowGetSize(window);
    newSize = ctx->hasSize ? ctx->size : actualSize;
    if(ctx->fromRight || ctx->fromBottom) {
        displayBounds = CGDisplayBounds(CGMainDisplayID());
        if(ctx->fromRight) {
            newPosition.x = CGRectGetMaxX(displayBounds) -
                (newSize.width + fabs(newPosition.x));
        }
        if(ctx->fromBottom) {
            newPosition.y = CGRectGetMaxY(displayBounds) -
                (newSize.height + fabs(newPosition.y));
        }
    }

    /* Move window, unless positions already match */
    actualPosition = CGWindowGetPosition(window);
    if(!CGPointEqualToPoint(newPosition, actualPosition)) {
        if(!appWindow) appWindow = AXWindowFromCGWindow(window);
        AXWindowSetPosition(appWindow, newPosition);
    }

    /* If size was specified, resize window, unless sizes already match */
    if(ctx->hasSize) {
        if(!CGSizeEqualToSize(newSize, actualSize)) {
            if(!appWindow) appWindow = AXWindowFromCGWindow(window);
            AXWindowSetSize(appWindow, newSize);
        }
    }

    /* Record that we moved a window, so we will skip all subsequent ones */
    ctx->movedWindow = 1;
}

/* Silence warning that address of _AXUIElementGetWindow is always true */
#pragma GCC diagnostic ignored "-Waddress"

int main(int argc, char **argv) {
    MoveWinCtx ctx;
    int ch, negativeOffScreen = 0;
    char *pattern = NULL;

#define WARN(msg) { fprintf(stderr, ME ": " msg "\n"); }
#define DIE(msg) { fprintf(stderr, ME ": " msg "\n"); exit(1); }
#define DIE_USAGE(msg) { fprintf(stderr, ME ": " msg "\n" USAGE); exit(1); }
#define DIE_OPT(msg) \
    { fprintf(stderr, ME ": " msg " -- %c\n" USAGE, optopt); return 1; }

    /* Parse and sanitize command line arguments */
    ctx.id = -1;
    while((ch = getopt(argc, argv, ":hni:")) != -1) {
        switch(ch) {
            case 'h':
                 printf(FULL_USAGE);
                 return 0;
            case 'n':
                negativeOffScreen = 1;
                break;
            case 'i':
                if(!_AXUIElementGetWindow) {
                    DIE("unable to use window IDs for reference");
                }
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
    if(ctx.id == -1) {
        if(argc < 1) DIE_USAGE("missing required window title");
        pattern = argv[0];
        if(!pattern || !*pattern) DIE_USAGE("missing required window title");
        argc--;
        argv++;
    }
    if(argc < 2) DIE_USAGE("missing required window x and y coordinates");
    ctx.position.x = atoi(argv[0]);
    ctx.position.y = atoi(argv[1]);
    if(negativeOffScreen) {
        ctx.fromRight = ctx.fromBottom = 0;
    } else {
        ctx.fromRight = startsWithMinus(argv[0]);
        ctx.fromBottom = startsWithMinus(argv[1]);
        ctx.position.x = fabs(ctx.position.x);
        ctx.position.y = fabs(ctx.position.y);
    }
    argc -= 2;
    argv += 2;
    if(argc == 1) DIE_USAGE("height is required if width is present");
    if(argc > 1) {
        ctx.size.width = atoi(argv[0]);
        ctx.size.height = atoi(argv[1]);
        if(ctx.size.width <= 0) DIE("width must be positive integer");
        if(ctx.size.height <= 0) DIE("height must be positive integer");
        ctx.hasSize = 1;
    } else {
        ctx.size.width = ctx.size.height = 0;
        ctx.hasSize = 0;
    }
    argc -= 2;
    argv += 2;
    if(argc > 0) WARN("ignoring extraneous arguments");

    /* Die if we are not authorized to do screen recording */
    if(!isAuthorizedForScreenRecording()) DIE("not authorized to do screen recording");

    /* Die if we are not authorized to use OS X accessibility */
    if(!isAuthorizedForAccessibility()) DIE("not authorized to use accessibility API");

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

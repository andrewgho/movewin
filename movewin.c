#include "winutils.h"

#define ME "movewin"
#define USAGE "usage: " ME " title x y [width height]\n"

/* Hold target position, optional size, mutex so we only move first window */
typedef struct {
    int fromRight;       /* x coordinate is offset from right, not left */
    int fromBottom;      /* y coordinate is offset from bottom, not top */
    CGPoint position;    /* move window to this position */
    CGSize size;         /* resize window to this size */
    int hasSize;         /* only resize if this is true */
    int movedWindow;     /* set to true if we have moved any window */
} MoveWinCtx;

/* Return true if string starts with two minus signs, and remove them */
static bool startsWithDoubleMinus(char *s) {
    char *p = s;
    while(*p && isspace((unsigned char)*p)) p++;  /* skip leading whitespace */
    if(*p == '-' && *(p + 1) == '-') {
        *p = *(p + 1) = ' ';
        return 1;
    } else {
        return 0;
    }
}

/* Return true if and only if we are authorized to call accessibility APIs */
static bool isAuthorized() {
#if MAC_OS_X_VERSION_MIN_REQUIRED < 1090
    return AXAPIEnabled() || AXIsProcessTrusted();
#else
    /* Mavericks and later have only per-process accessibility permissions */
    return AXIsProcessTrusted();
#endif
}

/* Callback for EnumerateWindows() moves the first window it encounters */
void MoveWindow(CFDictionaryRef window, void *ctxPtr) {
    MoveWinCtx *ctx = (MoveWinCtx *)ctxPtr;
    CGPoint newPosition, actualPosition;
    CGSize actualSize;
    CGRect displayBounds;
    AXUIElementRef appWindow = NULL;
    int minIdx;

    /* If we already moved a window, skip all subsequent ones */
    if(ctx->movedWindow) return;

    /* Recalculate target window position if we got negative values */
    newPosition = ctx->position;
    actualSize = CGWindowGetSize(window);
    if(ctx->fromRight || ctx->fromBottom) {
        displayBounds = CGDisplayBounds(CGMainDisplayID());
        if(ctx->fromRight) {
            newPosition.x = CGRectGetMaxX(displayBounds) -
                (actualSize.width + abs(ctx->position.x));
        }
        if(ctx->fromBottom) {
            newPosition.y = CGRectGetMaxY(displayBounds) -
                (actualSize.height + abs(ctx->position.y));
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
        if(!CGSizeEqualToSize(ctx->size, actualSize)) {
            if(!appWindow) appWindow = AXWindowFromCGWindow(window, minIdx = 0);
            AXWindowSetSize(appWindow, ctx->size);
        }
    }

    /* Record that we moved a window, so we will skip all subsequent ones */
    ctx->movedWindow = 1;
}

int main(int argc, char **argv) {
    char *pattern;
    MoveWinCtx ctx;

#define WARN(msg) { fprintf(stderr, ME ": " msg "\n"); }
#define DIE(msg) { fprintf(stderr, ME ": " msg "\n"); exit(1); }
#define DIE_USAGE(msg) { fprintf(stderr, ME ": " msg "\n" USAGE); exit(1); }

    /* Parse and sanitize command line arguments */
    if(argc < 2) DIE_USAGE("missing required window title");
    if(argc < 4) DIE_USAGE("missing required window x and y coordinates");
    if(argc == 5) DIE_USAGE("height is required if width is present");
    pattern = argv[1];
    if(!pattern || !*pattern) DIE_USAGE("missing required title");
    ctx.fromRight = startsWithDoubleMinus(argv[2]);
    ctx.fromBottom = startsWithDoubleMinus(argv[3]);
    ctx.position.x = atoi(argv[2]);
    ctx.position.y = atoi(argv[3]);
    if(argc > 5) {
        ctx.size.width = atoi(argv[4]);
        ctx.size.height = atoi(argv[5]);
        if(ctx.size.width <= 0) DIE("width must be positive integer");
        if(ctx.size.height <= 0) DIE("height must be positive integer");
        ctx.hasSize = 1;
    } else {
        ctx.size.width = ctx.size.height = 0;
        ctx.hasSize = 0;
    }
    if(argc > 6) WARN("ignoring extraneous arguments");

    /* Die if we are not authorized to use OS X accessibility */
    if(!isAuthorized()) DIE("not authorized to use accessibility API");

    /* Try to move a window */
    ctx.movedWindow = 0;
    EnumerateWindows(pattern, MoveWindow, (void *)&ctx);

    /* Return success if we moved any window, failure otherwise */
    return ctx.movedWindow ? 0 : 1;

#undef DIE_USAGE
#undef DIE
#undef WARN
}

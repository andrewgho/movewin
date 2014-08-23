#include "winutils.h"

#define ME "movewin"
#define USAGE "usage: " ME " title x y [width height]\n"

/* Hold target position, optional size, mutex so we only move first window */
typedef struct {
    CGPoint position;    /* move window to this position */
    CGSize size;         /* resize window to this size */
    int hasSize;         /* only resize if this is true */
    int movedWindow;     /* set to true if we have moved any window */
} MoveWinCtx;

/* Return true if and only if we are authorized to call accessibility APIs */
static bool isAuthorized() {
    /* TODO: silence deprecation warning in Mavericks and later */
    return AXAPIEnabled() || AXIsProcessTrusted();
}

/* Callback for windowList() moves the first window it encounters */
void moveWindow(CFDictionaryRef window, void *ctxPtr) {
    MoveWinCtx *ctx = (MoveWinCtx *)ctxPtr;
    AXUIElementRef appWindow = NULL;
    CGPoint actualPosition;
    CGPoint actualSize;

    /* If we already moved a window, skip all subsequent ones */
    if(ctx->movedWindow) return;

    /* Move window, unless positions already match */
    actualPosition = CGWindowGetPosition(window);
    if(!CGPointEqualToPoint(ctx->position, CGWindowGetPosition(window))) {
        appWindow = AXWindowFromCGWindow(window);
        AXWindowSetPosition(appWindow, ctx->position);
    }

    /* If size was specified, resize window, unless sizes already match */
    if(ctx->hasSize) {
        actualSize = CGWindowGetPosition(window);
        if(!CGSizeEqualToSize(ctx->size, CGWindowGetSize(window))) {
            if(!appWindow) appWindow = AXWindowFromCGWindow(window);
            AXWindowSetPosition(appWindow, ctx->position);
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
    windowList(pattern, moveWindow, (void *)&ctx);

    /* Return success if we moved any window, failure otherwise */
    return ctx.movedWindow ? 0 : 1;

#undef DIE_USAGE
#undef DIE
#undef WARN
}

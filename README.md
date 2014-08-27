movewin - move windows from OS X command line
=============================================

This repository is the source code for the `lswin` and `movewin`
programs, which are OS X command line programs to list and move desktop
windows, respectively.

Getting Started
---------------

To build from source:

    $ make

To list open windows, their locations, and sizes:

    $ lswin

To move a window from to the upper left corner:

    $ movewin 'iTerm - Default' 0 0

Moving windows requires accessibility access to be enabled.

Description
-----------

These OS X command line programs list and move desktop windows. Listing
windows is done using Quartz window functions, and moving them is
accomplished using the OS X accessibility APIs.

### Listing Windows

The `lswin` program lists each visible window on the current desktop
of the current display. Each line of output includes the application
name (like `iTerm` or `Firefox`), the window title (like `Default` or
`Google`), and the position and size of each window. For example:

    $ lswin
    iTerm - Default - 0 22 745 458
    Firefox - Google - 216 22 1224 874

In the above example, there are two windows open in the current desktop
of the current display; one an 80x24 iTerm terminal at the upper left,
the other a Firefox window at the upper right, spanning the entirety of
a 2010-era MacBook Air.

`lswin` takes a single optional command line argument, which is a
pattern, treated like a filename glob, that is matched (unanchored)
against the "Application Name - Window Title" string. For example:

    $ lswin iTerm
    iTerm - Default - 0 22 745 458

    $ lswin 'G*le'
    Firefox - Google - 216 22 1224 874

### Moving Windows

The `movewin` program moves windows. It takes a required pattern, which
is treated the same way as patterns are in `lswin`, required new X and Y
coordinates to move the window to, and an optional new width and height
to resize the window to. For example, this moves iTerm to near the
middle of the screen, and resizes it:

    $ movewin 'iTerm - Default' 340 240 745 458

Coordinates may be negative, so this snaps a window to the upper right:

    $ movewin 'Firefox - GitHub' -0 0

In case you actually do want to move a window off screen, the `-n`
command line option tells `movewin` to treat negative coordinates as
absolute coordinates, rather than being relative to the right or bottom
edge of the display. This moves a terminal window so its left edge is
off the display:

    $ movewin -n iTerm -100 240

If multiple windows have the same title, you can use the `-i` (index)
option to select which window to move. The index starts at zero.

### Enabling Accessibility Access

The `movewin` program requires the "Enable access for assistive devices"
setting to be enabled in the "Accessibility" System Preferences pane in
OS X pre-Mavericks. To enable assistive UI scripting in Mavericks, see
this Apple KB article:
[http://support.apple.com/kb/HT5914](http://support.apple.com/kb/HT5914)

Author
------

Andrew Ho (<andrew@zeuscat.com>)

License
-------

    Copyright (c) 2014, Andrew Ho.
    All rights reserved.
    
    Redistribution and use in source and binary forms, with or without 
    modification, are permitted provided that the following conditions
    are met:
    
    Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
    
    Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    
    Neither the name of the author nor the names of its contributors may
    be used to endorse or promote products derived from this software 
    without specific prior written permission.
    
    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

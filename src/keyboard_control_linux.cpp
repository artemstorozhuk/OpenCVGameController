//implementation of keyboard events for linux
//windows should have another implementation
#include "keyboard_control.h"

#include <X11/Xlib.h>
#include <X11/keysym.h>

//Some global X variables
static Window rootWindow;
static Display *display;

static bool upPressed = false;
static bool leftPressed = false;
static bool rightPressed = false;

bool initDisplay()
{
    display = XOpenDisplay(0);

    if (display != NULL)
    {
        // Get the root window for the current display.
        rootWindow = XDefaultRootWindow(display);

        return true;
    }
    else
    {
        return false;
    }
}


void closeDisplay()
{
    XCloseDisplay(display);
}

// Function to create a keyboard event
XKeyEvent createKeyEvent(Display *display, Window &win,
                           Window &winRoot, bool press,
                           int keycode, int modifiers)
{
    XKeyEvent event;

    event.display     = display;
    event.window      = win;
    event.root        = winRoot;
    event.subwindow   = None;
    event.time        = CurrentTime;
    event.x           = 1;
    event.y           = 1;
    event.x_root      = 1;
    event.y_root      = 1;
    event.same_screen = True;
    event.keycode     = XKeysymToKeycode(display, keycode);
    event.state       = modifiers;

    if(press)
    {
        event.type = KeyPress;
    }
    else
    {
        event.type = KeyRelease;
    }

    return event;
}


void simulateKeyPress(int key, bool keyType)
{
    // Find the window which has the current keyboard focus.
    Window winFocus;
    int    revert;
    XGetInputFocus(display, &winFocus, &revert);

    // Send a fake key press event to the window.
    XKeyEvent event = createKeyEvent(display, winFocus, rootWindow, keyType, key, 0);
    XSendEvent(event.display, event.window, True, KeyPressMask, (XEvent *)&event);
}

void releaseLeftKey()
{
    if (leftPressed)
    {
        leftPressed = false;
        simulateKeyPress(XK_Left, false);
    }
}


void releaseRightKey()
{
    if (rightPressed)
    {
        rightPressed = false;
        simulateKeyPress(XK_Right, false);
    }
}


void releaseUpKey()
{
    if (upPressed)
    {
        upPressed = false;
        simulateKeyPress(XK_Up, false);
    }
}


void releaseAllKeysKey()
{
    releaseLeftKey();
    releaseRightKey();
    releaseUpKey();
}


void pressRightKey()
{
    rightPressed = true;
    simulateKeyPress(XK_Right, true);
}


void pressLeftKey()
{
    leftPressed = true;
    simulateKeyPress(XK_Left, true);
}


void pressUpKey()
{
    upPressed = true;
    simulateKeyPress(XK_Up, true);
}


#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "platform.h"
#include "viewport.h"

Display* display;
Window window;
GC gc;
Atom atom_wmdw;

void FillRectangle(float left, float top, float right, float bottom)
{
	XFillRectangle(display, window, gc, client_x(left), client_y(top), client_x(right) - client_x(left), client_y(bottom) - client_y(top));
}

void DrawRectangle(float left, float top, float right, float bottom)
{
	XDrawRectangle(display, window, gc, client_x(left), client_y(top), client_x(right) - client_x(left), client_y(bottom) - client_y(top));
}

void DrawCircle(float center_x, float center_y, float radius)
{
	XDrawArc(display, window, gc, client_x(center_x - radius), client_y(center_y - radius), client_size(radius * 2), client_size(radius * 2), 0, 360 * 64);
}

void DrawLine(float x1, float y1, float x2, float y2)
{
	XDrawLine(display, window, gc, client_x(x1), client_y(y1), client_x(x2), client_y(y2));
}

void create_window(int width, int height, const char* title)
{
	XSetWindowAttributes winatt;
	XSizeHints wmsize;
	XWMHints wmhints;
	XTextProperty windowName;

	display = XOpenDisplay(NULL);
	int screen = DefaultScreen(display);
	int display_width = DisplayWidth(display, screen);
	int display_height = DisplayHeight(display, screen);
	int x = (display_width - width) / 2;
	int y = (display_height - height) / 2;

	winatt.background_pixel = 0x333333UL;
	winatt.event_mask = ButtonPressMask | ButtonReleaseMask | PointerMotionMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask | ExposureMask;
	unsigned long valuemask = CWBackPixel | CWEventMask;
	window = XCreateWindow(display, RootWindow(display, screen), x, y, width, height, 0, DefaultDepth(display, screen), InputOutput, DefaultVisual(display, screen), valuemask, &winatt);

	atom_wmdw = XInternAtom(display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(display, window, &atom_wmdw, 1);

	wmsize.flags = USPosition | USSize;
	XSetWMNormalHints(display, window, &wmsize);
	wmhints.initial_state = NormalState;
	wmhints.flags = StateHint;
	XSetWMHints(display, window, &wmhints);
	XStringListToTextProperty((char**)&title, 1, &windowName);
	XSetWMName(display, window, &windowName);
	XMapWindow(display, window);
	XFlush(display);
	gc = DefaultGC(display, screen);
}

int update_window()
{
	// KeySym ks;
	XEvent event;
	XNextEvent(display, &event);

	if (event.type == Expose)
	{
		XSetForeground(display, gc, 0xFFFFFF);
		FillRectangle(0.0, 0.0, canvas_width(), canvas_height());
		XSetForeground(display, gc, 0);
		DrawRectangle(2.0, 2.0, 3.0, 3.0);
		DrawCircle(2.5, 2.5, 0.25);
		DrawLine(1.0, 1.0, 9.5, 1.0);
		return 1;
	}
	else if (event.type == ConfigureNotify)
	{
		set_viewport_window(event.xconfigure.width, event.xconfigure.height);
		set_canvas_size(10.5, 8.0);

		return 1;
	}
	else if (event.type == ClientMessage)
	{
		if (event.xclient.data.l[0] == (long)atom_wmdw)
			return 0;
	}

	// switch (event.type)
	// {
	// case ButtonPress:
	// 	if (event.xbutton.button < 4)
	// 		handleMousePress(event.xbutton.button, event.xbutton.x, event.xbutton.y, event.xbutton.state);
	// 	else if (event.xbutton.button < 6)
	// 		handleMouseWheel(0, 9 - (event.xbutton.button * 2), event.xbutton.x, event.xbutton.y, event.xbutton.state);
	// 	else
	// 		handleMouseWheel(13 - (event.xbutton.button * 2), 0, event.xbutton.x, event.xbutton.y, event.xbutton.state);
	// 	break;

	// case ButtonRelease:
	// 	if (event.xbutton.button < 4)
	// 		handleMouseRelease(event.xbutton.button, event.xbutton.x, event.xbutton.y, event.xbutton.state);
	// 	break;

	// case MotionNotify:
	// 	handleMouseMove(event.xmotion.x, event.xmotion.y, event.xmotion.state);
	// 	break;

	// case KeyPress:
	// 	XLookupString(&event.xkey, NULL, 0, &ks, NULL);
	// 	handleKeyPress(ks, event.xkey.state);
	// 	break;

	// case KeyRelease:
	// 	XLookupString(&event.xkey, NULL, 0, &ks, NULL);
	// 	handleKeyRelease(ks, event.xkey.state);
	// 	break;

	return 1;
}

int main()
{
	create_window(800, 600, "Spice GUI");

	while (update_window())
	{
	}

	XCloseDisplay(display);
	return 0;
}

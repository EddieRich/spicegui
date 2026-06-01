#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "platform.h"
#include "viewport.h"
#include "schematic.h"

static Display* display;
static Window window;
static GC gc;
static Atom atom_wmdw;
static PenThickness last_pt = PT_THIN;

void FillRectangle(float left, float top, float right, float bottom)
{
	XFillRectangle(display, window, gc, client_x(left), client_y(top), client_x(right) - client_x(left), client_y(bottom) - client_y(top));
}

void DrawRectangle(float left, float top, float right, float bottom, PenThickness pt)
{
	if (pt != last_pt)
	{
		last_pt = pt;
		XSetLineAttributes(display, gc, pt * 2 + 1, LineSolid, CapRound, JoinRound);
	}

	XDrawRectangle(display, window, gc, client_x(left), client_y(top), client_x(right) - client_x(left), client_y(bottom) - client_y(top));
}

void DrawCircle(float center_x, float center_y, float radius, PenThickness pt)
{
	if (pt != last_pt)
	{
		last_pt = pt;
		XSetLineAttributes(display, gc, pt * 2 + 1, LineSolid, CapRound, JoinRound);
	}

	XDrawArc(display, window, gc, client_x(center_x - radius), client_y(center_y - radius), client_size(radius * 2), client_size(radius * 2), 0, 360 * 64);
}

void DrawLine(float x1, float y1, float x2, float y2, PenThickness pt)
{
	if (pt != last_pt)
	{
		last_pt = pt;
		XSetLineAttributes(display, gc, pt * 2 + 1, LineSolid, CapRound, JoinRound);
	}

	XDrawLine(display, window, gc, client_x(x1), client_y(y1), client_x(x2), client_y(y2));
}

void DrawArc(float center_x, float center_y, float a, float b, float start_angle, float end_angle, PenThickness pt)
{
	int x = client_x(center_x - a / 2.0f);
	int y = client_y(center_y - b / 2.0f);
	int w = client_size(a);
	int h = client_size(b);
	int start = (int)(start_angle * 64);
	int sweep = (int)((end_angle - start_angle) * 64);

	if (pt != last_pt)
	{
		last_pt = pt;
		XSetLineAttributes(display, gc, pt * 2 + 1, LineSolid, CapRound, JoinRound);
	}

	XDrawArc(display, window, gc, x, y, w, h, start, sweep);
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

		render_schematic();

		return 1;
	}
	else if (event.type == ConfigureNotify)
	{
		set_viewport_window(event.xconfigure.width, event.xconfigure.height);
		float sw, sh;
		create_schematic(&sw, &sh);
		set_canvas_size(sw, sh);

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

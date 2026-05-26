#include <X11/Xlib.h>
#include <X11/Xutil.h>

Display* display;
Window window;
Atom atom_wmdw;

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

	winatt.background_pixel = 0x00003000;
	winatt.event_mask = ButtonPressMask | ButtonReleaseMask | PointerMotionMask | KeyPressMask | KeyReleaseMask /* | StructureNotifyMask | ExposureMask*/;
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
}

int update_window()
{
	// KeySym ks;
	XEvent event;
	XNextEvent(display, &event);

	if (event.type == ClientMessage)
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

	// case ConfigureNotify:
	// 	printf("Configure Notify Event: %d %d %d x %d\n", event.xconfigure.x, event.xconfigure.y, event.xconfigure.width, event.xconfigure.height);
	// 	break;

	// case Expose:
	// 	printf("Expose Event: Count = %d\n", event.xexpose.count);
	// 	break;

	// default:
	// 	break;
	// }

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

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#include "color_utils.h"

#define MAX_ENTITIES 1000
#define GRID_SIZE 40.0f
#define SNAP_SIZE 20.0f

static struct x11_app_struct {
	Display *display;
	Window window;
	int screen;
	int screen_width;
	int screen_height;
	GC gc;
	Pixmap backbuffer;
	int color_depth;
} app = { 0 };

typedef enum { SHAPE_NONE, SHAPE_LINE } ShapeType;

typedef struct {
	ShapeType type;
	float startX, startY;
	float endX, endY;
} Entity;

typedef struct {
	float targetX, targetY;
	float offsetX, offsetY;
	float zoom;
} Camera2D;

// color utils

// Converts a raw World location to exact Screen Pixel offsets
void WorldToScreen(Camera2D cam, float wx, float wy, int *sx, int *sy)
{
	*sx = (int)((wx - cam.targetX) * cam.zoom + cam.offsetX);
	*sy = (int)((wy - cam.targetY) * cam.zoom + cam.offsetY);
}

// Converts raw Screen Coordinates back into infinite World Coordinates
void ScreenToWorld(Camera2D cam, int sx, int sy, float *wx, float *wy)
{
	*wx = ((float)sx - cam.offsetX) / cam.zoom + cam.targetX;
	*wy = ((float)sy - cam.offsetY) / cam.zoom + cam.targetY;
}

float SnapValue(float value, float snapSize)
{
	return roundf(value / snapSize) * snapSize;
}

int main()
{
	app.display = XOpenDisplay(NULL);
	if (!app.display) {
		fprintf(stderr, "Unable to open X display\n");
		return 1;
	}

	app.screen = DefaultScreen(app.display);
	app.screen_width = 1280;
	app.screen_height = 720;

	app.window = XCreateSimpleWindow(app.display, RootWindow(app.display, app.screen), 0, 0, app.screen_width,
																	 app.screen_height, 0, 0, 0);

	XSelectInput(app.display, app.window,
							 ExposureMask | StructureNotifyMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask |
								 KeyPressMask | KeyReleaseMask);
	XMapWindow(app.display, app.window);

	app.gc = XCreateGC(app.display, app.window, 0, NULL);
	unsigned long colorWhite = WhitePixel(app.display, app.screen);
	unsigned long colorDarkGray = get_rgb_color(app.display, app.screen, 0x28, 0x28, 0x28);
	unsigned long colorRed = get_named_color(app.display, app.screen, "red");
	unsigned long colorGreen = get_named_color(app.display, app.screen, "green");
	unsigned long colorBlue = get_named_color(app.display, app.screen, "blue");

	app.color_depth = DefaultDepth(app.display, app.screen);
	app.backbuffer = XCreatePixmap(app.display, app.window, app.screen_width, app.screen_height, app.color_depth);

	Camera2D camera = { .targetX = 0.0f, .targetY = 0.0f, .zoom = 1.0f };
	camera.offsetX = app.screen_width / 2.0f;
	camera.offsetY = app.screen_height / 2.0f;

	Entity entities[MAX_ENTITIES];
	int entityCount = 0;

	bool isDrawing = false;
	float lineStartWorldX = 0, lineStartWorldY = 0;
	bool snapEnabled = true;

	int lastMouseX = 0, lastMouseY = 0;
	int currentMouseX = 0, currentMouseY = 0;
	bool ctrlPressed = false;

	Atom wmDeleteMessage = XInternAtom(app.display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(app.display, app.window, &wmDeleteMessage, 1);

	bool running = true;
	while (running) {
		bool shouldRedraw = (XPending(app.display) == 0);

		while (XPending(app.display) > 0) {
			XEvent event;
			XNextEvent(app.display, &event);

			if (event.type == KeyPress || event.type == KeyRelease) {
				KeySym sym = XLookupKeysym(&event.xkey, 0);
				if (sym == XK_Control_L || sym == XK_Control_R) {
					ctrlPressed = (event.type == KeyPress);
				}
				if (event.type == KeyPress && sym == XK_s) {
					snapEnabled = !snapEnabled;
					shouldRedraw = true;
				}
			}

			switch (event.type) {
			case ConfigureNotify:
				if (event.xconfigure.width != app.screen_width || event.xconfigure.height != app.screen_height) {
					app.screen_width = event.xconfigure.width;
					app.screen_height = event.xconfigure.height;
					XFreePixmap(app.display, app.backbuffer);
					app.backbuffer = XCreatePixmap(app.display, app.window, app.screen_width, app.screen_height, app.color_depth);
					camera.offsetX = app.screen_width / 2.0f;
					camera.offsetY = app.screen_height / 2.0f;
					shouldRedraw = true;
				}
				break;

			case MotionNotify:
				currentMouseX = event.xmotion.x;
				currentMouseY = event.xmotion.y;
				if (ctrlPressed && (event.xmotion.state & Button1Mask)) {
					int dx = currentMouseX - lastMouseX;
					int dy = currentMouseY - lastMouseY;
					camera.targetX -= (float)dx / camera.zoom;
					camera.targetY -= (float)dy / camera.zoom;
				}
				lastMouseX = currentMouseX;
				lastMouseY = currentMouseY;
				shouldRedraw = true;
				break;

			case ButtonPress:
				currentMouseX = event.xbutton.x;
				currentMouseY = event.xbutton.y;
				if (event.xbutton.button == Button4) {
					float wx, wy;
					ScreenToWorld(camera, currentMouseX, currentMouseY, &wx, &wy);
					camera.zoom *= 1.1f;
					if (camera.zoom > 10.0f)
						camera.zoom = 10.0f;
					camera.offsetX = currentMouseX;
					camera.offsetY = currentMouseY;
					camera.targetX = wx;
					camera.targetY = wy;
					shouldRedraw = true;
				} else if (event.xbutton.button == Button5) {
					float wx, wy;
					ScreenToWorld(camera, currentMouseX, currentMouseY, &wx, &wy);
					camera.zoom /= 1.1f;
					if (camera.zoom < 0.1f)
						camera.zoom = 0.1f;
					camera.offsetX = currentMouseX;
					camera.offsetY = currentMouseY;
					camera.targetX = wx;
					camera.targetY = wy;
					shouldRedraw = true;
				} else if (event.xbutton.button == Button1 && !ctrlPressed) {
					float wx, wy;
					ScreenToWorld(camera, currentMouseX, currentMouseY, &wx, &wy);
					if (snapEnabled) {
						wx = SnapValue(wx, SNAP_SIZE);
						wy = SnapValue(wy, SNAP_SIZE);
					}
					if (!isDrawing) {
						lineStartWorldX = wx;
						lineStartWorldY = wy;
						isDrawing = true;
					} else {
						if (entityCount < MAX_ENTITIES) {
							entities[entityCount] = (Entity){
								.type = SHAPE_LINE, .startX = lineStartWorldX, .startY = lineStartWorldY, .endX = wx, .endY = wy
							};
							entityCount++;
						}
						isDrawing = false;
					}
					shouldRedraw = true;
				} else if (event.xbutton.button == Button3 && isDrawing) {
					isDrawing = false;
					shouldRedraw = true;
				}
				lastMouseX = currentMouseX;
				lastMouseY = currentMouseY;
				break;

			case ClientMessage:
				// FIX: Explicitly target index [0] of the long data payload array
				if ((Atom)event.xclient.data.l[0] == wmDeleteMessage) {
					running = false;
				}
				break;

			case Expose:
				shouldRedraw = true;
				break;
			}
		}

		if (shouldRedraw) {
			XSetForeground(app.display, app.gc, 0x181818);
			XFillRectangle(app.display, app.backbuffer, app.gc, 0, 0, app.screen_width, app.screen_height);

			float wTLX, wTLY, wBRX, wBRY;
			ScreenToWorld(camera, 0, 0, &wTLX, &wTLY);
			ScreenToWorld(camera, app.screen_width, app.screen_height, &wBRX, &wBRY);

			float startX = floorf(wTLX / GRID_SIZE) * GRID_SIZE;
			float endX = ceilf(wBRX / GRID_SIZE) * GRID_SIZE;
			float startY = floorf(wTLY / GRID_SIZE) * GRID_SIZE;
			float endY = ceilf(wBRY / GRID_SIZE) * GRID_SIZE;

			XSetForeground(app.display, app.gc, colorDarkGray);
			for (float x = startX; x <= endX; x += GRID_SIZE) {
				int sx1, sy1, sx2, sy2;
				WorldToScreen(camera, x, startY, &sx1, &sy1);
				WorldToScreen(camera, x, endY, &sx2, &sy2);
				XDrawLine(app.display, app.backbuffer, app.gc, sx1, sy1, sx2, sy2);
			}
			for (float y = startY; y <= endY; y += GRID_SIZE) {
				int sx1, sy1, sx2, sy2;
				WorldToScreen(camera, startX, y, &sx1, &sy1);
				WorldToScreen(camera, endX, y, &sx2, &sy2);
				XDrawLine(app.display, app.backbuffer, app.gc, sx1, sy1, sx2, sy2);
			}

			XSetForeground(app.display, app.gc, colorWhite);
			for (int i = 0; i < entityCount; i++) {
				if (entities[i].type == SHAPE_LINE) {
					int sx1, sy1, sx2, sy2;
					WorldToScreen(camera, entities[i].startX, entities[i].startY, &sx1, &sy1);
					WorldToScreen(camera, entities[i].endX, entities[i].endY, &sx2, &sy2);
					int lineWidth = (int)roundf(2.0f * camera.zoom);
					if (lineWidth < 1)
						lineWidth = 1;
					XSetLineAttributes(app.display, app.gc, lineWidth, LineSolid, CapRound, JoinRound);
					XDrawLine(app.display, app.backbuffer, app.gc, sx1, sy1, sx2, sy2);
				}
			}
			XSetLineAttributes(app.display, app.gc, 1, LineSolid, CapRound, JoinRound);

			float cmWX, cmWY;
			ScreenToWorld(camera, currentMouseX, currentMouseY, &cmWX, &cmWY);
			if (snapEnabled) {
				cmWX = SnapValue(cmWX, SNAP_SIZE);
				cmWY = SnapValue(cmWY, SNAP_SIZE);
			}

			int targetScreenX, targetScreenY;
			WorldToScreen(camera, cmWX, cmWY, &targetScreenX, &targetScreenY);

			if (isDrawing) {
				int sx1, sy1;
				WorldToScreen(camera, lineStartWorldX, lineStartWorldY, &sx1, &sy1);
				XSetForeground(app.display, app.gc, colorRed);
				XDrawLine(app.display, app.backbuffer, app.gc, sx1, sy1, targetScreenX, targetScreenY);
			}

			XSetForeground(app.display, app.gc, snapEnabled ? colorGreen : colorBlue);
			XFillArc(app.display, app.backbuffer, app.gc, targetScreenX - 4, targetScreenY - 4, 8, 8, 0, 360 * 64);

			XSetForeground(app.display, app.gc, colorWhite);
			char hudTop[256];
			char hudBottom[256];

			snprintf(hudTop, sizeof(hudTop), "Controls: Ctrl+Left Drag=Pan | Scroll=Zoom | S=Toggle Snap (%s)",
							 snapEnabled ? "ON" : "OFF");
			XDrawString(app.display, app.backbuffer, app.gc, 15, 25, hudTop, strlen(hudTop));

			snprintf(hudBottom, sizeof(hudBottom), "World Pos: X: %.1f, Y: %.1f", cmWX, cmWY);
			XDrawString(app.display, app.backbuffer, app.gc, 15, app.screen_height - 20, hudBottom, strlen(hudBottom));

			XCopyArea(app.display, app.backbuffer, app.window, app.gc, 0, 0, app.screen_width, app.screen_height, 0, 0);
			XFlush(app.display);
		}
	}

	XFreePixmap(app.display, app.backbuffer);
	XFreeGC(app.display, app.gc);
	XDestroyWindow(app.display, app.window);
	XCloseDisplay(app.display);
	return 0;
}

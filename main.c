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

// Converts a raw World location to exact Screen Pixel offsets
void world_to_screen(Camera2D cam, float wx, float wy, int *sx, int *sy)
{
	*sx = (int)((wx - cam.targetX) * cam.zoom + cam.offsetX);
	*sy = (int)((wy - cam.targetY) * cam.zoom + cam.offsetY);
}

// Converts raw Screen Coordinates back into infinite World Coordinates
void screen_to_world(Camera2D cam, int sx, int sy, float *wx, float *wy)
{
	*wx = ((float)sx - cam.offsetX) / cam.zoom + cam.targetX;
	*wy = ((float)sy - cam.offsetY) / cam.zoom + cam.targetY;
}

float snap_value(float value, float snapSize)
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
	XStoreName(app.display, app.window, "Spice GUI");
	XMapWindow(app.display, app.window);

	app.gc = XCreateGC(app.display, app.window, 0, NULL);
	unsigned long color_white = WhitePixel(app.display, app.screen);
	unsigned long color_dark_gray = get_rgb_color(app.display, app.screen, 0x28, 0x28, 0x28);
	unsigned long color_red = get_named_color(app.display, app.screen, "red");
	unsigned long color_green = get_named_color(app.display, app.screen, "green");
	unsigned long color_blue = get_named_color(app.display, app.screen, "blue");

	app.color_depth = DefaultDepth(app.display, app.screen);
	app.backbuffer = XCreatePixmap(app.display, app.window, app.screen_width, app.screen_height, app.color_depth);

	Camera2D camera = { .targetX = 0.0f, .targetY = 0.0f, .zoom = 1.0f };
	camera.offsetX = app.screen_width / 2.0f;
	camera.offsetY = app.screen_height / 2.0f;

	Entity entities[MAX_ENTITIES];
	int entity_count = 0;

	bool is_drawing = false;
	float line_start_world_x = 0, line_start_world_y = 0;
	bool snap_enabled = true;

	int last_mouse_x = 0, last_mouse_y = 0;
	int current_mouse_x = 0, current_mouse_y = 0;
	bool ctrl_pressed = false;

	Atom wm_delete_message = XInternAtom(app.display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(app.display, app.window, &wm_delete_message, 1);

	bool running = true;
	while (running) {
		bool should_redraw = (XPending(app.display) == 0);

		while (XPending(app.display) > 0) {
			XEvent event;
			XNextEvent(app.display, &event);

			if (event.type == KeyPress || event.type == KeyRelease) {
				KeySym sym = XLookupKeysym(&event.xkey, 0);
				if (sym == XK_Control_L || sym == XK_Control_R) {
					ctrl_pressed = (event.type == KeyPress);
				}
				if (event.type == KeyPress && sym == XK_s) {
					snap_enabled = !snap_enabled;
					should_redraw = true;
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
					should_redraw = true;
				}
				break;

			case MotionNotify:
				current_mouse_x = event.xmotion.x;
				current_mouse_y = event.xmotion.y;
				if (ctrl_pressed && (event.xmotion.state & Button1Mask)) {
					int dx = current_mouse_x - last_mouse_x;
					int dy = current_mouse_y - last_mouse_y;
					camera.targetX -= (float)dx / camera.zoom;
					camera.targetY -= (float)dy / camera.zoom;
				}
				last_mouse_x = current_mouse_x;
				last_mouse_y = current_mouse_y;
				should_redraw = true;
				break;

			case ButtonPress:
				current_mouse_x = event.xbutton.x;
				current_mouse_y = event.xbutton.y;
				if (event.xbutton.button == Button4) {
					float wx, wy;
					screen_to_world(camera, current_mouse_x, current_mouse_y, &wx, &wy);
					camera.zoom *= 1.1f;
					if (camera.zoom > 10.0f)
						camera.zoom = 10.0f;
					camera.offsetX = current_mouse_x;
					camera.offsetY = current_mouse_y;
					camera.targetX = wx;
					camera.targetY = wy;
					should_redraw = true;
				} else if (event.xbutton.button == Button5) {
					float wx, wy;
					screen_to_world(camera, current_mouse_x, current_mouse_y, &wx, &wy);
					camera.zoom /= 1.1f;
					if (camera.zoom < 0.1f)
						camera.zoom = 0.1f;
					camera.offsetX = current_mouse_x;
					camera.offsetY = current_mouse_y;
					camera.targetX = wx;
					camera.targetY = wy;
					should_redraw = true;
				} else if (event.xbutton.button == Button1 && !ctrl_pressed) {
					float wx, wy;
					screen_to_world(camera, current_mouse_x, current_mouse_y, &wx, &wy);
					if (snap_enabled) {
						wx = snap_value(wx, SNAP_SIZE);
						wy = snap_value(wy, SNAP_SIZE);
					}
					if (!is_drawing) {
						line_start_world_x = wx;
						line_start_world_y = wy;
						is_drawing = true;
					} else {
						if (entity_count < MAX_ENTITIES) {
							entities[entity_count] = (Entity){
								.type = SHAPE_LINE, .startX = line_start_world_x, .startY = line_start_world_y, .endX = wx, .endY = wy
							};
							entity_count++;
						}
						is_drawing = false;
					}
					should_redraw = true;
				} else if (event.xbutton.button == Button3 && is_drawing) {
					is_drawing = false;
					should_redraw = true;
				}
				last_mouse_x = current_mouse_x;
				last_mouse_y = current_mouse_y;
				break;

			case ClientMessage:
				// FIX: Explicitly target index [0] of the long data payload array
				if ((Atom)event.xclient.data.l[0] == wm_delete_message) {
					running = false;
				}
				break;

			case Expose:
				should_redraw = true;
				break;
			}
		}

		if (should_redraw) {
			XSetForeground(app.display, app.gc, 0x181818);
			XFillRectangle(app.display, app.backbuffer, app.gc, 0, 0, app.screen_width, app.screen_height);

			float w_tlx, w_tly, w_brx, w_bry;
			screen_to_world(camera, 0, 0, &w_tlx, &w_tly);
			screen_to_world(camera, app.screen_width, app.screen_height, &w_brx, &w_bry);

			float start_x = floorf(w_tlx / GRID_SIZE) * GRID_SIZE;
			float end_x = ceilf(w_brx / GRID_SIZE) * GRID_SIZE;
			float start_y = floorf(w_tly / GRID_SIZE) * GRID_SIZE;
			float end_y = ceilf(w_bry / GRID_SIZE) * GRID_SIZE;

			XSetForeground(app.display, app.gc, color_dark_gray);
			for (float x = start_x; x <= end_x; x += GRID_SIZE) {
				int sx1, sy1, sx2, sy2;
				world_to_screen(camera, x, start_y, &sx1, &sy1);
				world_to_screen(camera, x, end_y, &sx2, &sy2);
				XDrawLine(app.display, app.backbuffer, app.gc, sx1, sy1, sx2, sy2);
			}
			for (float y = start_y; y <= end_y; y += GRID_SIZE) {
				int sx1, sy1, sx2, sy2;
				world_to_screen(camera, start_x, y, &sx1, &sy1);
				world_to_screen(camera, end_x, y, &sx2, &sy2);
				XDrawLine(app.display, app.backbuffer, app.gc, sx1, sy1, sx2, sy2);
			}

			XSetForeground(app.display, app.gc, color_white);
			for (int i = 0; i < entity_count; i++) {
				if (entities[i].type == SHAPE_LINE) {
					int sx1, sy1, sx2, sy2;
					world_to_screen(camera, entities[i].startX, entities[i].startY, &sx1, &sy1);
					world_to_screen(camera, entities[i].endX, entities[i].endY, &sx2, &sy2);
					int line_width = (int)roundf(2.0f * camera.zoom);
					if (line_width < 1)
						line_width = 1;
					XSetLineAttributes(app.display, app.gc, line_width, LineSolid, CapRound, JoinRound);
					XDrawLine(app.display, app.backbuffer, app.gc, sx1, sy1, sx2, sy2);
				}
			}
			XSetLineAttributes(app.display, app.gc, 1, LineSolid, CapRound, JoinRound);

			float cm_wx, cm_wy;
			screen_to_world(camera, current_mouse_x, current_mouse_y, &cm_wx, &cm_wy);
			if (snap_enabled) {
				cm_wx = snap_value(cm_wx, SNAP_SIZE);
				cm_wy = snap_value(cm_wy, SNAP_SIZE);
			}

			int target_screen_x, target_screen_y;
			world_to_screen(camera, cm_wx, cm_wy, &target_screen_x, &target_screen_y);

			if (is_drawing) {
				int sx1, sy1;
				world_to_screen(camera, line_start_world_x, line_start_world_y, &sx1, &sy1);
				XSetForeground(app.display, app.gc, color_red);
				XDrawLine(app.display, app.backbuffer, app.gc, sx1, sy1, target_screen_x, target_screen_y);
			}

			XSetForeground(app.display, app.gc, snap_enabled ? color_green : color_blue);
			XFillArc(app.display, app.backbuffer, app.gc, target_screen_x - 4, target_screen_y - 4, 8, 8, 0, 360 * 64);

			XSetForeground(app.display, app.gc, color_white);
			char hud_top[256];
			char hud_bottom[256];

			snprintf(hud_top, sizeof(hud_top), "Controls: Ctrl+Left Drag=Pan | Scroll=Zoom | S=Toggle Snap (%s)",
							 snap_enabled ? "ON" : "OFF");
			XDrawString(app.display, app.backbuffer, app.gc, 15, 25, hud_top, strlen(hud_top));

			snprintf(hud_bottom, sizeof(hud_bottom), "World Pos: X: %.1f, Y: %.1f", cm_wx, cm_wy);
			XDrawString(app.display, app.backbuffer, app.gc, 15, app.screen_height - 20, hud_bottom, strlen(hud_bottom));

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

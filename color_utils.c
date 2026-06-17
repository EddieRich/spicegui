/*
 * Why this is production grade:
 * It completely abstracts the XColor struct definitions out of
 * your main loop. It scales 8-bit (255) variables up to 16-bit (65535)
 * layouts safely. It guarantees that even if you run this application
 * on an embedded system or legacy console, it will safely fallback to
 * White instead of crashing or generating random garbage visual noise.
 */

#include <X11/Xlib.h>
#include <stdio.h>

#include "color_utils.h"

unsigned long get_rgb_color(Display *display, int screen, unsigned char r, unsigned char g, unsigned char b)
{
	XColor color;

	// Scale standard 8-bit components (0-255) up to X11 16-bit space (0-65535)
	color.red = (r * 65535) / 255;
	color.green = (g * 65535) / 255;
	color.blue = (b * 65535) / 255;
	color.flags = DoRed | DoGreen | DoBlue;

	if (XAllocColor(display, XDefaultColormap(display, screen), &color)) {
		return color.pixel; // Perfect machine-dependent pixel index
	}

	// Bulletproof Fallback if the hardware palette is completely exhausted
	return WhitePixel(display, screen);
}

unsigned long get_hex_color(Display *display, int screen, const char *hex_str)
{
	unsigned int r = 0, g = 0, b = 0;

	// Parse the hex layout string cleanly
	if (hex_str[0] == '#') {
		sscanf(hex_str, "#%02x%02x%02x", &r, &g, &b);
	} else {
		sscanf(hex_str, "%02x%02x%02x", &r, &g, &b);
	}

	return get_rgb_color(display, screen, (unsigned char)r, (unsigned char)g, (unsigned char)b);
}

unsigned long get_named_color(Display *display, int screen, const char *name)
{
	XColor exact_def, screen_def;

	if (XAllocNamedColor(display, XDefaultColormap(display, screen), name, &screen_def, &exact_def)) {
		return screen_def.pixel;
	}

	// Dynamic fallback if the user provides a string that doesn't exist
	return WhitePixel(display, screen);
}

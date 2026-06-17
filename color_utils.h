#ifndef __COLOR_UTILS_H__
#define __COLOR_UTILS_H__

unsigned long get_rgb_color(Display *display, int screen, unsigned char r, unsigned char g, unsigned char b);
unsigned long get_hex_color(Display *display, int screen, const char *hex_str);
unsigned long get_named_color(Display *display, int screen, const char *name);

#endif // __COLOR_UTILS_H__

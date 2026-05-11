#ifndef __PLATFORM__
#define __PLATFORM__

/*
 * This is everything common to both platforms
 */

typedef enum eColor {
	NONE,
	BLACK,
	WHITE,
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	PURPLE,
	COLOR_COUNT
} Color;

void FillRectangle(Rect rect, Color color);
void DrawRectangle(Rect rect, Color color);

#endif // __PLATFORM__

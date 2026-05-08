#ifndef __VIEWPORT__
#define __VIEWPORT__

#include "types.h"

typedef struct viewport_s
{
	Size client;
	SizeF canvas;
	Point client_offset;
	float zoom;							// zoom factor, 0 is unset, minimum is 1.0
	float scale;						// scale factor based on client, canvas, zoom
} t_Viewport;

int set_viewport_window(int width, int height);
int set_canvas_size(float width, float height);
Rect get_canvas_client_bounds();
Rect rectf_to_client(RectF rectf);

#endif // __VIEWPORT__

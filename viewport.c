#include "types.h"
#include "viewport.h"

static t_Viewport viewport = {0};

static void calculate_layout(void)
{
	if (viewport.zoom < 0.5)
		viewport.zoom = 1.0;

	float hscale = viewport.zoom * viewport.client.width / viewport.canvas.width;
	float vscale = viewport.zoom * viewport.client.height / viewport.canvas.height;
	viewport.scale = (hscale > vscale) ? vscale : hscale;
	viewport.client_offset.x = (viewport.client.width - (int)(viewport.canvas.width * viewport.scale)) >> 1;
	viewport.client_offset.y = (viewport.client.height - (int)(viewport.canvas.height * viewport.scale)) >> 1;
}

int set_viewport_window(int width, int height)
{
	viewport.client.width = width;
	viewport.client.height = height;
	if (viewport.canvas.width > 0.0 && viewport.canvas.height > 0.0)
	{
		calculate_layout();
		return 1;
	}

	return 0;
}

int set_canvas_size(float width, float height)
{
	viewport.canvas.width = width;
	viewport.canvas.height = height;
	if (viewport.client.width && viewport.client.height && viewport.zoom < 0.999)
	{
		calculate_layout();
		return 1;
	}

	return 0;
}

// called to paint the blank canvas
Rect get_canvas_client_bounds()
{
	Rect bounds;
	bounds.point = viewport.client_offset;
	bounds.size.width = (int)(viewport.canvas.width * viewport.scale);
	bounds.size.height = (int)(viewport.canvas.height * viewport.scale);
	return bounds;
}

Rect rectf_to_client(RectF rectf)
{
	Rect r;
	r.point.x = viewport.client_offset.x + rectf.point.x * viewport.scale;
	r.point.y = viewport.client_offset.y + rectf.point.y * viewport.scale;
	r.size.width = (int)(rectf.size.width * viewport.scale);
	r.size.height = (int)(rectf.size.height * viewport.scale);
	return r;
}

#include "viewport.h"

static struct
{
	int client_width;
	int client_height;
	float canvas_width;
	float canvas_height;
	int canvas_client_x;
	int canvas_client_y;
	float zoom;							// zoom factor, 0 is unset, minimum is 1.0
	float scale;						// scale factor based on client, canvas, zoom
} vp = {0};

static const float zoom_scale = 1.4142135;

static void calculate_layout(void)
{
	if (vp.zoom < 0.5)
		vp.zoom = 1.0;

	float hscale = vp.zoom * vp.client_width / vp.canvas_width;
	float vscale = vp.zoom * vp.client_height / vp.canvas_height;
	vp.scale = (hscale > vscale) ? vscale : hscale;
	vp.canvas_client_x = (vp.client_width - (int)(vp.canvas_width * vp.scale)) >> 1;
	vp.canvas_client_y = (vp.client_height - (int)(vp.canvas_height * vp.scale)) >> 1;
}

int set_viewport_window(int width, int height)
{
	vp.client_width = width;
	vp.client_height = height;
	if (vp.canvas_width > 0.0 && vp.canvas_height > 0.0)
	{
		calculate_layout();
		return 1;
	}

	return 0;
}

int set_canvas_size(float width, float height)
{
	vp.canvas_width = width;
	vp.canvas_height = height;
	if (vp.client_width && vp.client_height)
	{
		calculate_layout();
		return 1;
	}

	return 0;
}

float canvas_width()
{
	return vp.canvas_width;
}

float canvas_height()
{
	return vp.canvas_height;
}

int client_x(float x)
{
	return vp.canvas_client_x + (x * vp.scale);
}

int client_y(float y)
{
	return vp.canvas_client_y + (y * vp.scale);
}

int client_size(float sz)
{
	return sz * vp.scale;
}

int point_in_window(int x, int y)
{
	return (x >= 0 && y >= 0 && x < vp.client_width && y < vp.client_height);
}

void zoom_in_at(int cursor_x, int cursor_y)
{
	(void)cursor_x;
	(void)cursor_y;
	vp.zoom *= zoom_scale;
	calculate_layout();
}
